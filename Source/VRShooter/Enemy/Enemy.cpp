#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "VRShooter/Character/VRShooterCharacter.h"
#include "VRShooter/Actors/WidgetActor.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "DrawDebugHelpers.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "GameFramework/Controller.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraShakeBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SceneComponent.h"
#include "VRShooter/Pickups/PickupSpawner.h"
#include "VRShooter/Weapon/Projectile.h"
#include "Perception/AIPerceptionComponent.h"
#include "EnemySpawner.h"

AEnemy::AEnemy()
{
 	PrimaryActorTick.bCanEverTick = true;

	HealthBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBar->SetupAttachment(GetRootComponent());
	HealthBar->SetVisibility(false);

	ShieldBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("ShieldBar"));
	ShieldBar->SetupAttachment(GetRootComponent());
	ShieldBar->SetVisibility(false);

	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());
	
	CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatRangeSphere"));
	CombatRangeSphere->SetupAttachment(GetRootComponent());

	// Left and Right WeaponCollisionBoxes
	LeftWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftWeaponCollision"));
	LeftWeaponCollision->SetupAttachment(GetMesh(), FName("LeftWeaponBone"));

	RightWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RightWeaponCollision"));
	RightWeaponCollision->SetupAttachment(GetMesh(), FName("RightWeaponBone"));

	// TODO: This is only for test purposes. It's will be changed to AI Seeing peception
	ShootingRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ShootingRangeSphere"));
	ShootingRangeSphere->SetupAttachment(GetRootComponent());

	// AI Smooth rotation
	bUseControllerRotationYaw = false;

	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnAgroSphereOverlap);
	
	CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnCombatRangeSphereOverlap);
	CombatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::OnCombatRangeSphereEndOverlap);
	ShootingRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnShootingRangeSphereOverlap);
	ShootingRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::OnShootingRangeSphereEndOverlap);

	// Bind funtions to overlap events for weapon boxes
	LeftWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnLeftWeaponOverlap);
	RightWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnRightWeaponOverlap);

	// Set collision presets for weapon boxes
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	LeftWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	LeftWeaponCollision->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Pawn, 
		ECollisionResponse::ECR_Overlap
	);

	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	RightWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	RightWeaponCollision->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Pawn,
		ECollisionResponse::ECR_Overlap
	);

	GetMesh()->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Visibility, 
		ECollisionResponse::ECR_Block
	);

	GetMesh()->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Camera, 
		ECollisionResponse::ECR_Ignore
	);

	GetCapsuleComponent()->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Camera,
		ECollisionResponse::ECR_Ignore
	);

	// Get the AIController
	EnemyController = Cast<AEnemyController>(GetController());

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), true);
	}

	// Converting PatrolPoint from LocalSpace to WorldSpace
	const FVector WorldPatrolPoint = UKismetMathLibrary::TransformLocation(
		GetActorTransform(), 
		PatrolPoint
	);

	/*DrawDebugSphere(
		GetWorld(),
		WorldPatrolPoint,
		25.f,
		12,
		FColor::Red,
		true
	);*/

	const FVector WorldPatrolPoint2 = UKismetMathLibrary::TransformLocation(
		GetActorTransform(),
		PatrolPoint2
	);

	/*DrawDebugSphere(
		GetWorld(),
		PatrolPoint2,
		25.f,
		12,
		FColor::Red,
		true
	);*/

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsVector(
			TEXT("PatrolPoint"),
			WorldPatrolPoint
		);

		EnemyController->GetBlackboardComponent()->SetValueAsVector(
			TEXT("PatrolPoint2"),
			WorldPatrolPoint2
		);

		EnemyController->GetBlackboardComponent()->SetValueAsBool(
			TEXT("ShootingEnemy"),
			bShootingEnemy
		);

		EnemyController->RunBehaviorTree(BehaviorTree);
	}
	
	if (bExplosiveEnemy &&
		ScreamSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			ScreamSound,
			GetActorLocation()
		);
	}
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (VRShooterCharacter)
	{
		if (HealthBar &&
			HealthBar->IsWidgetVisible())
		{
			RotateWidgetToPlayer(HealthBar, VRShooterCharacter->GetActorLocation());
		}
		if (ShieldBar &&
			Shield > 0.f &&
			ShieldBar->IsWidgetVisible())
		{
			RotateWidgetToPlayer(ShieldBar, VRShooterCharacter->GetActorLocation());
		}

		// Spawning Enemies
		StartRestartEnemySpawnTimer();
	
		if (bShouldRotateToPlayer &&
			!bDying)
		{
			RotateToPlayer(DeltaTime);
		}
	}

	if (bIsRagdoll)
	{
		RagdollUpdate();
	}
	if (bRotateMesh)
	{
		GetMesh()->SetRelativeRotation(FMath::RInterpTo(GetMesh()->GetRelativeRotation(), FRotator(0.f, -90.f, 0.f), DeltaTime, RagdollInterpTime));
	}
}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AEnemy::OnAgroSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bfromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr) return;

	VRShooterCharacter = Cast<AVRShooterCharacter>(OtherActor);
	//VRShooterCharacter = VRShooterCharacter == nullptr ? Cast<AVRShooterCharacter>(OtherActor) : VRShooterCharacter;

	if (VRShooterCharacter &&
		EnemyController &&
		EnemyController->GetBlackboardComponent())
	{
		// Set the value of the TargetBlackboardKey
		EnemyController->GetBlackboardComponent()->SetValueAsObject(
			TEXT("Target"), 
			VRShooterCharacter
		);
	}
}

