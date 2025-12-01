// Copyright TeamName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RRUIFunctionLibrary.generated.h"

struct FGameplayTag;
class UCommonActivatableWidget;

UCLASS()
class RIPPLERUN_API URRUIFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "GlimUIFunctionLibrary")
	static UCommonActivatableWidget* PushWidgetToLayer(const APlayerController* PlayerController, UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerName, UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UCommonActivatableWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, Category = "GlimUIFunctionLibrary")
	static void PopWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget);
};
