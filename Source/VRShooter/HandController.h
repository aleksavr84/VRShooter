#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MotionControllerComponent.h"
#include "HandController.generated.h"

UCLASS()
class VRSHOOTER_API AHandController : public AActor
{
	GENERATED_BODY()
	
public:	
	AHandController();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

private:
	// Default sub object
	UPROPERTY(VisibleAnywhere)
	UMotionControllerComponent* MotionController;

	class AVRShooterCharacter* PlayerCharacter;
	class APlayerController* PlayerController;

	// Callbacks
	UFUNCTION()
	void ActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
	void ActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor);

	// Helpers
	bool CanClimb() const;

	// State
	bool bCanClimb = false;
	bool bIsClimbing = false;
	FVector ClimbingStartLocation;
	AHandController* OtherController;

	void SetMovementMode(EMovementMode Mode);

private:
	// Components
	UPROPERTY(EditAnywhere, Category = Initialization)
	USkeletalMeshComponent* RightHandMesh;
	
	UPROPERTY(EditAnywhere, Category = Initialization)
	USkeletalMeshComponent* LeftHandMesh;

	UPROPERTY(EditDefaultsOnly)
	class UHapticFeedbackEffect_Base* HapticEffect;

public:
	void SetHand(EControllerHand Hand) { MotionController->SetTrackingSource(Hand); }
	bool bMirroring = false;

	UFUNCTION(BlueprintCallable, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* GetRightHandMesh() { return RightHandMesh; }

	UFUNCTION(BlueprintCallable, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* GetLeftHandMesh() { return LeftHandMesh; }
	
	void PairController(AHandController* Controller);

	void Grip();
	void Release();

	void PlayHapticEffect();
};