void AEnemy::OnCombatRangeSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bfromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr) return;

	VRShooterCharacter = Cast<AVRShooterCharacter>(OtherActor);
	//VRShooterCharacter = VRShooterCharacter == nullptr ? Cast<AVRShooterCharacter>(OtherActor) : VRShooterCharacter;
	
	if (VRShooterCharacter)
	{
		bInAttackRange = true;

		if (bExplosiveEnemy)
		{
			// TODO: Implementing Blackboard
			// Explode
			DoDamage(VRShooterCharacter, true);
			Die();
		}
		else
		{
			if (EnemyController)
			{
				EnemyController->GetBlackboardComponent()->SetValueAsBool(
					TEXT("InAttackRange"),
					true
				);
			}
		}
	}
}

void AEnemy::OnShootingRangeSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bfromSweep, const FHitResult& SweepResult)

{
	if (OtherActor == nullptr) return;

	// TODO: Aim to Player
	// TODO: Rotate To Player
	VRShooterCharacter = Cast<AVRShooterCharacter>(OtherActor);
	//VRShooterCharacter = VRShooterCharacter == nullptr ? Cast<AVRShooterCharacter>(OtherActor) : VRShooterCharacter;
	
	if (EnemyController &&
		VRShooterCharacter)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(
			TEXT("InShootingRange"),
			true
		);
	}
}

void AEnemy::OnShootingRangeSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == nullptr) return;

	VRShooterCharacter = Cast<AVRShooterCharacter>(OtherActor);
	//VRShooterCharacter = VRShooterCharacter == nullptr ? Cast<AVRShooterCharacter>(OtherActor) : VRShooterCharacter;

	if (VRShooterCharacter)
	{
		bInAttackRange = false;

		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(
				TEXT("InShootingRange"),
				false
			);
		}
	}
}

void AEnemy::PlayAttackMontage(FName Section, float PlayRate)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance &&
		AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage, PlayRate);
		AnimInstance->Montage_JumpToSection(Section, AttackMontage);
	}

	bCanAttack = false;

	GetWorldTimerManager().SetTimer(
		AttackWaitTimer,
		this,
		&AEnemy::ResetCanAttack,
		AttackWaitTime
	);

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), false);
	}
}

FName AEnemy::GetAttackSectionName()
{
	const int32 Section{ FMath::RandRange(1, 4) };
	FName SectionName;

	switch (Section)
	{
	case 1:
		SectionName = AttackLFast;
		
		break;
	case 2:
		SectionName = AttackRFast;
		
		break;
	case 3:
		SectionName = AttackLSword;
		
		break;
	case 4:
		SectionName = AttackRSword;
		
		break;
	}

	return SectionName;
}

void AEnemy::ToggleRotateToPlayer(bool bRotate)
{
	bShouldRotateToPlayer = bRotate;
}

void AEnemy::RotateToPlayer(float DeltaTime)
{
	if (VRShooterCharacter)
	{
		// Target direction
		FVector PlayerLocation = VRShooterCharacter->GetActorLocation();
		// Enemy LookAtDirektion
		//FVector EnemyLookAtDirection = GetActorLocation() * GetActorForwardVector().Normalize();
		FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), PlayerLocation);

	  SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, RotationSpeed));
	}
}

void AEnemy::OnCombatRangeSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == nullptr) return;

	VRShooterCharacter = Cast<AVRShooterCharacter>(OtherActor);
	//VRShooterCharacter = VRShooterCharacter == nullptr ? Cast<AVRShooterCharacter>(OtherActor) : VRShooterCharacter;
	
	if (VRShooterCharacter)
	{
		bInAttackRange = false;

		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(
				TEXT("InAttackRange"), 
				false
			);
		}
	}
}

