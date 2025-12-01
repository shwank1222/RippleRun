// Copyright TeamName. All Rights Reserved.


#include "RRGameState.h"

#include "Net/UnrealNetwork.h"

void ARRGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ARRGameState, ReplayTargetStone);
}
