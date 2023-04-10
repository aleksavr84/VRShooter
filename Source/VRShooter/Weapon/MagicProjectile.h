#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "MagicProjectile.generated.h"

UCLASS()
class VRSHOOTER_API AMagicProjectile : public AProjectile
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	void OnProjecileOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bfromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnProjectileEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

protected:
	virtual void BeginPlay() override;
};
