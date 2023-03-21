#include "Explosive.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "Camera/CameraShakeBase.h"
#include "TimerManager.h"
#include "VRShooter/Enemy/Enemy.h"
#include "NiagaraFunctionLibrary.h"

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

	if (ExplodeParticlesNiagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			ExplodeParticlesNiagara,
			HitResult.Location,
			FRotator(0.f)
		);
	}

	// Apply explosive damage
	//TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

	for (auto Actor : OverlappingActors)
	{
		if (bIsSlowMotion)
		{
			StartSlowMotion(Actor);			
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

		AEnemy* Enemy = Cast<AEnemy>(Actor);

		if (Enemy)
		{
			float DistanceToEnemy = (Enemy->GetActorLocation() - GetActorLocation()).Length();
			
			FVector Impulse = Enemy->GetActorLocation() - GetActorLocation();
			Impulse.Normalize();

			// Calculate Falloff
			// radius beyond which full falloff happens
			const float FalloffRadius = OverlapSphere->GetScaledSphereRadius();
			// strength of the falloff effect
			const float FalloffExponent = 2.f;

			float Fraction = DistanceToEnemy / FalloffRadius;
			// clamp to ensure 0-1 range
			Fraction = FMath::Clamp(Fraction, 0.1f, 1.f);

			//float FalloffFactor = FMath::Pow(1 - Fraction, FalloffExponent);
			Impulse *= Fraction * 7'000.f;
			Enemy->AddHitReactImpulse(Impulse, Enemy->GetActorLocation(), FName("pelvis"), true);
			Enemy->SetRagdollTime(FMath::FRandRange(2.5f, 5.0f) * Fraction);
			Enemy->RagdollStart();
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
