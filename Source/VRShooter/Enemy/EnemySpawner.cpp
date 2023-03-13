#include "EnemySpawner.h"
#include "Enemy.h"

void AEnemySpawner::SpawningTheActor()
{
	Super::SpawningTheActor();

	int32 NumPickupClasses = EnemyClasses.Num();

	if (NumPickupClasses > 0)
	{
		int32 Selection = FMath::RandRange(0, NumPickupClasses - 1);
		SpawnedEnemy = GetWorld()->SpawnActor<AEnemy>(EnemyClasses[Selection], GetActorTransform());

		if (SpawnedEnemy)
		{
			SpawnedEnemy->OnDestroyed.AddDynamic(this, &AEnemySpawner::StartSpawnTimer);
		}
	}
}

void AEnemySpawner::StartSpawnTimer(AActor* DestroyedActor)
{
	Super::StartSpawnTimer(DestroyedActor);
}
