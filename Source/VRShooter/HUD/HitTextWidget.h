#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HitTextWidget.generated.h"

UCLASS()
class VRSHOOTER_API UHitTextWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HitText;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* HitNumberAnim;

	void SetDisplayText(FString TextToDisplay);
	void SetDisplayTextColor(FColor Color);
};
