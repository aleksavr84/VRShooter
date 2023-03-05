#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "VRShooterCharacter.h"
#include "WidgetActor.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"

AEnemy::AEnemy()
{
 	PrimaryActorTick.bCanEverTick = true;

	HealthBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBar->SetupAttachment(GetRootComponent());
	HealthBar->SetVisibility(false);

	//AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	//AreaSphere->SetupAttachment(GetRootComponent());
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	//AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnSphereOverlap);
	//AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::OnSphereEndOverlap);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
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

//void AEnemy::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bfromSweep, const FHitResult& SweepResult)
//{
//	if (OtherActor)
//	{
//		VRShooterCharacter = VRShooterCharacter == nullptr ? Cast<AVRShooterCharacter>(OtherActor) : VRShooterCharacter;
//
//		if (VRShooterCharacter)
//		{
//			HealthBar->SetVisibility(true);
//			bShouldRotateTheHealthBar = true;
//		}
//	}
//}
//
//void AEnemy::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
//{
//	if (OtherActor)
//	{
//		VRShooterCharacter = VRShooterCharacter == nullptr ? Cast<AVRShooterCharacter>(OtherActor) : VRShooterCharacter;
//
//		if (VRShooterCharacter)
//		{
//			HealthBar->SetVisibility(false);
//			bShouldRotateTheHealthBar = false;
//		}
//	}
//}

void AEnemy::BulletHit_Implementation(FHitResult HitResult, AVRShooterCharacter* CauserCharacter)
{
	VRShooterCharacter = VRShooterCharacter == nullptr ? Cast<AVRShooterCharacter>(CauserCharacter) : VRShooterCharacter;

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			ImpactSound,
			GetActorLocation()
		);
	}

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

	ShowHealthBar();
	PlayHitMontage(FName("HitReactFront"));
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
	if (Health - DamageAmount <= 0.f)
	{
		Health = 0.f;

		Die();

	}
	else
	{
		Health -= DamageAmount;
	}

	return Health;
}

void AEnemy::ResetHitReactTimer()
{
	bCanHitReact = true;
}

void AEnemy::Die()
{
	HideHealthBar();
	PlayHitMontage(FName("DeathA"));
}

