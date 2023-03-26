#pragma once

#include "CoreMinimal.h"
#include "VRShooter/Actors/Spawner.h"
#include "EnemySpawner.generated.h"

UCLASS()
class VRSHOOTER_API AEnemySpawner : public ASpawner
{
	GENERATED_BODY()

public:
	virtual void SpawningTheActor() override;

protected:
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class AEnemy>> EnemyClasses;

	UPROPERTY()
	AEnemy* SpawnedEnemy;

	TArray<AEnemy*> SpawnedEnemies;

	UPROPERTY(EditAnywhere)
	int32 MaxNumberOfSpawns = 3;

	int32 NumberOfSpawns = 0;

	UPROPERTY(EditAnywhere)
	bool bInfinitSpawn = false;

	virtual void StartSpawnTimer(AActor* DestroyedActor) override;
};
