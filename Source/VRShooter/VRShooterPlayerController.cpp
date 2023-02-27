#include "VRShooterPlayerController.h"
#include "VRShooterCharacter.h"
#include "Components/WidgetComponent.h"
//#include "Blueprint/UserWidget.h"
//#include "HandController.h"

AVRShooterPlayerController::AVRShooterPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AVRShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	//// Check our HUDOverlayClass TSubclassOf variable
	//if (HUDOverlayClass)
	//{
	//	HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayClass);

	//	ShooterCharacter = Cast<AVRShooterCharacter>(GetPawn());
	//}
}

void AVRShooterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//if (!bIsInitialized)
	//{
	//	Init();
	//}
}

//void AVRShooterPlayerController::Init()
//{
//	/*if (ShooterCharacter)
//	{
//		RightHandController = RightHandController == nullptr ? ShooterCharacter->GetRightHandController() : RightHandController;
//
//		if (RightHandController)
//		{
//			if (RightHandController->GetHUDWidget() && HUDOverlay)
//			{
//				RightHandController->GetHUDWidget()->SetWidget(HUDOverlay);
//				HUDOverlay->SetVisibility(ESlateVisibility::Visible);
//
//				bIsInitialized = true;
//			}
//		}
//	}*/
//}


