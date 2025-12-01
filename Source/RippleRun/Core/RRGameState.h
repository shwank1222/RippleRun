// Copyright TeamName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "RRGameState.generated.h"

class ASkippingStone;

UCLASS()
class RIPPLERUN_API ARRGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(Replicated, BlueprintReadWrite)
	TObjectPtr<ASkippingStone> ReplayTargetStone;
};
