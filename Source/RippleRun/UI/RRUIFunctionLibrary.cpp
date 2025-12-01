// Copyright TeamName. All Rights Reserved.


#include "RRUIFunctionLibrary.h"

#include "CommonActivatableWidget.h"
#include "RRUIManagerSubsystem.h"

UCommonActivatableWidget* URRUIFunctionLibrary::PushWidgetToLayer(const APlayerController* PlayerController, const FGameplayTag LayerName,
                                                                  const TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	if (!ensure(IsValid(PlayerController)) || !ensure(WidgetClass))
	{
		return nullptr;
	}
	
	if (const URRUIManagerSubsystem* UIManager = PlayerController->GetGameInstance()->GetSubsystem<URRUIManagerSubsystem>())
	{
		return UIManager->PushWidgetToLayer(LayerName, WidgetClass);
	}
	
	return nullptr;
}

void URRUIFunctionLibrary::PopWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget)
{
	if (!IsValid(ActivatableWidget))
	{
		return;
	}
	
	if (const APlayerController* PC = ActivatableWidget->GetOwningPlayer())
	{
		if (const URRUIManagerSubsystem* UIManager = PC->GetGameInstance()->GetSubsystem<URRUIManagerSubsystem>())
		{
			if (URootWidget* RootWidget = UIManager->GetRootWidget())
			{
				RootWidget->FindAndRemoveWidgetFromLayer(ActivatableWidget);	
			}
		}
	}
}
