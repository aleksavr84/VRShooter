#include "ProjectileGrenade.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/BoxComponent.h"
#include "VRShooter/Enemy/Enemy.h"
#include "GameFramework/ProjectileMovementComponent.h"

AProjectileGrenade::AProjectileGrenade()
{
    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Greanade Mesh"));
    ProjectileMesh->SetupAttachment(RootComponent);
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileGrenade::BeginPlay()
{
    AActor::BeginPlay();

    StartDestroyTimer();
    SpawnTrailSystem();

    ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce);
    CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileGrenade::OnHit);
}

void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
    AActor* HitActor = ImpactResult.GetActor();

    if (BounceSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            BounceSound,
            GetActorLocation()
        );
    }
}

void AProjectileGrenade::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    AEnemy* Enemy = Cast<AEnemy>(Hit.GetActor());

    if (Enemy && Hit.bBlockingHit)
    {
        Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
    }
}

void AProjectileGrenade::Destroyed()
{
    ExplodeDamage();
    Super::Destroyed();
}