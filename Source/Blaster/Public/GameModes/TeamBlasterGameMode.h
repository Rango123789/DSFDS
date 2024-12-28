// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/BlasterGameMode.h"
#include "TeamBlasterGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ATeamBlasterGameMode : public ABlasterGameMode
{
	GENERATED_BODY()

public:
	/** Called after a successful login.  This is the first place it is safe to call replicated functions on the PlayerController. 
	= I dont see any PC::PostLogin or PC::Logout that propogate this ORIGIN lol, so may it names differently, like OnPossess or else? */
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/** Called when a Controller with a PlayerState leaves the game or is destroyed */
	virtual void Logout(AController* Exiting) override;

	//override the custom virtual function from its parent ABlasterGameMode:
	virtual float CalculateDamage(AController* AttackController, AController* VictimController, const float& BaseDamage) override;
protected:
	virtual void HandleMatchHasStarted() override;

public:
};
