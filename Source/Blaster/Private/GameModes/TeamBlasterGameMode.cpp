// Fill out your copyright notice in the Description page of Project Settings.

#include "GameModes/TeamBlasterGameMode.h"
#include "GameState/GameState_Blaster.h"
#include "PlayerStates/PlayerState_Blaster.h"

float ATeamBlasterGameMode::CalculateDamage(AController* AttackController, AController* VictimController, const float& BaseDamage)
{
	//DO NOT call Super:: , we dont want it.
	
	APlayerState_Blaster* AttackPlayer = AttackController->GetPlayerState<APlayerState_Blaster>();
	APlayerState_Blaster* VictimPlayer = VictimController->GetPlayerState<APlayerState_Blaster>();

	//We decide that even if there is no AttackController, we still let the Victim receive damage:
	if (AttackPlayer == nullptr) return BaseDamage;
	//This case is NOT likely, stephen decide to return BaseDamage, but I return "0" (in case it is already elimmed I guess? ): no matter anyway
	if (VictimPlayer == nullptr) return 0.f;

	//allow self-hit, hence return BaseDamage: 
	if (AttackPlayer == VictimPlayer) return BaseDamage;

	//this is the MAIN case in case for Team match: we dont have to check nullptr again, because if it is nullptr it already returns above, amazing!
	if (AttackPlayer->GetTeam() != VictimPlayer->GetTeam()) return BaseDamage;
	else return 0.f;
}

void ATeamBlasterGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	if (GameState == nullptr) return;

//access GS::PlayerArray (TArray<PS> ) sort each element either into GS::RedTeam or GS::RedTeam, Set element::Team = Red/Blue too
	//Cast to GameState_Blaster:
	AGameState_Blaster* GameState_Blaster = Cast<AGameState_Blaster>(GameState);
	//you can't cast a whole array, you could only cast element to a child element as you iterate through the array:
	 TArray<APlayerState*> PlayerStates = GameState->PlayerArray; //GameState_Blaster->Playerarray

	if (GameState_Blaster)
	{
		for (auto& PlayerState : PlayerStates)
		{
			APlayerState_Blaster* PlayerState_Blaster = Cast<APlayerState_Blaster>(PlayerState);

			//we only need to sort a specific PlayerState if it starts off with NoTeam:
			//note that respawn char wont affect PS and GS's DATA at all.
			if (PlayerState_Blaster && PlayerState_Blaster->GetTeam() == ETeam::ET_NoTeam)
			{
				if ( GameState_Blaster->RedTeam.Num() <= GameState_Blaster->BlueTeam.Num())
				{
					GameState_Blaster->RedTeam.AddUnique(PlayerState_Blaster);
					PlayerState_Blaster->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					GameState_Blaster->BlueTeam.AddUnique(PlayerState_Blaster);
					PlayerState_Blaster->SetTeam(ETeam::ET_BlueTeam);
				}
			}

		}
	}
}

void ATeamBlasterGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (NewPlayer == nullptr) return;

//access NewPlayer_PC::PS, sort each element either into GS::RedTeam or GS::RedTeam, Set element::Team = Red/Blue too
	//Cast to GameState_Blaster:
	AGameState_Blaster* GameState_Blaster = Cast<AGameState_Blaster>(GameState);
	//Access the PS associate with NewPlayer:
	APlayerState_Blaster* PlayerState_Blaster = NewPlayer->GetPlayerState<APlayerState_Blaster>();

	//without the loop condition TIRE1 combine with TIRE2, understandable:
	if (GameState_Blaster && PlayerState_Blaster && PlayerState_Blaster->GetTeam() == ETeam::ET_NoTeam)
	{
		if (GameState_Blaster->RedTeam.Num() <= GameState_Blaster->BlueTeam.Num())
		{
			GameState_Blaster->RedTeam.AddUnique(PlayerState_Blaster);
			PlayerState_Blaster->SetTeam(ETeam::ET_RedTeam);
		}
		else
		{
			GameState_Blaster->BlueTeam.AddUnique(PlayerState_Blaster);
			PlayerState_Blaster->SetTeam(ETeam::ET_BlueTeam);
		}
	}
}

void ATeamBlasterGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

//access Exiting_PC::PS, remove it from whichever team it currently in:
	AGameState_Blaster* GameState_Blaster = Cast<AGameState_Blaster>(GameState);
	//Access the PS associate with NewPlayer:
	APlayerState_Blaster* PlayerState_Blaster = Exiting->GetPlayerState<APlayerState_Blaster>();

	if (GameState_Blaster && PlayerState_Blaster)
	{
		if (GameState_Blaster->RedTeam.Contains(PlayerState_Blaster))
		{
			GameState_Blaster->RedTeam.Remove(PlayerState_Blaster);
		}
		if (GameState_Blaster->BlueTeam.Contains(PlayerState_Blaster))
		{
			GameState_Blaster->BlueTeam.Remove(PlayerState_Blaster);
		}
	}
}

