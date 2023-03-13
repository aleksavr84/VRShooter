#pragma once

#include "CoreMinimal.h"
#include "VRShooter/Pickups/Item.h"
#include "Types.h"
#include "Engine/DataTable.h"
#include "Weapon.generated.h"

USTRUCT(BlueprintType)
struct FWeaponDataTable : public FTableRowBase
{
	GENERATED_BODY()

	// Weapon Details -> Initialization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Initialization")
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Initialization")
	EFireType FireType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Initialization")
	TSubclassOf<class AProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Initialization")
	TSubclassOf<class ACasing> CasingClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Initialization")
	EAmmoType AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Initialization")
	int32 WeaponAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Initialization")
	int32 MagazineCapacity;

	// Inventory
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	UTexture2D* InventoryIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	UTexture2D* AmmoIcon;

	// Meshes and MaterialInstances
	UPROPERTY(EditAnywhere, Category = "Meshes and Materials")
	USkeletalMesh* ItemMesh;

	UPROPERTY(EditAnywhere, Category = "Meshes and Materials")
	UMaterialInstance* MaterialInstance;

	UPROPERTY(EditAnywhere, Category = "Meshes and Materials")
	int32 MaterialIndex;

	// Animations and Montages
	UPROPERTY(EditAnywhere, Category = "Animations and Montages")
	TSubclassOf<UAnimInstance> AnimBP;

	UPROPERTY(EditAnywhere, Category = "Animations and Montages")
	class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere, Category = "Animations and Montages")
	class UAnimationAsset* ReloadAnimation;

	UPROPERTY(EditAnywhere, Category = "Animations and Montages")
	class UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category = "Animations and Montages")
	float WeaponReloadAnimLength;

	UPROPERTY(EditAnywhere, Category = "Animations and Montages")
	FName ReloadMontageSection;

	// VFX
	UPROPERTY(EditAnywhere, Category = "Visual Effects")
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, Category = "Visual Effects")
	class UNiagaraSystem* MuzzleFlashNiagara;

	UPROPERTY(EditAnywhere, Category = "Visual Effects")
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, Category = "Visual Effects")
	UParticleSystem* BeamParticles;

	// SFX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Effects")
	class USoundCue* PickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Effects")
	USoundCue* EquipSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Effects")
	USoundCue* FireSound;

	// Bones and Sockets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bones and Sockets")
	FName HandSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bones and Sockets")
	FName MuzzleFlashSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bones and Sockets")
	FName ClipBoneName;

	// Weapon Properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties")
	bool bAutomatic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties")
	float AutoFireRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties")
	float HeadShotDamage;

	// Crosshairs
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshairs")
	UTexture2D* CrosshairsMiddle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshairs")
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshairs")
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshairs")
	UTexture2D* CrosshairsBottom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshairs")
	UTexture2D* CrosshairsTop;
};

UCLASS()
class VRSHOOTER_API AWeapon : public AItem
{
	GENERATED_BODY()
	
public:
	AWeapon();
	virtual void Tick(float DeltaTime) override;

protected:
	void StopFalling();
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	// DataTable for weapon properties
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Initialization", meta = (AllowPrivateAccess = "true"))
	UDataTable* WeaponDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Initialization", meta = (AllowPrivateAccess = "true"))
	EWeaponType WeaponType = EWeaponType::EWT_Pistol;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Initialization", meta = (AllowPrivateAccess = "true"))
	EFireType FireType = EFireType::EFT_HitScan;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Initialization", meta = (AllowPrivateAccess = "true"))
	class TSubclassOf<AProjectile> ProjectileClass;

