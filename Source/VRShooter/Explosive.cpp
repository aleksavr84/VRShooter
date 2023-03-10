#include "Explosive.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "Camera/CameraShakeBase.h"
#include "TimerManager.h"
#include "Enemy.h"

AExplosive::AExplosive()
{
 	PrimaryActorTick.bCanEverTick = true;

	ExplosiveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExplosiveMesh"));
	SetRootComponent(ExplosiveMesh);

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(GetRootComponent());
}

void AExplosive::BeginPlay()
{
	Super::BeginPlay();
	
}

void AExplosive::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AExplosive::BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* ShooterController)
{
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			ImpactSound,
			GetActorLocation()
		);
	}

	if (ExplodeParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ExplodeParticles,
			HitResult.Location,
			FRotator(0.f),
			true
		);
	}

	// Apply explosive damage
	//TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

	for (auto Actor : OverlappingActors)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Actor damaged by explosive: %s"), *Actor->GetName());

		if (bIsSlowMotion)
		{
			StartSlowMotion(Actor);
			
			/*AEnemy* Enemy = Cast<AEnemy>(Actor);
			
			if (Enemy)
			{
				Enemy->SetStunned(true);
			}*/
		}

		if (bIsCausingDamage)
		{
			UGameplayStatics::ApplyDamage(
				Actor,
				Damage,
				ShooterController,
				Shooter,
				UDamageType::StaticClass()
			);
		}
	}

	PlayCameraShake();

	if (!bIsSlowMotion)
	{ 
		Destroy();
	}
	else
	{
		SetActorHiddenInGame(true);
	}
}

void AExplosive::StartSlowMotion(AActor* Actor)
{
	GetWorldTimerManager().SetTimer(
		SlowMotionResetTimer,
		this,
		&AExplosive::StopSlowMotion,
		SlowMotionResetTime
	);

	Actor->CustomTimeDilation = 0.1f;
}

void AExplosive::StopSlowMotion()
{
	for (auto Actor : OverlappingActors)
	{
		if (Actor)
		{
			Actor->CustomTimeDilation = 1.f;
		}
	}
	Destroy();
}

void AExplosive::PlayCameraShake()
{
	if (ExplosionCameraShake)
	{
		UGameplayStatics::PlayWorldCameraShake(
			GetWorld(),
			ExplosionCameraShake,
			GetActorLocation(),
			1000.f,
			1000.f
		);
	}
}
