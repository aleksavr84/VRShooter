#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HandController.h"
#include "VRShooterCharacter.generated.h"

USTRUCT(BlueprintType)
struct FInterpLocation
{
	GENERATED_BODY()

	// Scene Component to use for it's location for interping
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SceneComponent;

	// Number of items interping to/at this scene component location
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ItemCount;
};

UCLASS()
class VRSHOOTER_API AVRShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AVRShooterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;

	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser
	);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Debug, meta = (AllowPrivateAccess = "true"))
	float FrameRate = 0.f;

	void Die();
	void StartFade(float FromAlpha, float ToAlpha, float FadeTime);

protected:
	virtual void BeginPlay() override;
	void TraceForItems();

	void InitializeInterpLocations();

	UFUNCTION(BlueprintCallable)
	void FinishDeath();

private:
	// Camera Shake on taking damage
	void PlayCameraShake();

	// PostProcessing
	void UpdateBlinkers();
	FVector2D GetBlinkerCenter();

	// Movement
	void MoveForward(float Value);
	void MoveRight(float Value);
	
	// Actions
	void GripLeft() { LeftController->Grip(); }
	void ReleaseLeft() { LeftController->Release(); }
	void GripRight() { RightController->Grip(); }
	void ReleaseRight() { RightController->Release(); }
	void FireButtonPressed();
	void FireButtonReleased();
	void ReloadButtonPressed();
	void SelectButtonPressed();
	void SelectButtonReleased();
	void XButtonPressed();
	void FKeyPressed();
	void OneKeyPressed();
	void TwoKeyPressed();
	void ThreeKeyPressed();
	void FourKeyPressed();
	void FiveKeyPressed();
	void SwitchInventoryItem(float Value);

	// Teleport
	void BeginTeleport();
	void FinishTeleport();
	bool FindTeleportDestination(TArray<FVector>& OutPath, FVector& OutLocation);
	void UpdateDestinationMarker();
	void DrawTeleportPath(const TArray<FVector>& Path);
	void UpdateSpline(const TArray<FVector>& Path);

	// Tracing

	//// The item currently hit by our trace in TraceForItems (could be null!)
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	//class AItem* TraceHitItem;
	
	bool bShouldTraceForItems = false;
	int8 OverlappedItemCount;

	UFUNCTION()
	void OnRightWeaponOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bfromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnRightWeaponEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	UFUNCTION(BlueprintCallable)
	void ActivateRightWeaponCollision();

	UFUNCTION(BlueprintCallable)
	void DeactivateRightWeaponCollision();

	UFUNCTION()
	void OnLeftWeaponOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bfromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION(BlueprintCallable)
	void ActivateLeftWeaponCollision();

	UFUNCTION(BlueprintCallable)
	void DeactivateLeftWeaponCollision();

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Initialization, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* BodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Initialization, meta = (AllowPrivateAccess = "true"))
	class AHandController* LeftController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Initialization, meta = (AllowPrivateAccess = "true"))
	AHandController* RightController;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName LeftWeaponSocket = TEXT("LeftWeaponBone");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName RightWeaponSocket = TEXT("RightWeaponBone");

	// Collision volume for the left weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* LeftWeaponCollision;

	// Collision volume for the right weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UBoxComponent* RightWeaponCollision;

	UPROPERTY(VisibleAnywhere)
	class USceneComponent* VRRoot;

	UPROPERTY(VisibleAnywhere, Category = Initialization)
	class USplineComponent* TeleportPath;

	UPROPERTY(VisibleAnywhere, Category = Initialization)
	class UStaticMeshComponent* DestinationMarker;

	UPROPERTY()
	class UPostProcessComponent* PostProcessComponent;

	UPROPERTY()
	class UMaterialInstanceDynamic* BlinkerMaterialInstance;

	UPROPERTY()
	TArray<class USplineMeshComponent*> TeleportPathMeshPool;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WeaponInterpComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp4;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp6;

	// Array of interp location structs
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FInterpLocation> InterpLocations;

	FTimerHandle PickupSoundTimer;
	FTimerHandle EquipSoundTimer;

	bool bShouldPlayPickupSound = true;
	bool bShouldPlayEquipSound = true;

	void ResetPickupSoundTimer();
	void ResetEquipSoundTimer();

	// Initialization
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Initialization, meta = (AllowPrivateAccess = "true"))
	class USoundClass* MasterSoundClass;

	// CameraShake for TakeDamge
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Initialization, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UCameraShakeBase> TakeDamageCameraShake;

	// Time to wait before we can play another PickupSound
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Initialization, meta = (AllowPrivateAccess = "true"))
	float PickupSoundResetTime = .2f;

	// Time to wait before we can play another EquipSound
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Initialization, meta = (AllowPrivateAccess = "true"))
	float EquipSoundResetTime = .2f;

	// Teleport
	UPROPERTY(EditDefaultsOnly, Category = Initialization)
	class UStaticMesh* TeleportArchMesh;

	UPROPERTY(EditDefaultsOnly, Category = Initialization)
	class UMaterialInterface* TeleportArchMaterial;

	UPROPERTY(EditAnywhere, Category = Initialization)
	float TeleportProjectileRadius = 10.f;

	UPROPERTY(EditAnywhere, Category = Initialization)
	float TeleportProjectileSpeed = 1000.f;

	UPROPERTY(EditAnywhere, Category = Initialization)
	float TeleportSimulationTime = 2.f;

	UPROPERTY(EditAnywhere, Category = Initialization)
	float TeleportFadeTime = 1.f;

	UPROPERTY(EditAnywhere, Category = Initialization)
	FVector TeleportProjectionExtent = FVector(100, 100, 100);

	// Blinker
	UPROPERTY(EditAnywhere, Category = Initialization)
	class UMaterialInterface* BlinkerMaterialBase;

	UPROPERTY(EditAnywhere, Category = Initialization)
	class UCurveFloat* RadiusVsVelocity;

	UPROPERTY(EditAnywhere, Category = Initialization)
	float ControllerRotation = -90.f;

	UPROPERTY(EditDefaultsOnly, Category = Initialization)
	TSubclassOf<AHandController> HandControllerClass;

	// Object References
	class APlayerController* PlayerController;
	class AEnemyController* EnemyController;

	// CameraInterp
	// Distance outward from the camera for the interp destination
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Initialization, meta = (AllowPrivateAccess = "true"))
	float CameraInterpDistance = 250.f;

	// Distance upward from the camera for the interp destination
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Initialization, meta = (AllowPrivateAccess = "true"))
	float CameraInterpElevation = 65.f;

	// HUD
	UPROPERTY(EditDefaultsOnly, Category = Initialization)
	TSubclassOf<class AVRHUD> VRHUDClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Initialization, meta = (AllowPrivateAccess = "true"))
	class AVRHUD* VRHUD;

	bool bWeaponHUDShowing = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Initialization, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* DeathMontage;

	bool bSwitchingInventoryItem = false;
	bool bIsFightingForLife = false;
	bool bIsDead = false;

