#pragma once

#include "CoreMinimal.h"
#include "Spawner.h"
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

	virtual void StartSpawnTimer(AActor* DestroyedActor) override;
};
