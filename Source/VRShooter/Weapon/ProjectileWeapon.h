#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

UCLASS()
class VRSHOOTER_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	void Fire(const FVector& HitTarget) override;
};
