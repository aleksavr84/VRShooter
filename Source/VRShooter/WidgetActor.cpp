#include "WidgetActor.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "VRShooterCharacter.h"
#include "HitTextWidget.h"

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
	UHitTextWidget* HitTextWidget = Cast<UHitTextWidget>(WidgetComp->GetWidget());

	if (HitTextWidget)
	{
		// Set HitText
		if (HitTextWidget->HitText)
		{
			HitTextWidget->SetDisplayText(Text);

			// SetColor if HeadShot
			if (bChangeColor)
			{
				HitTextWidget->SetDisplayTextColor(SecondaryColor);
			}
		}
		// Start Animation
		if (HitTextWidget->HitNumberAnim)
		{
			HitTextWidget->PlayAnimation(
				HitTextWidget->HitNumberAnim,
				0.f,
				1
			);
		}
	}
}

void AWidgetActor::RotateWidgetToPlayer()
{
	if (ShooterCharacter)
	{
		FRotator WidgetRotation = WidgetComp->GetRelativeRotation();
		FVector Direction = ShooterCharacter->GetActorLocation() - GetActorLocation();
		FRotator Rotation = FRotationMatrix::MakeFromX(Direction).Rotator();
		WidgetComp->SetWorldRotation(FRotator(WidgetRotation.Pitch, Rotation.Yaw, WidgetRotation.Roll));
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

