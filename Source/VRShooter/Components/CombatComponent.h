#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VRShooter/Weapon/Types.h"
#include "CombatComponent.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_FireTimerInProgress UMETA(DisplayName = "FireTimerInProgress"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_Equipping UMETA(DisplayName = "Equipping"),

	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquipItemDelegate, int32, CurrentSlotIndex, int32, NewSlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHighlightIconDelegate, int32, SlotIndex, bool, bStartAnimation);

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
	void PlayFireSound();
	void SendBullet();
	void PlayGunFireMontage();
	void PlayHapticEffect();
	void ReloadWeapon();

	void FightForYourLife();

	// Line trace for Items under the crosshairs
	bool TraceUnderCrosshairs(FHitResult& OutHitResult);

	// Weapon
	class AWeapon* SpawnDefaultWeapon();
	void EquipWeapon(AWeapon* WeaponToEquip, bool bSwapping = false);
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

	// Inventory
	void ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex);
	int32 GetEmptyInventorySlot();
	void HighlightInventorySlot();

	// Pickup
	void PickupAmmo(class AAmmo* Ammo);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AWeapon* EquippedWeapon;

	void UpdateHitCounter(int32 HitValue);
	void UpdateHitMultiplier(int32 MultiplierValue);
	void CalculateScore();

private:
	UPROPERTY()
	class AVRShooterCharacter* Character;

	// Weapon

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> DefaultWeaponClass;

	// Combat

	// The item currently hit by our trace in TraceForItems (could be null!)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	class AItem* TraceHitItem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	AItem* TraceHitItemLastFrame;

	// Map to keep track of ammo of the different ammo types
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TMap<EAmmoType, int32> AmmoMap;

	// Starting Ammount of 9mm ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	int32 Starting9mmAmmo = 85;

	// Starting Ammount of Assault Rifle ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	int32 StartingARAmmo = 120;

	// Starting Ammount of Grenade Launcher ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	int32 StartingGrenadeAmmo = 20;

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	// Montage for reload Animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* EquipMontage;

	UFUNCTION(BlueprintCallable)
	void FinishEquipping();

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
	FTimerHandle AutoFireTimer;
	
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* MeleeImpactSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BloodParticles;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UNiagaraSystem* BloodNiagara;

	// Counters for Hit, Multiplier and Combo
	
	// HitCounter
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"));
	int32 HitCounter = 0;

	FTimerHandle HitCounterResetTimer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HitCounterResetTime = 5.f;

	// HitMultiplier
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"));
	int32 HitMultiplier = 1;

	FTimerHandle HitMultiplierResetTimer;
	void ResetHitMultiplier();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HitMultiplierResetTime = 5.f;

	void StartHitMultiplierTimer();
	void ClearHitMultiplierTimer();

	// Widgets
	class AWidgetActor* SpawnWidgetActor(TSubclassOf<class AWidgetActor> WidgetActorClass, FVector LocationOffset);
	void ShowHideWidget(AWidgetActor* Widget, bool bShouldShow);

	// Announcements
	UPROPERTY(EditDefaultsOnly, Category = Initialization)
	TSubclassOf<AWidgetActor> AnnouncementActorClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Initialization, meta = (AllowPrivateAccess = "true"))
	AWidgetActor* AnnouncementWidgetActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Initialization, meta = (AllowPrivateAccess = "True"))
	FVector AnnouncementWidgetLocationOffset = FVector(100.f, -50.f, 0.f);

	// Fight for your life:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Initialization, meta = (AllowPrivateAccess = "True"))
	FString FightForYourLifeText = TEXT("Fight for your life bitch!");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Initialization, meta = (AllowPrivateAccess = "true"))
	USoundCue* FightForYourLifeSurviveSound;

	// KillCounter
	// Announcements - KillCounter
	UPROPERTY(EditDefaultsOnly, Category = Initialization)
	TSubclassOf<AWidgetActor> KillCounterWidgetActorClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Initialization, meta = (AllowPrivateAccess = "true"))
	AWidgetActor* KillCounterWidgetActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Initialization, meta = (AllowPrivateAccess = "True"))
	FVector KillCounterLocationOffset = FVector(100.f, 50.f, 0.f);

	// Important!!! the first array element should be an empty text!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Initialization, meta = (AllowPrivateAccess = "True"))
	TArray<FString> KillTexts{ TEXT(""), TEXT("Not Bad"), TEXT("Nice"), TEXT("Crazy"), TEXT("Brutal"), TEXT("Killmachine") };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Initialization, meta = (AllowPrivateAccess = "True"))
	int32 KillTextSeps = 3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"));
	int32 KillCounter = 0;

	FTimerHandle KillCounterResetTimer;
	void ResetKillCounter();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float KillCounterResetTime = 5.f;

	void StartKillCounterTimer();

	// Generate KillCounterText and update the KillCounterWidgetActor
	void GenerateKillCounterText(int32 Kills);
	
	// Player Score
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"));
	int32 PlayerScore = 0;

	// Inventory
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"));
	TArray<class AItem*> Inventory;

	const int32 INVENTORY_CAPACITY{ 6 };

	// Delegate for sending slot information to Inventory when equipping
	UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
	FEquipItemDelegate EquipItemDelegate;

	// Delegate for sending slot information to playing the icon animation
	UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
	FHighlightIconDelegate HighlightIconDelegate;

	// The index for the currently highlighted slot
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"));
	int32 HighlightedSlot = -1;

	FVector BeamEndPoint;

	// Fight for your life
	FTimerHandle FightForYourLifeTimer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float FightForYourLifeTime = 5.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float FightForYourLifeTimeRemaining = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HealtAfterSurvive = 70.f;

	void FightForYourLifeTimerStart();
	void FightForYourLifeTimerEnd();
	void FightForYourLifeReset();

public:
	UFUNCTION(BlueprintCallable, meta = (AllowPrivateAccess = "true"))
	FORCEINLINE bool GetIsEquipped() { return bIsEquipped; }

	FORCEINLINE USoundCue* GetMeleeImpactSound() const { return MeleeImpactSound; }
	FORCEINLINE UParticleSystem* GetBloodParticles() const { return BloodParticles; }
	FORCEINLINE UNiagaraSystem* GetBloodNiagara() const { return BloodNiagara; }
	FORCEINLINE int32 GetHitCounter() const { return HitCounter; }
	FORCEINLINE int32 GetHitMultiplier() const { return HitMultiplier; }
	FORCEINLINE int32 GetKillCounter() const { return KillCounter; }
	FORCEINLINE int32 GetPlayerScore() const { return PlayerScore; }

	void UpdateKillCounter(int32 KillsToAdd);

	// inventory
	void UnHighlightInventorySlot();
};
