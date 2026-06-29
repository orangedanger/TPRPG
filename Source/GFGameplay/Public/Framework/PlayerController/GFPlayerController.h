#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GFPlayerController.generated.h"

UCLASS(Abstract, Blueprintable)
class GFGAMEPLAY_API AGFPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AGFPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnRep_Pawn() override;
	virtual void SetupInputComponent() override;
};
