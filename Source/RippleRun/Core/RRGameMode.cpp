// Copyright TeamName. All Rights Reserved.


#include "RRGameMode.h"

#include "RRGameState.h"
#include "RRPlayerState.h"

ARRGameMode::ARRGameMode()
{
	GameStateClass = ARRGameState::StaticClass();
	PlayerStateClass = ARRPlayerState::StaticClass();
}
