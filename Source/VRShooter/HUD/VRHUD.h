#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VRHUD.generated.h"

UCLASS()
class VRSHOOTER_API AVRHUD : public AActor
{
	GENERATED_BODY()
	
public:	
	AVRHUD();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Initialization, meta = (AllowPrivateAccess = "true"))
	USceneComponent* Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Initialization, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* HUDWidget;

	// Reference to the Overall HUD Overlay Blueprint class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "True"))
	TSubclassOf<class UUserWidget> HUDOverlayClass;

	// Variable to hold the HUD Overlay Widget after created it
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Widgets, meta = (AllowPrivateAccess = "True"))
	UUserWidget* HUDOverlay;

	class AHandController* HandController;
	class AVRShooterCharacter* ShooterCharacter;

	// Initialization
	UPROPERTY(EditDefaultsOnly, Category = Initialization)
	float HUDOffsetX = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = Initialization)
	float HUDOffsetY = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = Initialization)
	float HUDOffsetZ = 10.0f;

	void UpdateHUDLocationAndRotation();

public:
	UWidgetComponent* GetHUDWidget() { return HUDWidget; }
	UUserWidget* GetHUDUserWidget();
	void SetHandController(AHandController* Controller) { HandController = Controller; }
};
