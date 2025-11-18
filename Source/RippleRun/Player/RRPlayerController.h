// Copyright TeamName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RRPlayerController.generated.h"

class UInputMappingContext;

UCLASS()
class RIPPLERUN_API ARRPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	
#pragma region Input
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TArray<TObjectPtr<UInputMappingContext>> DefaultMappingContexts;
	
#pragma endregion

#pragma region UI

private:
	void CreateRootWidget();
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> RootWidgetClass;
	UPROPERTY()
	TObjectPtr<UUserWidget> RootWidget;
	
#pragma endregion
};
