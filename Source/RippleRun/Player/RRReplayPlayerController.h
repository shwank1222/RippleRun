// Copyright TeamName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RRReplayPlayerController.generated.h"

UCLASS()
class RIPPLERUN_API ARRReplayPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintNativeEvent, Category = "UI")
	void ShowReplayWidget();
};
