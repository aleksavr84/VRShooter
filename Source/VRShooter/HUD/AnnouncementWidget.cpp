#include "AnnouncementWidget.h"
#include "Components/TextBlock.h"

void UAnnouncementWidget::SetDisplayText(FString TextToDisplay)
{
    if (CenterLeftText)
    {
        CenterLeftText->SetText(FText::FromString(TextToDisplay));
    }
}
