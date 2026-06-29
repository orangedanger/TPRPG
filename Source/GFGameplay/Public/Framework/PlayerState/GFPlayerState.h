#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GFPlayerState.generated.h"

UCLASS(Abstract, Blueprintable)
class GFGAMEPLAY_API AGFPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AGFPlayerState();

	UFUNCTION(BlueprintPure, Category = "GF|Player")
	int32 GetCharacterLevel() const;

	UFUNCTION(BlueprintCallable, Category = "GF|Player")
	void SetCharacterLevel(int32 NewLevel);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_CharacterLevel, Category = "GF|Player", meta = (ClampMin = "1"))
	int32 CharacterLevel = 1;

	UFUNCTION()
	virtual void OnRep_CharacterLevel();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
