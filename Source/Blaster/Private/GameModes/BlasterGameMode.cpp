// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/BlasterGameMode.h"
#include "Characters/BlasterCharacter.h"
#include "PlayerController/BlasterPlayerController.h"

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* ElimininatedCharacter, ABlasterPlayerController* EliminatedController, ABlasterPlayerController* AttackerController)
{
	//you can in fact call this directly from char::ReceiveDamge~>Health < 0 ?
	if(ElimininatedCharacter) ElimininatedCharacter->Elim(); //
}

void ABlasterGameMode::RequestRespawn(ABlasterCharacter* ElimininatedCharacter, ABlasterPlayerController* EliminatedController)
{
	if (ElimininatedCharacter)
	{
		//I see this many times, but first time got a chance to use it:
		ElimininatedCharacter->Reset();
		ElimininatedCharacter->Destroy();
	}

	RestartPlayer(EliminatedController);
}
