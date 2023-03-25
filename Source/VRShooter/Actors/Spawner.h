#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Spawner.generated.h"

UCLASS()
class VRSHOOTER_API ASpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	ASpawner();

	virtual void SpawningTheActor();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void StartSpawnTimer(AActor* DestroyedActor);
	
	void SpawnTimerFinished();

private:
	FTimerHandle SpawnPickupTimer;

	UPROPERTY(EditAnywhere)
	float SpawnTimeMin = 1.f;

	UPROPERTY(EditAnywhere)
	float SpawnTimeMax = 10.f;

	// Sound to play when hit by bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* SpawnSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bShouldPlayPickupSound = true;

public:	
	virtual void Tick(float DeltaTime) override;

};
