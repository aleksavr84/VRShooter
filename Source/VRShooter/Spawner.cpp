#include "Spawner.h"

ASpawner::ASpawner()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ASpawner::BeginPlay()
{
	Super::BeginPlay();
	
	StartSpawnTimer((AActor*)nullptr);
}

void ASpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASpawner::StartSpawnTimer(AActor* DestroyedActor)
{
	const float SpawnTime = FMath::FRandRange(SpawnTimeMin, SpawnTimeMax);
	GetWorldTimerManager().SetTimer(
		SpawnPickupTimer,
		this,
		&ASpawner::SpawnTimerFinished,
		SpawnTime
	);
}

void ASpawner::SpawnTimerFinished()
{
	SpawningTheActor();
}

void ASpawner::SpawningTheActor()
{
	
}