void AEnemy::ActivateLeftWeapon()
{
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::OnLeftWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bfromSweep, const FHitResult& SweepResult)
{
	VRShooterCharacter = Cast<AVRShooterCharacter>(OtherActor);
	//VRShooterCharacter = VRShooterCharacter == nullptr ? Cast<AVRShooterCharacter>(OtherActor) : VRShooterCharacter;
	
	if (VRShooterCharacter)
	{
		DoDamage(VRShooterCharacter);
		SpawnBloodParticles(VRShooterCharacter, LeftWeaponSocket);
	}
}

void AEnemy::DeactivateLeftWeapon()
{
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ActivateRightWeapon()
{
	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::OnRightWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bfromSweep, const FHitResult& SweepResult)
{
	VRShooterCharacter = Cast<AVRShooterCharacter>(OtherActor);
	//VRShooterCharacter = VRShooterCharacter == nullptr ? Cast<AVRShooterCharacter>(OtherActor) : VRShooterCharacter;
	
	if (VRShooterCharacter)
	{
		DoDamage(VRShooterCharacter);
		SpawnBloodParticles(VRShooterCharacter, RightWeaponSocket);
	}
}

void AEnemy::DeactivateRightWeapon()
{
	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::SpawnBloodParticles(AVRShooterCharacter* Victim, FName SocketName)
{
	const USkeletalMeshSocket* TipSocket{ GetMesh()->GetSocketByName(SocketName) };

	if (TipSocket)
	{
		const FTransform SocketTransform{ TipSocket->GetSocketTransform(GetMesh()) };

		if (Victim->GetBloodParticles())
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				Victim->GetBloodParticles(),
				SocketTransform
			);

			//BloodParticleSystemComponent->OnParticleCollide.AddDynamic(this, &AEnemy::OnBloodParticleCollide);
		}

		if (Victim->GetBloodNiagara())
		{
			SpawnBloodNiagara(Victim->GetBloodNiagara(), SocketTransform.GetLocation());
			/*UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				this,
				Victim->GetBloodNiagara(),
				SocketTransform.GetLocation(),
				GetActorRotation()
			);*/
		}
	}
}

void AEnemy::SpawnBloodNiagara(UNiagaraSystem* NiagaraSystem, FVector Location)
{
	UNiagaraComponent* SpawnedNiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		this,
		NiagaraSystem,
		Location,
		GetActorRotation()
	);

	if (SpawnedNiagaraComp)
	{
		SpawnedNiagaraComp->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnBloodNiagaraBeginOverlap);
		SpawnedNiagaraComp->OnComponentHit.AddDynamic(this, &AEnemy::OnBloodNiagaraHit);
		SpawnedNiagaraComp->SetNiagaraVariableObject("User.Data", this);
		//UE_LOG(LogTemp, Warning, TEXT("BloodNiagaraComponent"));
	}
}

void AEnemy::OnBloodParticleCollide(FName EventName, float EmitterTime, int32 ParticleTime, FVector Location, FVector Velocity, FVector Direction, FVector Normal, FName BoneName, UPhysicalMaterial* PhysMat)
{
	UE_LOG(LogTemp, Warning, TEXT("OnBloodParticleCollide"));
}

void AEnemy::OnBloodNiagaraBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bfromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("OnBloodNiagaraBeginOverlap"));
}

void AEnemy::OnBloodNiagaraHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("OnBloodNiagaraHit"));
}

void AEnemy::SpawningProjectile()
{
	const USkeletalMeshSocket* ProjectileSocket = GetMesh()->GetSocketByName(ProjectileSocketName);

	if (EnemyController)
	{
		VRShooterCharacter = Cast<AVRShooterCharacter>(EnemyController->GetBlackboardComponent()->GetValueAsObject(FName("Target")));

		if (ProjectileSocket &&
			VRShooterCharacter)
		{
			FTransform ProjectileSocketTransform = ProjectileSocket->GetSocketTransform(GetMesh());
			FVector SocketLocation = ProjectileSocketTransform.GetLocation();
			FVector Direction = VRShooterCharacter->GetActorLocation() - SocketLocation;
			FRotator Rotation = FRotationMatrix::MakeFromX(Direction).Rotator();

			//FQuat Rotation = ProjectileSocketTransform.GetRotation();

			FRotator TargetRotation = FRotator(Rotation);//ToTarget.Rotation();

			if (ProjectileClass)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = GetOwner();
				SpawnParams.Instigator = this;
				UWorld* World = GetWorld();

				if (World)
				{
					World->SpawnActor<AProjectile>(
						ProjectileClass,
						SocketLocation,
						TargetRotation,
						SpawnParams
						);
				}
			}
		}
	}
}

