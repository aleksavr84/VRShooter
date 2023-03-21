#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AnnouncementWidget.generated.h"

UCLASS()
class VRSHOOTER_API UAnnouncementWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CenterLeftText;

	void SetDisplayText(FString TextToDisplay);
};
