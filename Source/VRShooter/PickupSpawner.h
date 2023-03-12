#pragma once

#include "CoreMinimal.h"
#include "Spawner.h"
#include "PickupSpawner.generated.h"

UCLASS()
class VRSHOOTER_API APickupSpawner : public ASpawner
{
	GENERATED_BODY()

public:
	virtual void SpawningTheActor() override;

protected:
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class AItem>> ItemClasses;

	UPROPERTY()
	AItem* SpawnedItem;

	virtual void StartSpawnTimer(AActor* DestroyedActor) override;
};
