#include "KillCounterWidget.h"
#include "Components/TextBlock.h"
#include "Styling/SlateColor.h"

void UKillCounterWidget::SetDisplayText(FString TextToDisplay)
{
    if (KillText)
    {
        KillText->SetText(FText::FromString(TextToDisplay));
    }
}

void UKillCounterWidget::SetDisplayTextColor(FColor Color)
{
    if (KillText)
    {
        KillText->SetColorAndOpacity(FSlateColor(Color));
    }
}

