#include "Gameplay/Character/GPCharacter.h"



AGPCharacter::AGPCharacter()
{

}

void AGPCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AGPCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}