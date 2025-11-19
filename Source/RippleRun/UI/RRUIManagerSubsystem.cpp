// Copyright TeamName. All Rights Reserved.


#include "RRUIManagerSubsystem.h"

void URRUIManagerSubsystem::CreateRootWidget(APlayerController* PlayerController)
{
	if (IsValid(PlayerController))
	{
		if (const TSubclassOf<URootWidget> RootWidgetClass = RootWidgetSoftClass.LoadSynchronous())
		{
			RootWidget = CreateWidget<URootWidget>(PlayerController, RootWidgetClass);
			
			if (IsValid(RootWidget))
			{
				RootWidget->AddToViewport();
			}
		}
	}
}