	UPROPERTY(VisibleAnywhere, Category = "Initialization")
	TSubclassOf<class ACasing> CasingClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Initialization", meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType = EAmmoType::EAT_9mm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Initialization", meta = (AllowPrivateAccess = "true"))
	int32 Ammo = 0;

	// Maximum ammo that our weapon can hold
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Initialization", meta = (AllowPrivateAccess = "true"))
	int32 MagazineCapacity = 30;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animations and Montages", meta = (AllowPrivateAccess = "true"))
	class UAnimationAsset* FireAnimation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animations and Montages", meta = (AllowPrivateAccess = "true"))
	class UAnimationAsset* ReloadAnimation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animations and Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ReloadMontage;

	// Reload without ReloadMontage
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animations and Montages", meta = (AllowPrivateAccess = "true"))
	float WeaponReloadAnimLength = 1.73f;

	/// 
	///  VFX
	/// 
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Visual Effects", meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Visual Effects", meta = (AllowPrivateAccess = "true"))
	class UNiagaraSystem* MuzzleFlashNiagara;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Visual Effects", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;

	// Smoke Trail for bullets
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Visual Effects", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Effects", meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound;

	// Name for the clip bone
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bones and Sockets", meta = (AllowPrivateAccess = "true"))
	FName ClipBoneName = TEXT("Clip_Bone");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bones and Sockets", meta = (AllowPrivateAccess = "true"))
	FName MuzzleFlashSocketName = TEXT("Muzzle");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bones and Sockets", meta = (AllowPrivateAccess = "true"))
	FName HandSocketName = TEXT("RightHandSocket");

	// FName for the Reload Montage Section; 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bones and Sockets", meta = (AllowPrivateAccess = "true"))
	FName ReloadMontageSection = FName(TEXT("Default"));
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	float AutomaticFireRate = 0.25f;

	// Amount of damage caused by a bullet
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	float Damage = 25.f;

	// Amount of damage when a bullet hits the head
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	float HeadShotDamage = 50.f;

	// true when moving the clip while realoading
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	bool bMovingClip = false;

	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool bFalling;
	int32 PreviousMaterialIndex;

public:
	// Adds an impulse to the Weapon
	void ThrowWeapon();
	virtual void Fire(const FVector& HitTarget);
	void Reload();
	
	// Called from CombatComponent when firing weapon
	void DecrementAmmo();

	FORCEINLINE void SetProjectileClass(TSubclassOf<AProjectile> Projectile) { ProjectileClass = Projectile; }
	FORCEINLINE TSubclassOf<AProjectile> GetProjectileClass() const { return ProjectileClass; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	FORCEINLINE void SetReloadMontageSection(FName Name) { ReloadMontageSection = Name; }
	FORCEINLINE FName GetReloadMontageSection() const { return ReloadMontageSection; }
	FORCEINLINE int32 GetMagazineCapacity() const { return MagazineCapacity; }
	FORCEINLINE FName GetClipBoneName() const { return ClipBoneName; }
	FORCEINLINE void SetMovingClip(bool Move) { bMovingClip = Move; }
	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage; }
	FORCEINLINE void SetClipBoneName(FName Name) { ClipBoneName = Name; }
	FORCEINLINE FName GetHandSocketName() const { return HandSocketName; }
	FORCEINLINE EFireType GetFireType() const { return FireType; }
	FORCEINLINE UParticleSystem* GetMuzzleFlash() const { return MuzzleFlash; }
	FORCEINLINE UNiagaraSystem* GetMuzzleFlashNiagara() const { return MuzzleFlashNiagara; }
	FORCEINLINE UParticleSystem* GetImpactParticles() const { return ImpactParticles; }
	FORCEINLINE UParticleSystem* GetBeamParticles() const { return BeamParticles; }
	FORCEINLINE USoundCue* GetFireSound() const { return FireSound; }
	FORCEINLINE FName GetMuzzleFlashSocketName() const { return MuzzleFlashSocketName; }
	FORCEINLINE float GetAutomaticFireRate() const { return AutomaticFireRate; }
	FORCEINLINE float GetWeaponReloadAnimLength() const { return WeaponReloadAnimLength; }

	const USkeletalMeshSocket* GetMuzzleFlashSocket();

	void ReloadAmmo(int32 Amount);
	
};
