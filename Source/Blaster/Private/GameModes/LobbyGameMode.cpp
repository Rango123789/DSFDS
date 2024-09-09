// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int NumOfPlayers = GameState->PlayerArray.Num();

	if (GetNumPlayers() == 2)
	{
		//you can also check the box UseSeamlessTravel in BP___GameMode used by its appropriate level too: 
		bUseSeamlessTravel = true;

		//our plan: we travel from Lobby level with BP_this_GameMode into BlasterMap level (with a different game mode of course)
		if (GetWorld()) GetWorld()->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));

	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
}
