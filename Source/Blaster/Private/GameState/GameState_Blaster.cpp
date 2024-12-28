// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/GameState_Blaster.h"
#include <Net/UnrealNetwork.h>
#include "PlayerStates/PlayerState_Blaster.h"

void AGameState_Blaster::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGameState_Blaster, TopStorePlayerStates);
	DOREPLIFETIME(AGameState_Blaster, RedTeamScore);
	DOREPLIFETIME(AGameState_Blaster, BlueTeamScore);
}

//check if the ScoringPlayerState::Score > PlayerStates_TopScore[0]::Score
//if > - replace the PlayerStates_TopScore[0] with itself
//if < - do nothing
//if = - Add it along, dont replace
void AGameState_Blaster::UpdateTopScorePlayerStates(APlayerState_Blaster* ScoringPlayerState)
{
	if (ScoringPlayerState == nullptr) return;
//it is not safe to merge 'if' and the first 'if else'? not here LOL, you willaccess X[0] and it give you a crash lol:
	//at first there is NO element, so you need to add element for it first, this will always happen FIRST and then never happen again:
	if (TopStorePlayerStates.Num() == 0 )
	{
		TopStorePlayerStates.Add(ScoringPlayerState);
		TopScore = ScoringPlayerState->GetScore(); //absolutely need
	}
	//if it reaches here, hence Num must be > 0 already, if = then add alongside
	// , note: stephen .AddUnique() is incorrect LOL, use .Add() is correct for our purpose:
	// , note: you can already replace PlayerStates_TopScore[0]->GetScore() with TopScore!
	else if (ScoringPlayerState->GetScore() == TopStorePlayerStates[0]->GetScore())
	{
		TopStorePlayerStates.Add(ScoringPlayerState);
		TopScore = ScoringPlayerState->GetScore(); //no need
	}
	//if it reaches here, hence Num must be > 0 already, if >  then replace, but we should empty it first in worst case there is more than one player occupying the array previously:
	else if (ScoringPlayerState->GetScore() > TopStorePlayerStates[0]->GetScore())
	{
		TopStorePlayerStates.Empty();
		TopStorePlayerStates.Add(ScoringPlayerState);
		//PlayerStates_TopScore[0] = ScoringPlayerState;
		TopScore = ScoringPlayerState->GetScore(); //absolutely need
	}
}

void AGameState_Blaster::OnRep_RedTeamscore()
{

}

void AGameState_Blaster::OnRep_BlueTeamscore()
{

}

void AGameState_Blaster::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld()) UE_LOG(LogTemp, Warning, TEXT("GameState,  Constructor Time: %f "), GetWorld()->GetTimeSeconds())
}
