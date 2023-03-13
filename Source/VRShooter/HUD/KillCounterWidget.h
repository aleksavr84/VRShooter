#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KillCounterWidget.generated.h"

UCLASS()
class VRSHOOTER_API UKillCounterWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* KillText;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* KillTextAnim;

	void SetDisplayText(FString TextToDisplay);
	void SetDisplayTextColor(FColor Color);
};