void AEnemy::StartRestartEnemySpawnTimer()
{
	if (EnemyToSpawnClasses.Num() > 0 &&
		Shield > 0.f &&
		SpawnedEnemies.Num() == 0 &&
		!bEnemiesSpawningStarted &&
		!bDying)
	{
		bEnemiesSpawningStarted = true;

		GetWorldTimerManager().SetTimer(
			RestartEnemySpawnTimer,
			this,
			&AEnemy::SpawnEnemies,
			RestartEnemySpawnTime
		);
	}
}

void AEnemy::SpawnEnemies()
{
	if (EnemyToSpawnClasses.Num() > 0 &&
		SpawnedEnemies.Num() == 0 &&
		!bDying)
	{
		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(
				TEXT("SpawningEnemy"),
				true
			);
		}

		bEnemiesSpawning = true;
		bCanHitReact = false;

		for (auto SpawnClass : EnemyToSpawnClasses)
		{
			EnemyToSpawnClass = SpawnClass;
			StartEnemySpawnTimer();
		}
	}
}

void AEnemy::StartEnemySpawnTimer()
{
	FTimerHandle EnemySpawnTimer;
	FTimerDelegate EnemySpawnDelegate;
	EnemySpawnDelegate.BindUFunction(this, FName("PlaySpawningEnemyMontage"));
	GetWorld()->GetTimerManager().SetTimer(
		EnemySpawnTimer,
		EnemySpawnDelegate,
		EnemySpawnTime,
		false
	);
}

void AEnemy::PlaySpawningEnemyMontage()
{
	if (SpawningEnemiesMontage &&
		!bDying)
	{
		// TODO: Create a generic MontagePlay function to Playing HitReact, Attack, Death, Spell animations
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (AnimInstance)
		{
			AnimInstance->Montage_Play(SpawningEnemiesMontage, 1.f);
		}
	}
}

// It's called in AnimBP (AnimNotify, afet montage played)
void AEnemy::SpawnEnemy()
{
	if (EnemyToSpawnClass &&
		!bDying)
	{
		FVector EnemyLocation = GetActorLocation() + EnemySpawnOffset;
		FTransform SpawnedEnemyTransform = FTransform(FQuat(GetActorRotation()), FVector(EnemyLocation.X, EnemyLocation.Y, 268), FVector(1.f));
		SpawnedEnemy = GetWorld()->SpawnActor<AEnemy>(EnemyToSpawnClass, SpawnedEnemyTransform);
		
		if (SpawnedEnemy)
		{
			SpawnedEnemy->SetSpawnerEnemyReference(this);
			SpawnedEnemies.Add(SpawnedEnemy);
		}

		// Restart Timer -> Spawn new enemies if the SpawnedEnemies <= MaxNumberOfSpawnedEnemies
		if (SpawnedEnemies.Num() < MaxNumberOfSpawnedEnemies)
		{
			StartEnemySpawnTimer();
		}
		// Stop spawning Enemies
		else if (EnemyToSpawnClasses.Num() > 0 &&
			SpawnedEnemies.Num() == MaxNumberOfSpawnedEnemies)
		{
			bCanHitReact = true;
			bEnemiesSpawning = false;
			bEnemiesSpawningStarted = false;

			if (EnemyController)
			{
				EnemyController->GetBlackboardComponent()->SetValueAsBool(
					TEXT("SpawningEnemy"),
					false
				);

				SetPlayerCantHurtForAWhile();
			}
		}
	}
}

void AEnemy::SetPlayerCantHurtForAWhile()
{
	bPlayerCanHurt = false;

	GetWorldTimerManager().SetTimer(
		PlayerCantHurtTimer,
		this,
		&AEnemy::ResetPlayerCantHurt,
		PlayerCantHurtTime
	);
}

void AEnemy::ResetPlayerCantHurt()
{
	bPlayerCanHurt = true;
}

FVector AEnemy::GetRandomSpawningPoint()
{
	return UKismetMathLibrary::TransformLocation(
		GetActorTransform(),
		FMath::RandBool() == true ? EnemySpawnPoint : EnemySpawnPoin2
	);
}

void AEnemy::DoDamage(AVRShooterCharacter* Victim, bool bRadialDamage)
{
	if (Victim == nullptr) return;

	if (bRadialDamage)
	{
		TArray<AActor*, FDefaultAllocator> ActorsToIgnore;

		UGameplayStatics::ApplyRadialDamage(
			this,
			ExplosionDamage,
			GetActorLocation(),
			ExplosionRadius,
			UDamageType::StaticClass(),
			ActorsToIgnore,
			this,
			EnemyController
		);

		if (ExplosionSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				ExplosionSound,
				GetActorLocation()
			);
		}

		if (ExplosionCameraShake)
		{
			UGameplayStatics::PlayWorldCameraShake(
				GetWorld(),
				ExplosionCameraShake,
				GetActorLocation(),
				ExplosionRadius,
				ExplosionRadius
			);
		}

		if (ExplosionNiagara)
		{
			SpawnBloodNiagara(ExplosionNiagara, GetActorLocation());
			/*UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				this,
				ExplosionNiagara,
				GetActorLocation(),
				GetActorRotation()
			);*/
		}
	} 
	else
	{
		UGameplayStatics::ApplyDamage(
			Victim,
			BaseDamage,
			EnemyController,
			this,
			UDamageType::StaticClass()
		);
	}

	if (Victim->GetMeleeImpactSound())
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			Victim->GetMeleeImpactSound(),
			GetActorLocation()
		);
	}
}

