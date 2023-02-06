#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HandController.h"
#include "VRShooterCharacter.generated.h"

UCLASS()
class VRSHOOTER_API AVRShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AVRShooterCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Combat
	void FireWeapon();
	void AimingButtonPressed();
	void AimingButtonReleased();

private:
	// PostProcessing
	void UpdateBlinkers();
	FVector2D GetBlinkerCenter();

	// Movement
	void MoveForward(float Value);
	void MoveRight(float Value);
	void GripLeft() { LeftController->Grip(); }
	void ReleaseLeft() { LeftController->Release(); }
	void GripRight() { RightController->Grip(); }
	void ReleaseRight() { RightController->Release(); }

	// Teleport
	void BeginTeleport();
	void StartFade(float FromAlpha, float ToAlpha);
	void FinishTeleport();
	bool FindTeleportDestination(TArray<FVector>& OutPath, FVector& OutLocation);
	void UpdateDestinationMarker();
	void DrawTeleportPath(const TArray<FVector>& Path);
	void UpdateSpline(const TArray<FVector>& Path);

private:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "treu"))
	class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, Category = Initialization)
	class AHandController* LeftController;

	UPROPERTY(VisibleAnywhere, Category = Initialization)
	AHandController* RightController;

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

	// Initialization

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
	float ControllerRotation = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = Initialization)
	TSubclassOf<AHandController> HandControllerClass;

	// Effects

	// Weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;

	// Smoke Trail for bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	float CameraDefaultFOV;
	float CameraZoomedFOV;

	// Object References
	class APlayerController* PlayerController;

public:
	FORCEINLINE UCameraComponent* GetCameraComponent() const { return Camera; }
};
