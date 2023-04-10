#include "MagicProjectile.h"
#include "Components/BoxComponent.h"
#include "VRShooter/Enemy/Enemy.h"

void AMagicProjectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AMagicProjectile::OnProjecileOverlap);
	CollisionBox->OnComponentEndOverlap.AddDynamic(this, &AMagicProjectile::OnProjectileEndOverlap);
}

void AMagicProjectile::OnProjecileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bfromSweep, const FHitResult& SweepResult)
{
	AEnemy* Enemy = Cast<AEnemy>(OtherActor);

	if (Enemy)
	{
		UE_LOG(LogTemp, Error, TEXT("OnProjecileOverlap %s:"), *Enemy->GetName());
	}
}

void AMagicProjectile::OnProjectileEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AEnemy* Enemy = Cast<AEnemy>(OtherActor);

	if (Enemy)
	{
		UE_LOG(LogTemp, Error, TEXT("OnProjectileEndOverlap %s:"), *Enemy->GetName());
	}
}

