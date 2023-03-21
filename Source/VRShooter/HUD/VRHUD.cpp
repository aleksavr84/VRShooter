#include "VRHUD.h"
#include "Components/WidgetComponent.h"
#include "VRShooter/Character/VRShooterCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"

AVRHUD::AVRHUD()
{
 		PrimaryActorTick.bCanEverTick = true;

		Scene = CreateDefaultSubobject<USceneComponent>(FName("Scene"));
		SetRootComponent(Scene);

		HUDWidget = CreateDefaultSubobject<UWidgetComponent>(FName("HUDWidget"));
		HUDWidget->SetupAttachment(Scene);
}

void AVRHUD::BeginPlay()
{
	Super::BeginPlay();
}

void AVRHUD::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateHUDLocationAndRotation();
}

void AVRHUD::UpdateHUDLocationAndRotation()
{
	ShooterCharacter = ShooterCharacter == nullptr ? Cast<AVRShooterCharacter>(GetOwner()) : ShooterCharacter;

	if (ShooterCharacter)
	{
		if (ShooterCharacter->GetRightHandController() &&
			ShooterCharacter->GetCameraComponent() &&
			HUDWidget)
		{
			const FVector MotionControllerLocation = ShooterCharacter->GetRightHandController()->GetActorLocation();
			const FVector CameraLocation = ShooterCharacter->GetCameraComponent()->GetComponentLocation();

			FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(MotionControllerLocation, CameraLocation);

			FVector ForwardVector = UKismetMathLibrary::GetForwardVector(Rotation);
			FVector UpVector = UKismetMathLibrary::GetUpVector(Rotation);
			FVector RightVector = UKismetMathLibrary::GetRightVector(Rotation);

			FVector NewLocation = MotionControllerLocation + (ForwardVector * HUDOffsetX) + (UpVector * HUDOffsetZ) + (RightVector * HUDOffsetY);

			HUDWidget->SetWorldLocation(NewLocation);

			// Rotate Widget towards camera
			FVector WidgetLocation = HUDWidget->GetComponentLocation();
			FRotator WidgetToCameraRotation = UKismetMathLibrary::FindLookAtRotation(WidgetLocation, CameraLocation);

			HUDWidget->SetWorldRotation(WidgetToCameraRotation);
		}
	}
}

UUserWidget* AVRHUD::GetHUDUserWidget()
{
	if (HUDWidget)
	{
		return HUDWidget->GetWidget();
	}

	return nullptr;
}