void AEnemy::ResetCanAttack()
{
	bCanAttack = true;

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), true);
	}
}

void AEnemy::BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* ShooterController)
{
	VRShooterCharacter = VRShooterCharacter == nullptr ? Cast<AVRShooterCharacter>(Shooter) : VRShooterCharacter;

	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ImpactParticles,
			HitResult.Location,
			FRotator(0.f),
			true
		);
	}

	BloodNiagara = HitResult.BoneName.ToString() == GetHeadBone() ? HeadBloodNiagara : BodyBloodNiagara;

	if (BloodNiagara)
	{
		SpawnBloodNiagara(BloodNiagara, HitResult.Location);

		/*BloodNiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			BloodNiagara,
			HitResult.Location,
			GetActorRotation()
		);*/
		
		
		//if (BloodNiagaraComponent)
		//{
		//	BloodNiagaraComponent->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnBloodNiagaraBeginOverlap);
		//	BloodNiagaraComponent->OnComponentHit.AddDynamic(this, &AEnemy::OnBloodNiagaraHit);
		//	BloodNiagaraComponent->SetNiagaraVariableObject("User.Data", this);
		//	//UE_LOG(LogTemp, Warning, TEXT("BloodNiagaraComponent"));
		//}
	}

	if (bDying)
	{		
		return;
	}

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			ImpactSound,
			GetActorLocation()
		);
	}
	
	ShowOverlayWidget(HealthBar);
	
	if (Shield > 0)
	{
		ShowOverlayWidget(ShieldBar);
	}

	if (!bEnemiesSpawning &&
		bPlayerCanHurt &&
		!bIsBossEnemy)
	{
		// Determine whether bullet hit stunns
		const float Stunned = FMath::FRandRange(0.f, 1.f);

		if (Stunned <= StunChance)
		{
			// Stun the Enemy
			PlayHitMontage(FName("HitReactFront"));
			SetStunned(true);
		}
	}
}

void AEnemy::SwitchBloodParticles(bool bIsHeadshot)
{
	if (HeadBloodNiagara &&
		BodyBloodNiagara)
	{
		if (bIsHeadshot)
		{
			BloodNiagara = HeadBloodNiagara;
		}
		else
		{
			BloodNiagara = BodyBloodNiagara;
		}
	}
}

void AEnemy::BreakingBones(FVector Impulse, FVector HitLocation, FName Bone)
{
	FName BoneToBreak;

	if (Bone.ToString() == "lowerarm_l")
	{
		BoneToBreak = TEXT("lowerarm_l");
	}
	if (Bone.ToString() == "lowerarm_r")
	{
		BoneToBreak = TEXT("lowerarm_r");
	}
	/*if (Bone.ToString() == "head")
	{
		BoneToBreak = TEXT("head");
	}*/

	Impulse = GetActorLocation() - Impulse;
	Impulse.Normalize();

	Impulse *= 9'000;

	//GetMesh()->BreakConstraint(Impulse, HitLocation, BoneToBreak);
	//AddHitReactImpulse(Impulse, HitLocation, Bone, false);
}

void AEnemy::AddHitReactImpulse_Implementation(FVector Impulse, FVector HitLocation, FName Bone, bool bAddImpulse)
{

}

void AEnemy::SetStunned(bool Stunned)
{
	bStunned = Stunned;

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(
			TEXT("Stunned"), 
			Stunned
		);

		if (bStunned)
		{
			GetWorldTimerManager().SetTimer(
				StunnResetTimer,
				this,
				&AEnemy::ResetStunn,
				StunnResetTime
			);
		}
	}
}

void AEnemy::ResetStunn()
{
	SetStunned(false);
}

