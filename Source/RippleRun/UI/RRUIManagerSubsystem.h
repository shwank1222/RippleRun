// Copyright TeamName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RootWidget.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RRUIManagerSubsystem.generated.h"

UCLASS(Config = Game)
class RIPPLERUN_API URRUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void CreateRootWidget(APlayerController* PlayerController);
	
	template <typename WidgetType = UCommonActivatableWidget>
		WidgetType* PushWidgetToLayer(const FGameplayTag& LayerTag, const TSubclassOf<UCommonActivatableWidget> ActivatableWidgetClass) const
	{
		return IsValid(RootWidget) ? RootWidget->PushWidgetToLayer<WidgetType>(LayerTag, ActivatableWidgetClass) : nullptr;
	}
	
	FORCEINLINE URootWidget* GetRootWidget() const { return RootWidget; }
	
private:
	UPROPERTY(Config)
	TSoftClassPtr<URootWidget> RootWidgetSoftClass;
	
	UPROPERTY()
	TObjectPtr<URootWidget> RootWidget;
};
