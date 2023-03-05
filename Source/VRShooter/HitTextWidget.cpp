#include "HitTextWidget.h"
#include "Components/TextBlock.h"
#include "Styling/SlateColor.h"

void UHitTextWidget::SetDisplayText(FString TextToDisplay)
{
    if (HitText)
    {
        HitText->SetText(FText::FromString(TextToDisplay));
    }
}

void UHitTextWidget::SetDisplayTextColor(FColor Color)
{
    if (HitText)
    {
        HitText->SetColorAndOpacity(FSlateColor(Color));
    }
}
