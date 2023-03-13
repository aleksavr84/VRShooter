#include "VRShooterPlayerController.h"
#include "VRShooterCharacter.h"
#include "Components/WidgetComponent.h"

AVRShooterPlayerController::AVRShooterPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AVRShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AVRShooterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

