#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "VRShooterCharacter.h"
#include "WidgetActor.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "DrawDebugHelpers.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Controller.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraShakeBase.h"

AEnemy::AEnemy()
{
 	PrimaryActorTick.bCanEverTick = true;

	HealthBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBar->SetupAttachment(GetRootComponent());
	HealthBar->SetVisibility(false);

	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());

	CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatRangeSphere"));
	CombatRangeSphere->SetupAttachment(GetRootComponent());

	// Left and Right WeaponCollisionBoxes
	LeftWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftWeaponCollision"));
	LeftWeaponCollision->SetupAttachment(GetMesh(), FName("LeftWeaponBone"));

	RightWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RightWeaponCollision"));
	RightWeaponCollision->SetupAttachment(GetMesh(), FName("RightWeaponBone"));

	// AI Smooth rotation
	bUseControllerRotationYaw = false;
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnAgroSphereOverlap);
	
	CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnCombatRangeSphereOverlap);
	CombatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::OnCombatRangeSphereEndOverlap);
	
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

		EnemyController->RunBehaviorTree(BehaviorTree);
	}
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bShouldRotateTheHealthBar)
	{
		if (VRShooterCharacter)
		{
			RotateWidgetToPlayer(HealthBar, VRShooterCharacter->GetActorLocation());
		}
	}
}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::OnAgroSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bfromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		auto Character = Cast<AVRShooterCharacter>(OtherActor);

		if (Character &&
			EnemyController &&
			EnemyController->GetBlackboardComponent())
		{
			// Set the value of the TargetBlackboardKey
			EnemyController->GetBlackboardComponent()->SetValueAsObject(
				TEXT("Target"), 
				Character
			);
		}
	}
}

void AEnemy::OnCombatRangeSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bfromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr) return;

	auto ShooterCharacter = Cast<AVRShooterCharacter>(OtherActor);

	if (ShooterCharacter)
	{
		bInAttackRange = true;

		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(
				TEXT("InAttackRange"), 
				true
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


void AEnemy::OnCombatRangeSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == nullptr) return;

	auto ShooterCharacter = Cast<AVRShooterCharacter>(OtherActor);

	if (ShooterCharacter)
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
	auto Character = Cast<AVRShooterCharacter>(OtherActor);
	
	if (Character)
	{
		DoDamage(Character);
		SpawnBloodParticles(Character, LeftWeaponSocket);
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
	auto Character = Cast<AVRShooterCharacter>(OtherActor);

	if (Character)
	{
		DoDamage(Character);
		SpawnBloodParticles(Character, RightWeaponSocket);
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
		}

		if (Victim->GetBloodNiagara())
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				this,
				Victim->GetBloodNiagara(),
				SocketTransform.GetLocation(),
				GetActorRotation()
			);
		}
	}
}

void AEnemy::DoDamage(AVRShooterCharacter* Victim)
{
	if (Victim == nullptr) return;

	UGameplayStatics::ApplyDamage(
		Victim,
		BaseDamage,
		EnemyController,
		this,
		UDamageType::StaticClass()
	);

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
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			BloodNiagara,
			HitResult.Location,
			GetActorRotation()
		);
	}

	if (bDying)
	{
		//// Updating the C
		//if (VRShooterCharacter)
		//{
		//	VRShooterCharacter->UpdateKillCounter(1);
		//}
		
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

	ShowHealthBar();

	// Determine whether bullet hit stunns
	const float Stunned = FMath::FRandRange(0.f, 1.f);

	if (Stunned <= StunChance)
	{
		// Stun the Enemy
		PlayHitMontage(FName("HitReactFront"));
		SetStunned(true);
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

	GetMesh()->BreakConstraint(Impulse, HitLocation, BoneToBreak);
	AddHitReactImpulse(Impulse, HitLocation, Bone, false);
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
	}
}

void AEnemy::ShowHitNumber(AVRShooterCharacter* Causer, int32 Damage, FVector HitLocation, bool bHeadShot)
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

void AEnemy::ShowHealthBar()
{
	GetWorldTimerManager().ClearTimer(HealthBarTimer);
	GetWorldTimerManager().SetTimer(
		HealthBarTimer, 
		this, 
		&AEnemy::HideHealthBar, 
		HealthBarDisplayTime
	);

	HealthBar->SetVisibility(true);
	bShouldRotateTheHealthBar = true;

}

void AEnemy::HideHealthBar()
{
	HealthBar->SetVisibility(false);
	bShouldRotateTheHealthBar = false;
}

void AEnemy::RotateWidgetToPlayer(UWidgetComponent* Widget, FVector PlayerLocation)
{
		FRotator WidgetRotation = HealthBar->GetRelativeRotation();
		FVector Direction = PlayerLocation - GetActorLocation();
		FRotator Rotation = FRotationMatrix::MakeFromX(Direction).Rotator();
		HealthBar->SetWorldRotation(FRotator(WidgetRotation.Pitch, Rotation.Yaw, WidgetRotation.Roll));
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!bDying)
	{
		// Set the target blackboard key to agro the character
		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsObject(
				FName("Target"),
				DamageCauser
			);
		}

		if (Health - DamageAmount <= 0.f)
		{
			Health = 0.f;

			UpdatePlayerKillCounter(EventInstigator->GetPawn());
			Die();

		}
		else
		{
			Health -= DamageAmount;
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

	bDying = true;

	HideHealthBar();

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance &&
		DeathMontage &&
		!bRagdollOnDeath)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}
	else
	{
		RagdollStart();
		FinishDeath();
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

void AEnemy::RagdollStart()
{
	EnemyController->StopMovement();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetAllBodiesBelowSimulatePhysics(FName("pelvis"), true, true);
	GetMesh()->GetAnimInstance()->StopAllMontages(0.2f);

	/*FVector ActorForwardVector = GetActorForwardVector();
	ActorForwardVector.Normalize(0.0001f);

	GetMesh()->AddImpulseAtLocation(ActorForwardVector * -7500.f, GetActorLocation(), FName("spine_02"));*/
}

void AEnemy::FinishDeath()
{
	GetMesh()->bPauseAnims = true;

	GetWorldTimerManager().SetTimer(
		DeathTimer,
		this,
		&AEnemy::DestroyEnemy,
		DeathTime
	);
}

void AEnemy::DestroyEnemy()
{
	if (DeathNiagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			DeathNiagara,
			GetActorLocation(),
			GetActorRotation()
		);
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

	Destroy();
}