#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GFPlayerController.generated.h"

class UInputManager;

/**
 * Base player controller that owns the project input manager lifecycle.
 */
UCLASS(Abstract, Blueprintable)
class GFGAMEPLAY_API AGFPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AGFPlayerController();

	/** Returns the runtime input manager used by this local player controller. */
	UFUNCTION(BlueprintPure, Category = "Input")
	UInputManager* GetInputManager() const { return InputManager; }

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnRep_Pawn() override;
	virtual void SetupInputComponent() override;

	/** Input manager class used to configure mapping contexts and action DataTable rows. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TSubclassOf<UInputManager> InputManagerClass;
	
	/** Runtime input manager instance owned by this controller. */
	UPROPERTY(BlueprintReadOnly, Category = "Input", Transient)
	TObjectPtr<UInputManager> InputManager;

private:
	/** Creates the input manager if needed and refreshes its owner reference. */
	void InitializeInputManager();
};