void AEnemy::ShowHitNumber(AVRShooterCharacter* Causer, int32 Damage, FVector HitLocation, bool bHeadShot)
{
	// HitNumber will be only showed if no Spawned Enemies exists
	if (SpawnedEnemies.Num() == 0
		&& bPlayerCanHurt)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		UWorld* World = GetWorld();

		if (World)
		{
			AWidgetActor* HitNumberWidgetActor = World->SpawnActor<AWidgetActor>(
				WidgetActorClass,
				HitLocation,
				GetActorRotation(),
				SpawnParams
				);

			if (HitNumberWidgetActor &&
				Causer)
			{
				HitNumberWidgetActor->SetShooterCharacter(Causer);
				HitNumberWidgetActor->SetTextAndStartAnimation(FString::Printf(TEXT("%d"), Damage), bHeadShot);
				StoreHitNumber(HitNumberWidgetActor, HitLocation);
			}
		}
	}
}

void AEnemy::StoreHitNumber(AWidgetActor* HitNumber, FVector Location)
{
	HitNumberActors.Add(HitNumber, Location);

	FTimerHandle HitNumberTimer;
	FTimerDelegate HitNumberDelegate;
	HitNumberDelegate.BindUFunction(this, FName("DestroyHitNumber"), HitNumber);
	GetWorld()->GetTimerManager().SetTimer(
		HitNumberTimer,
		HitNumberDelegate,
		HitNumberDestroyTime,
		false
	);
}

void AEnemy::DestroyHitNumber(AWidgetActor* HitNumber)
{
	HitNumberActors.Remove(HitNumber);
	HitNumber->Destroy();
}

void AEnemy::PlayHitMontage(FName Section, float PlayRate)
{
	if (bCanHitReact)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(HitMontage, PlayRate);
			AnimInstance->Montage_JumpToSection(Section, HitMontage);
		}

		bCanHitReact = false;
		const float HitReactTime{ FMath::FRandRange(HitReactTimeMin, HitReactTimeMax) };
		
		GetWorldTimerManager().SetTimer(
			HitReactTimer,
			this,
			&AEnemy::ResetHitReactTimer,
			HitReactTime
		);
	}
}

void AEnemy::ShowOverlayWidget(UWidgetComponent* WidgetComponent)
{
	if (WidgetComponent)
	{
		if (!bIsBossEnemy)
		{
			FTimerHandle OverlayWidgetTimer;
			FTimerDelegate OverlayWidgetDelegate;
			OverlayWidgetDelegate.BindUFunction(this, FName("HideOverlayWidget"), WidgetComponent);
			GetWorld()->GetTimerManager().SetTimer(
				OverlayWidgetTimer,
				OverlayWidgetDelegate,
				OverlayWidgetDisplayTime,
				false
			);
		}
		WidgetComponent->SetVisibility(true);
	}
}

void AEnemy::HideOverlayWidget(UWidgetComponent* WidgetComponent)
{
	if (WidgetComponent)
	{
		WidgetComponent->SetVisibility(false);
	}
}

void AEnemy::RotateWidgetToPlayer(UWidgetComponent* Widget, FVector PlayerLocation)
{
	if (Widget)
	{
		FRotator WidgetRotation = Widget->GetRelativeRotation();
		FVector Direction = PlayerLocation - GetActorLocation();
		FRotator Rotation = FRotationMatrix::MakeFromX(Direction).Rotator();
		Widget->SetWorldRotation(FRotator(0.f, Rotation.Yaw, 0.f));
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	//UE_LOG(LogTemp, Error, TEXT("Dying: %d"), bDying);
	//UE_LOG(LogTemp, Error, TEXT("PlayerCanHurt: %d"), bPlayerCanHurt);
	if (!bDying && 		
		bPlayerCanHurt)
	{
		//UE_LOG(LogTemp, Error, TEXT("if statement %f"), DamageAmount);
		float DamageToHealth = DamageAmount;

		// Stop rotation to player
		ToggleRotateToPlayer(false);

		// Set the target blackboard key to agro the character
		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsObject(
				FName("Target"),
				DamageCauser
			);
		}
		if (SpawnedEnemies.Num() == 0)
		{
			if (Shield > 0.f)
			{
				if (Shield >= DamageAmount)
				{
					Shield = FMath::Clamp(Shield - DamageAmount, 0.f, MaxShield);
					DamageToHealth = 0.f;
				}
				else
				{
					DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, DamageAmount);
					Shield = 0.f;
				}
			}

			Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);

			if (Health <= 0.f)
			{
				Health = 0.f;
				UpdatePlayerKillCounter(EventInstigator->GetPawn());
				Die();

			}
		}
	}
	return Health;
}

void AEnemy::ResetHitReactTimer()
{
	bCanHitReact = true;
}

void AEnemy::Die()
{
	if (bDying) return;

	if (SpawnerEnemyReference)
	{
		SpawnerEnemyReference->RemoveEnemyFromSpawnedEnemies(this);
	}

	bDying = true;

	HideOverlayWidget(HealthBar);
	HideOverlayWidget(ShieldBar);
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance &&
		DeathMontage &&
		!bRagdollOnDeath)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}
	else
	{
		StartSlowMotion();
		RagdollStart();
	}

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(
			FName("Dead"), 
			true
		);

		EnemyController->StopMovement();
	}
}

