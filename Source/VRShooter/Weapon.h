#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Weapon.generated.h"

UCLASS()
class VRSHOOTER_API AWeapon : public AItem
{
	GENERATED_BODY()
	
public:
	AWeapon();
	virtual void Tick(float DeltaTime) override;

protected:
	void StopFalling();

private:
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	TSubclassOf<class ACasing> CasingClass;

	UPROPERTY(EditAnywhere, BLueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess= "true"))
	int32 Ammo = 0;

	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool bFalling;

public:
	// Adds an impulse to the Weapon
	void ThrowWeapon();
	void Fire();

	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	// Called from CombatComponent when firing weapon
	void DecrementAmmo();
};
