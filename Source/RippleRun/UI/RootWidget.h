// Copyright TeamName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "RootWidget.generated.h"

UCLASS()
class RIPPLERUN_API URootWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	template <typename WidgetType = UCommonActivatableWidget>
	WidgetType* PushWidgetToLayer(const FGameplayTag& LayerTag, const TSubclassOf<UCommonActivatableWidget> ActivatableWidgetClass) const
	{
		UCommonActivatableWidgetStack* LayerWidget = Layers.FindRef(LayerTag);
		if (!IsValid(LayerWidget))
		{
			UE_LOG(LogTemp, Warning, TEXT("Layer %s is not registered!"), *LayerTag.ToString());
			return nullptr;
		}

		return LayerWidget->AddWidget<WidgetType>(ActivatableWidgetClass);
	}
	
	void FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget);
	
protected:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void RegisterLayer(const FGameplayTag LayerTag, UCommonActivatableWidgetStack* LayerWidget)
	{
		Layers.Add(LayerTag, LayerWidget);
	}
	
private:
	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetStack>> Layers;
};
