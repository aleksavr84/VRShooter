#include "VRShooter/Actors/WidgetActor.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "VRShooter/Character/VRShooterCharacter.h"
#include "VRShooter/HUD/HitTextWidget.h"
#include "Camera/CameraComponent.h"

AWidgetActor::AWidgetActor()
{
 	PrimaryActorTick.bCanEverTick = true;

	WidgetComp = CreateDefaultSubobject<UWidgetComponent>(FName("WidgetComponent"));
	WidgetComp->SetupAttachment(GetRootComponent());
}

void AWidgetActor::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoDestroy)
	{
		GetWorldTimerManager().SetTimer(
			DestroyTimer,
			this,
			&AWidgetActor::DestroyWidget,
			DestroyTime
		);
	}
}

void AWidgetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RotateWidgetToPlayer();
	
	if (bShouldMoveUpwards)
	{
		MoveWidgetUpwards(DeltaTime);
	}
}

void AWidgetActor::SetTextAndStartAnimation(FString Text, bool bChangeColor)
{
	UHitTextWidget* TextWidget = Cast<UHitTextWidget>(WidgetComp->GetWidget());

	if (TextWidget)
	{
		// Set Text
		if (TextWidget->HitText)
		{
			TextWidget->SetDisplayText(Text);

			// SetColor if HeadShot
			if (bChangeColor)
			{
				TextWidget->SetDisplayTextColor(SecondaryColor);
			}
		}
		// Start Animation
		if (TextWidget->HitNumberAnim)
		{
			TextWidget->PlayAnimation(
				TextWidget->HitNumberAnim,
				0.f,
				1
			);
		}
	}
}

void AWidgetActor::RotateWidgetToPlayer()
{
	//UE_LOG(LogTemp, Warning, TEXT("RotateWidgetToPlayer"));
	if (ShooterCharacter)
	{
		//UE_LOG(LogTemp, Warning, TEXT("%s"), *ShooterCharacter->GetName());
		////UE_LOG(LogTemp, Warning, TEXT("ShooterCharacter"));
		FRotator WidgetRotation = WidgetComp->GetRelativeRotation();
		FVector Direction = ShooterCharacter->GetActorLocation() - GetActorLocation();
		FRotator Rotation = FRotationMatrix::MakeFromX(Direction).Rotator();
		WidgetComp->SetWorldRotation(FRotator(0.f, Rotation.Yaw, 0.f));
	}
}

void AWidgetActor::MoveWidgetUpwards(float DeltaTime)
{
	WidgetComp->AddWorldOffset(FVector(0.f, 0.f, MovementSpeed * DeltaTime));
}

void AWidgetActor::DestroyWidget()
{
	Destroy();
}