void AEnemy::RemoveEnemyFromSpawnedEnemies(AEnemy* EnemyToRemove)
{
	SpawnedEnemies.Remove(EnemyToRemove);
}

void AEnemy::RagdollStart()
{
	if (bRagdollDisabled) return;

	bIsRagdoll = true;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetAllBodiesBelowSimulatePhysics(FName("pelvis"), true, true);
	GetMesh()->GetAnimInstance()->StopAllMontages(0.2f);

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

	if (!bDying)
	{
		GetWorldTimerManager().SetTimer(
			RagdollTimer,
			this,
			&AEnemy::RagdollEnd,
			RagdollTime
		);
	}
}

void AEnemy::RagdollUpdate()
{
	// Set the last ragdoll velocity and speed
	LastRagdollVelocity = GetMesh()->GetPhysicsLinearVelocity(FName("root"));
	double LastRagdollSpeed = LastRagdollVelocity.Length();
	
	// Use the ragdoll velocity to scale the ragdoll's joint strength for physical animation
	float InSpring = FMath::GetMappedRangeValueClamped<float>(
		TRange<float>(0.f, 1000.f), 
		TRange<float>(0.f, 25'000.f),
		LastRagdollSpeed
	);
	
	GetMesh()->SetAllMotorsAngularDriveParams(
		InSpring,
		0.f,
		0.f,
		false	
	);
	
	// Disable gravity if falling faster than -4000 to prevent
	// continual acceleration.
	// This also prevents the ragdoll from going through the floor
	GetMesh()->SetEnableGravity(LastRagdollVelocity.Z > -4000 ? true : false);
	
	// Update the actor location to follow the ragdoll
	SetActorLocationDuringRagdoll();
}

void AEnemy::SetActorLocationDuringRagdoll()
{
	// Set the pelvis as the target location
	FVector TargetRagdollLocation;
	
	TargetRagdollLocation = GetMesh()->GetSocketLocation(FName("pelvis"));

	// Determine wether the ragdoll is facing up or down and set the target rotation accordingly
	FRotator PelvisRotation = GetMesh()->GetSocketRotation(FName("pelvis"));
	
	//GetMesh()->GetSocketWorldLocationAndRotation(FName("pelvis"), TargetRagdollLocation, PelvisRotation);//GetSocketRotation(FName("pelvis"));

	// FacingUp
	if (PelvisRotation.Roll < 0.f)
	{
		bRagdollFaceUp = true;
	}
	else
	{
		bRagdollFaceUp = false;
	}
	if (bRagdollFaceUp)
	{
		TargetRagdollRotation = FRotator(0.f, PelvisRotation.Yaw - 180.f, 0.f);
	}
	else
	{
		TargetRagdollRotation = FRotator(0.f, PelvisRotation.Yaw, 0.f);
	}
	
	// Trace downwards from the target location to offset the target location, 
	// preventing the lower half of the capsule from going through the floor 
	// when the ragdoll is laying on the ground
	float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FVector TraceEnd = FVector(
		TargetRagdollLocation.X, 
		TargetRagdollLocation.Y, 
		TargetRagdollLocation.Z - CapsuleHalfHeight
	);

	FHitResult OutHitResult;

	GetWorld()->LineTraceSingleByChannel(
		OutHitResult,
		TargetRagdollLocation,
		TraceEnd,
		ECollisionChannel::ECC_Visibility
	);

	if (OutHitResult.bBlockingHit)
	{
		bRagdollOnGround = true;
		float LocationOffset = CapsuleHalfHeight - FMath::Abs((OutHitResult.ImpactPoint.Z - OutHitResult.TraceStart.Z));

		FVector NewLocation = FVector(
				TargetRagdollLocation.X,
				TargetRagdollLocation.Y,
				TargetRagdollLocation.Z - LocationOffset + 2.f
		);
		
		//SetActorLocation(NewLocation);
		SetActorLocationAndRotationUpdateTarget(NewLocation, TargetRagdollRotation, false, false);
	}
	else
	{
		bRagdollOnGround = true;
		float LocationOffset = CapsuleHalfHeight - FMath::Abs((OutHitResult.ImpactPoint.Z - OutHitResult.TraceStart.Z));

		FVector NewLocation = FVector(
			TargetRagdollLocation.X,
			TargetRagdollLocation.Y,
			TargetRagdollLocation.Z - LocationOffset + 2.f
		);
		//SetActorLocation(NewLocation);
		SetActorLocationAndRotationUpdateTarget(NewLocation, TargetRagdollRotation, false, false);
	}
}

void AEnemy::SetActorLocationAndRotationUpdateTarget(FVector NewLocation, FRotator NewRotation, bool bSweep, bool bTeleport)
{
	//TargetRagdollRotation = NewRotation;
	SetActorLocationAndRotation(NewLocation, TargetRagdollRotation);
}

void AEnemy::RagdollEnd()
{
	MeshDefaultRotaion = GetMesh()->GetRelativeRotation();
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	// Save a snapshot of the current Ragdoll pose for use in AnimGraph to blend out of the ragdoll
	if (AnimInstance)
	{
		AnimInstance->SavePoseSnapshot(FName("RagdollPose"));
	}

	if (bRagdollOnGround)
	{
		// TODO:
		// If the ragdoll is on the ground, set the movement mode to walking and play a GetUp animation.
		if (bRagdollFaceUp)
		{
			if (AnimInstance &&
				GetUpAnimationFaceUp)
			{
				AnimInstance->Montage_Play(GetUpAnimationFaceUp, 1.f);
			}
			else
			{
				Die();
			}
		}
		else
		{
			if (AnimInstance &&
				GetUpAnimationFaceDown)
			{
				AnimInstance->Montage_Play(GetUpAnimationFaceDown, 1.f);
			}
			else
			{
				Die();
			}
		}
	}
	else
	{
		// TODO:
		// If not, set the movement mode to falling and update the character movement velocity to match the
		// last ragdoll velocity
	}

	// Re-Enable capsule collision and disable physics simulation on the mesh
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetAllBodiesSimulatePhysics(false);
	GetMesh()->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepWorldTransform);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
}

