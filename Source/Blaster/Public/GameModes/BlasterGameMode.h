// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ABlasterGameMode();
	virtual void Tick(float DeltaTime) override;

	virtual void PlayerEliminated(class ABlasterCharacter* ElimininatedCharacter, class ABlasterPlayerController* EliminatedController, class ABlasterPlayerController* AttackerController);
	virtual void RequestRespawn(ABlasterCharacter* ElimininatedCharacter, ABlasterPlayerController* EliminatedController);
protected:
	void virtual BeginPlay() override;

	virtual void OnMatchStateSet() override;

	//New1, it should ORIGINATE from GM instead LOL:
	UPROPERTY(EditAnywhere)
	float MatchTime = 120.f;

	//New2, we should also create PC::WarmUpTime to be propogate to from here
	UPROPERTY(EditAnywhere)
	float WarmUpTime = 10.f;

	float CountDownTime = WarmUpTime;//with this you dont even need BeginPlay

	//StarterMap -> LobbyMap ->     GameMap
	//    0                      BlasterLevelStartingTime
	float LevelStartingTime =0.f; //to be set in BeginPlay()


public:
	float GetLevelStartingTime() { return LevelStartingTime; }
	float GetWarmUpTime() { return WarmUpTime; }
	float GetMatchTime() { return MatchTime; }

	FName GetMatchState() { return MatchState; }
};
