#pragma once

#include "CoreMinimal.h"
#include "Framework/Character/GFCharacter.h"
#include "GPCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputComponent;

UCLASS(Blueprintable)
class GPGAMEPLAY_API AGPCharacter : public AGFCharacter
{
	GENERATED_BODY()

public:
	AGPCharacter();
protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	
};
