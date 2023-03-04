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

protected:
	virtual void BeginPlay() override;
	void TraceForItems();

	void InitializeInterpLocations();

private:
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

	// Teleport
	void BeginTeleport();
	void StartFade(float FromAlpha, float ToAlpha);
	void FinishTeleport();
	bool FindTeleportDestination(TArray<FVector>& OutPath, FVector& OutLocation);
	void UpdateDestinationMarker();
	void DrawTeleportPath(const TArray<FVector>& Path);
	void UpdateSpline(const TArray<FVector>& Path);

	// Tracing

	// The item currently hit by our trace in TraceForItems (could be null!)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	class AItem* TraceHitItem;
	
	bool bShouldTraceForItems = false;
	int8 OverlappedItemCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	AItem* TraceHitItemLastFrame;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Initialization, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* BodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Initialization, meta = (AllowPrivateAccess = "true"))
	class AHandController* LeftController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Initialization, meta = (AllowPrivateAccess = "true"))
	AHandController* RightController;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Initialization, meta = (AllowPrivateAccess = "true"))
	//class UWidgetComponent* HUDWidget;

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

public:
	FORCEINLINE UCameraComponent* GetCameraComponent() const { return Camera; }
	FORCEINLINE USkeletalMeshComponent* GetBodyMesh() const { return BodyMesh; }
	FORCEINLINE AHandController* GetRightHandController() const { return RightController; }
	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return Combat; }
	FORCEINLINE bool ShouldPlayPickupSound() const { return bShouldPlayPickupSound; }
	FORCEINLINE bool ShouldPlayEquipSound() const { return bShouldPlayEquipSound; }

	void StartPickupSoundTimer();
	void StartEquipSoundTimer();
	
	FInterpLocation GetInterpLoctaion(int32 Index);
	/*UWidgetComponent* GetHUDWidget() { return HUDWidget; }*/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bFireButtonPressed = false;

	void IncrementOverlappedItemCount(int8 Amount);

	// No longer needed; AItem has GetInterpLocation
	/*FVector GetCameraInterpLocation();*/

	void GetPickupItem(AItem* Item);

	// WeaponHUD
	void ShowWeaponHUD();

	// Returns the index in InterpLocations array with lowest item count
	int32 GetInterpLocationIndex();

	void IncrementInterpLocItemCount(int32 Index, int32 Amount);
};
