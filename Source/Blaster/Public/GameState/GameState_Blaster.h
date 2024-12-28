// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GameState_Blaster.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AGameState_Blaster : public AGameState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	TArray<class APlayerState_Blaster*> TopStorePlayerStates;

	//We in fact dont need it to be replicated, because TopScore =f(PS::GetScore()) that is self-replicaed
	//not to mention PlayerStates_TopScore is already replicated!
	//anyway this TopScore is just optional as it will be always Array[0]->GetScore();
	uint32 TopScore{};

	//we should call this whenever a player kill another player ReceiveDamage()~>GM::PlayerElimmed()
	// from there you have PC, so you can access PS from PC (PS is a decent member of PC):
	//this will help you 'top-score' playerstate to the array as well as update the TopScore value
	void UpdateTopScorePlayerStates(APlayerState_Blaster* ScoringPlayerState);

	/*
	* Team
	*/
	//Stephen says we dont need them to be replicated because it will be handled in the server:
	TArray<APlayerState_Blaster*> RedTeam;
	TArray<APlayerState_Blaster*> BlueTeam;

	UPROPERTY(ReplicatedUsing = OnRep_RedTeamscore)
	uint32 RedTeamScore{};

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamscore)
	uint32 BlueTeamScore{};

	UFUNCTION()
	void OnRep_RedTeamscore();
	UFUNCTION()
	void OnRep_BlueTeamscore();

protected:
	void virtual BeginPlay() override;
};
