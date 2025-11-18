// Copyright TeamName. All Rights Reserved.


#include "RRPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"

void ARRPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	CreateRootWidget();
}

void ARRPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (const UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}
	}
}

void ARRPlayerController::CreateRootWidget()
{
	if (ensure(RootWidgetClass != nullptr))
	{
		RootWidget = CreateWidget<UUserWidget>(this, RootWidgetClass);
		
		if (RootWidget)
		{
			RootWidget->AddToViewport();
		}
	}
}