public:
	FORCEINLINE UCameraComponent* GetCameraComponent() const { return Camera; }
	FORCEINLINE USkeletalMeshComponent* GetBodyMesh() const { return BodyMesh; }
	FORCEINLINE AHandController* GetRightHandController() const { return RightController; }
	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return Combat; }
	FORCEINLINE bool ShouldPlayPickupSound() const { return bShouldPlayPickupSound; }
	FORCEINLINE bool ShouldPlayEquipSound() const { return bShouldPlayEquipSound; }
	FORCEINLINE bool GetIsDead() const { return bIsDead; }
	FORCEINLINE void SetIsDead(bool bDead) { bIsDead = bDead; }
	FORCEINLINE bool GetIsFightingForLife() const { return bIsFightingForLife; }
	FORCEINLINE void SetIsFightingForLife(bool bFightingForLife) { bIsFightingForLife = bFightingForLife; }
	FORCEINLINE USoundClass* GetMasterSoundClass() const { return MasterSoundClass; }

	void StartPickupSoundTimer();
	void StartEquipSoundTimer();
	
	FInterpLocation GetInterpLoctaion(int32 Index);
	/*UWidgetComponent* GetHUDWidget() { return HUDWidget; }*/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bFireButtonPressed = false;

	void IncrementOverlappedItemCount(int8 Amount);

	// No longer needed; AItem has GetInterpLocation
	/*FVector GetCameraInterpLocation();*/

	void GetPickupItem(class AItem* Item);

	// WeaponHUD
	void ShowWeaponHUD();

	// Returns the index in InterpLocations array with lowest item count
	int32 GetInterpLocationIndex();

	void IncrementInterpLocItemCount(int32 Index, int32 Amount);

	class USoundCue* GetMeleeImpactSound();
	UParticleSystem* GetBloodParticles();
	class UNiagaraSystem* GetBloodNiagara();

	void UpdateKillCounter(int32 KillsToAdd);
	void UnHighlightInventorySlot();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetSoundPitch(float NewPitch, float FadeInTime);
};
