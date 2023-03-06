#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AmmoType.h"
#include "CombatComponent.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_FireTimerInProgress UMETA(DisplayName = "FireTimerInProgress"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),

	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRSHOOTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	friend class AVRShooterCharacter;

	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser
	);

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
	void PlayFireSound();
	void SendBullet();
	void PlayGunFireMontage();
	void PlayHapticEffect();
	void ReloadWeapon();

	// Line trace for Items under the crosshairs
	bool TraceUnderCrosshairs(FHitResult& OutHitResult);

	// Weapon
	class AWeapon* SpawnDefaultWeapon();
	void EquipWeapon(AWeapon* WeaponToEquip);
	void DropWeapon();
	// Drops currently equipped Weapon and Equips TraceHitItem
	void SwapWeapon(AWeapon* WeaponToSwap);
	// Initialize the AmmoMap with Ammo values
	void InitializeAmmoMap();
	// Check to make sure our waepon has ammo
	bool WeaponHasAmmo();
	// Checks to see if we have ammo of the EquippedWeapon Ammo Type
	bool CarryingAmmo();
	// Called from Animation Blueprint with GrabClip notify
	UFUNCTION(BlueprintCallable)
	void GrabClip();
	// Called from Animation Blueprint with RelaseClip notify
	UFUNCTION(BlueprintCallable)
	void ReleaseClip();

	// Pickup
	void PickupAmmo(class AAmmo* Ammo);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AWeapon* EquippedWeapon;

private:
	UPROPERTY()
	class AVRShooterCharacter* Character;

	// Weapon

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

	// Map to keep track of ammo of the different ammo types
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TMap<EAmmoType, int32> AmmoMap;

	// Starting Ammount of 9mm ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	int32 Starting9mmAmmo = 85;

	// Starting Ammount of Assault Rifle ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	int32 StartingARAmmo = 120;

	// Montage for reload Animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ReloadMontage;

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	// Transform of the clip when we first grab the clip during reloading
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FTransform ClipTransform;

	// Scene compoent to attach to the Character~s hand during reloading
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USceneComponent* HandSceneComponent;

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
	
	// Reload without ReloadMontage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float WeaponReloadAnimLength = 1.73f;

	FTimerHandle WeaponReloadTimer;
	void WeaponReloadAnimStart();
	void WeaponReloadAnimFinished();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"));
	ECombatState CombatState  = ECombatState::ECS_Unoccupied;

	// Character Health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float Health = 100.f;

	// Character MaxHealth
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float MaxHealth = 100.f;

public:
	UFUNCTION(BlueprintCallable, meta = (AllowPrivateAccess = "true"))
	FORCEINLINE bool GetIsEquipped() { return bIsEquipped; }
};
