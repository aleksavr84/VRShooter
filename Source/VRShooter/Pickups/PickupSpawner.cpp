#include "PickupSpawner.h"
#include "Item.h"

void APickupSpawner::SpawningTheActor()
{
	Super::SpawningTheActor();

	int32 NumPickupClasses = ItemClasses.Num();

	if (NumPickupClasses > 0)
	{
		int32 Selection = FMath::RandRange(0, NumPickupClasses - 1);

		FTransform Transform;

		if (bAddImpulseAfterSpawning)
		{
			SpawnedItem = GetWorld()->SpawnActor<AItem>(ItemClasses[Selection], Transform);
			SpawnedItem->SetItemState(EItemState::EIS_Falling);
			SpawnedItem->ThrowItem();
		}
		else
		{
			Transform = GetActorTransform();
			SpawnedItem = GetWorld()->SpawnActor<AItem>(ItemClasses[Selection], Transform);
		}

		if (SpawnedItem &&
			bSouldRespawnAfterPickedUp)
		{
			SpawnedItem->OnDestroyed.AddDynamic(this, &APickupSpawner::StartSpawnTimer);
		}
	}
}

void APickupSpawner::StartSpawnTimer(AActor* DestroyedActor)
{
	Super::StartSpawnTimer(DestroyedActor);
}
