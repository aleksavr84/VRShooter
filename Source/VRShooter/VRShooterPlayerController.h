#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "VRShooterPlayerController.generated.h"

UCLASS()
class VRSHOOTER_API AVRShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AVRShooterPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	//// Reference to the Overall HUD Overlay Blueprint class
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "True"))
	//TSubclassOf<class UUserWidget> HUDOverlayClass;

	//// Variable to hold the HUD Overlay Widget after created it
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Widgets, meta = (AllowPrivateAccess = "True"))
	//UUserWidget* HUDOverlay;

	//class AVRShooterCharacter* ShooterCharacter;
	//class AHandController* RightHandController;

	//bool bIsInitialized = false;
	//void Init();
};
