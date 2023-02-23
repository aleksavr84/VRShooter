#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRSHOOTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	friend class AVRShooterCharacter;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Firing
	void FireButtonPressed();
	void FireButtonReleased();
	void StartFireTimer();
	
	UFUNCTION()
	void AutoFireReset();

	void FireWeapon();

	// Line trace for Items under the crosshairs
	bool TraceUnderCrosshairs(FHitResult& OutHitResult);

	// Weapon
	class AWeapon* SpawnDefaultWeapon();
	void EquipWeapon(AWeapon* WeaponToEquip);
	void DropWeapon();
	// Drops currently equipped Weapon and Equips TraceHitItem
	void SwapWeapon(AWeapon* WeaponToSwap);

private:
	UPROPERTY()
	class AVRShooterCharacter* Character;

	// Weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AWeapon* EquippedWeapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> DefaultWeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;

	// Smoke Trail for bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;

	// Combat
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	float CameraDefaultFOV;
	float CameraZoomedFOV;

	bool bIsEquipped = false;

	// Automatic Fire
	bool bFireButtonPressed = false;
	bool bShouldFire = true;
	float AutomaticFireRate = 0.25f;
	FTimerHandle AutoFireTimer;

public:
	UFUNCTION(BlueprintCallable, meta = (AllowPrivateAccess = "true"))
	FORCEINLINE bool GetIsEquipped() { return bIsEquipped; }
};
