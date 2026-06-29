#include "Gameplay/GameMode/GPGameMode.h"

#include "Gameplay/Character/GPCharacter.h"
#include "Gameplay/GameState/GPGameState.h"
#include "Gameplay/PlayerController/GPPlayerController.h"
#include "Gameplay/PlayerState/GPPlayerState.h"

AGPGameMode::AGPGameMode()
{
	GameStateClass = AGPGameState::StaticClass();
	PlayerStateClass = AGPPlayerState::StaticClass();
	PlayerControllerClass = AGPPlayerController::StaticClass();
	DefaultPawnClass = AGPCharacter::StaticClass();
}

void AGPGameMode::InitGameState()
{
	Super::InitGameState();
}
