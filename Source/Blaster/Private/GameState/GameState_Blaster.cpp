// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/GameState_Blaster.h"
#include <Net/UnrealNetwork.h>
#include "PlayerStates/PlayerState_Blaster.h"
#include <CharacterComponents/CombatComponent.h>
#include <PlayerController/BlasterPlayerController.h>
#include <HUD/BlasterHUD.h>

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

//you would need to update it to ALL players, not just any specific player, hence iterate over the PC array is needed, also you already have OnRep_ , so you dont need to create ClientUpdateHUD this time:
void AGameState_Blaster::UpdateHUDRedTeamScore()
{
	RedTeamScore++;
	//you may be thinking iterate over all PCs, and then set HUD for all client at once here, but the fact is that GetHUD() ONLY valid in CD, hence we iterate over the whole PC array here just to be in one server device (even if the device receive damage originate from a client device or else) - because currently here, in the server, only the server can pass PC::GetHUD()
	//meaning we can add 'PC::IsLocalController()' to skip the rest for better performance, but if you dont, PC::SetHUDX() can still sort the rest out naturally; but we want more performance right?

	//OPTION1: iterate over the whole array:
		//FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();
		//for (It; It; It++)
		//{
		//	ABlasterPlayerController* PC = Cast<ABlasterPlayerController>(*It);
		//	if (PC && PC->IsLocalController())
		//	{
		//		//this is no need, the job is done from PC locally already LOL:
		//		ABlasterHUD* BlasterHUD = PC->GetHUD<ABlasterHUD>();
		//		//ONLY this is needed:
		//		PC->SetHUDRedTeamScore(RedTeamScore);
		//	}
		//}

	//OPTION2: use UWorld::GetFirstPlayerController() to access the PC that is associated with the DC char (not neccessary the char that ReceiveDamage originate from), isn't amazing?
	ABlasterPlayerController* PC = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC) PC->SetHUDRedTeamScore(RedTeamScore);
}

void AGameState_Blaster::UpdateHUDBlueTeamScore()
{
	BlueTeamScore++;

		//FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();
		//for (It; It; It++)
		//{
		//	ABlasterPlayerController* PC = Cast<ABlasterPlayerController>(*It);
		//	if (PC && PC->IsLocalController())
		//	{
		//		//this is no need, the job is done from PC locally already LOL:
		//		ABlasterHUD* BlasterHUD = PC->GetHUD<ABlasterHUD>();
		//		//ONLY this is needed
		//		PC->SetHUDBlueTeamScore(BlueTeamScore);
		//	}
		//}

	//OPTION2: use UWorld::GetFirstPlayerController() to access the PC that is associated with the DC char (not neccessary the char that ReceiveDamage originate from), isn't amazing?
	ABlasterPlayerController* PC = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC) PC->SetHUDBlueTeamScore(BlueTeamScore);
}

//each client execute on OnRep_, iterate over the whole array is just to SetHUDX for itself, the reason is simple: only CD can pass PC::GetHUD()
void AGameState_Blaster::OnRep_RedTeamscore()
{
		//FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();
		//for (It; It; It++)
		//{
		//	ABlasterPlayerController* PC = Cast<ABlasterPlayerController>(*It);
		//	if (PC && PC->IsLocalController())
		//	{
		//		//this is no need, the job is done from PC locally already LOL:
		//		ABlasterHUD* BlasterHUD = PC->GetHUD<ABlasterHUD>();
		//		//ONLY this is needed
		//		PC->SetHUDRedTeamScore(RedTeamScore);
		//	}
		//}

	//OPTION2: use UWorld::GetFirstPlayerController() to access the PC that is associated with the DC char (not neccessary the char that ReceiveDamage originate from), isn't amazing?
	ABlasterPlayerController* PC = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC) PC->SetHUDRedTeamScore(RedTeamScore);
}

void AGameState_Blaster::OnRep_BlueTeamscore()
{
		//FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();
		//for (It; It; It++)
		//{
		//	ABlasterPlayerController* PC = Cast<ABlasterPlayerController>(*It);
		//	if (PC && PC->IsLocalController())
		//	{
		//		//this is no need, the job is done from PC locally already LOL:
		//		ABlasterHUD* BlasterHUD = PC->GetHUD<ABlasterHUD>();
		//		//ONLY this is needed
		//		PC->SetHUDBlueTeamScore(BlueTeamScore);
		//	}
		//}

	//OPTION2: use UWorld::GetFirstPlayerController() to access the PC that is associated with the DC char (not neccessary the char that ReceiveDamage originate from), isn't amazing?
	ABlasterPlayerController* PC = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if(PC) PC->SetHUDBlueTeamScore(BlueTeamScore);
}


void AGameState_Blaster::CheckAndSetRedTeamScore()
{
}

void AGameState_Blaster::CheckAndSetBlueeamScore()
{
}

void AGameState_Blaster::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld()) UE_LOG(LogTemp, Warning, TEXT("GameState,  Constructor Time: %f "), GetWorld()->GetTimeSeconds())
}