void AEnemy::ResetMeshLocationAndRotation()
{
	bRotateMesh = true;
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	/*GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));*/
	bIsRagdoll = false;
}

void AEnemy::PlayKnockBackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance &&
		KnockBackMontage)
	{
		AnimInstance->Montage_Play(KnockBackMontage, 1.f);
	}
}

void AEnemy::StartSlowMotion()
{
	GetWorldTimerManager().SetTimer(
		SlowMotionResetTimer,
		this,
		&AEnemy::StopSlowMotion,
		SlowMotionResetTime
	);

	UGameplayStatics::SetGlobalTimeDilation(this, 0.25f);
}

void AEnemy::StopSlowMotion()
{
	UGameplayStatics::SetGlobalTimeDilation(this, 1.f);
	FinishDeath();
}

void AEnemy::UpdatePlayerKillCounter(APawn* Shooter)
{
	VRShooterCharacter = VRShooterCharacter == nullptr ? Cast<AVRShooterCharacter>(Shooter) : VRShooterCharacter;

	// Updating the KillCounter in the Character class
	if (VRShooterCharacter &&
		!bKillCounterUpdated)
	{
		VRShooterCharacter->UpdateKillCounter(1);
		bKillCounterUpdated = true;
	}
}

void AEnemy::FinishDeath()
{
	GetMesh()->bPauseAnims = true;

	GetWorldTimerManager().SetTimer(
		DeathTimer,
		this,
		&AEnemy::SpawnPickup,
		DeathTime
	);
}

void AEnemy::SpawnPickup()
{
	const float SpawnPickup = FMath::FRandRange(0.f, 1.f);

	if (SpawnPickup <= SpawnPickupChance)
	{
		if (PickupSpawnerClasses.Num() > 0)
		{
			for (auto PickupSpawnerClass : PickupSpawnerClasses)
			{				
				FVector EnemyLocation = FVector(GetActorLocation().X + FMath::RandRange(10.f, 300.f) , GetActorLocation().Y + FMath::RandRange(10.f, 300.f), GetActorLocation().Z);
				FTransform SpawnedPickupTransform = FTransform(FQuat(GetActorRotation()), FVector(EnemyLocation.X, EnemyLocation.Y, 268), FVector(1.f));
				SpawnedPickup = GetWorld()->SpawnActor<APickupSpawner>(PickupSpawnerClass, SpawnedPickupTransform);
			}
		}
	}
	DestroyEnemy();
}

void AEnemy::DestroyEnemy()
{
	if (DeathNiagara)
	{
		SpawnBloodNiagara(DeathNiagara, GetMesh()->GetComponentLocation());
		//UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		//	this,
		//	DeathNiagara,
		//	GetMesh()->GetComponentLocation(),
		//	GetActorRotation()
		//);
	}

	if (DeathCameraShake)
	{
		UGameplayStatics::PlayWorldCameraShake(
			GetWorld(),
			DeathCameraShake,
			GetActorLocation(),
			1000.f,
			1000.f
		);
	}

	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			DeathSound,
			GetActorLocation()
		);
	}

	Destroy();
}