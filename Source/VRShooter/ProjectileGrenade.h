#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileGrenade.generated.h"

UCLASS()
class VRSHOOTER_API AProjectileGrenade : public AProjectile
{
	GENERATED_BODY()
	
public:
	AProjectileGrenade();
	virtual void Destroyed() override;

protected:
	void BeginPlay() override;

	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

private:
	UPROPERTY(EditAnywhere)
	USoundCue* BounceSound;
};
