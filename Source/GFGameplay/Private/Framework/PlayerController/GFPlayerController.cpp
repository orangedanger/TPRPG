#include "Framework/PlayerController/GFPlayerController.h"

AGFPlayerController::AGFPlayerController()
{
	bReplicates = true;
}

void AGFPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AGFPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AGFPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
}

void AGFPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}
