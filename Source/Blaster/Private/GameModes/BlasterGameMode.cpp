// Fill out your copyright notice in the Description page of Project Settings.

#include "GameModes/BlasterGameMode.h"
#include "Characters/BlasterCharacter.h"
#include "PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include <PlayerStates/PlayerState_Blaster.h>
#include "GameState/GameState_Blaster.h"

namespace MatchState
{
	const FName CoolDown = FName(TEXT("CoolDown"));
}

ABlasterGameMode::ABlasterGameMode()
{
	if (GetWorld()) UE_LOG(LogTemp, Warning, TEXT("GameMode,  Constructor Time: %f "), GetWorld()->GetTimeSeconds())
	//this will stop StartMatch() being auto-called, spawn each Player 'spectator = DEFAULT pawn' to fly around
	//TimeSeconds start to count when GM or the FRIST GM is initialized, not sure which one but surely not when the game offically start
	// 
	bDelayedStart = true;
}

void ABlasterGameMode::BeginPlay()
{
	if(GetWorld()) UE_LOG(LogTemp, Warning, TEXT("GameMode,  BeginPlay Time: %f ") , GetWorld()->GetTimeSeconds())

	//forget to call this line will stop the server to posses its pawn, though the clients work normally:
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
	CountingDownTime = WarmUpTime; //NO NEED
}

void ABlasterGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//start counting to escape WaitingToStart
	if (MatchState == MatchState::WaitingToStart)
	{
		CountingDownTime = WarmUpTime - (GetWorld()->GetTimeSeconds() - LevelStartingTime);

		//after this, you're no longer in ::WaitingToStart and you dont have to worry this code will run again at all :D :D
		if (CountingDownTime <= 0.f)
		{
			StartMatch(); //SetMatchState(MatchState::InProgress);
		}
	}
	//Start counting to escape InProgress
	else if (MatchState == MatchState::InProgress)
	{
		CountingDownTime = MatchTime - (GetWorld()->GetTimeSeconds() - (LevelStartingTime + WarmUpTime));
		if (CountingDownTime <= 0.f)
		{
			SetMatchState(MatchState::CoolDown);
		}
	}
	else if (MatchState == MatchState::CoolDown)
	{
		CountingDownTime = CoolDownTime - (GetWorld()->GetTimeSeconds() - (LevelStartingTime + WarmUpTime + MatchTime));
		//other things:
		 if (CountingDownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

//will trigger whenever GM::MatchState change
void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	//our goal is just to propogate GM::MatchState to PC::MatchState, and then handle thing from there:
		//access the whole PC array || its Iterator_[new] stored in gamemode and call PC::SetMatchState(), the rest will be handled from there.
	FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();
	for (It; It; ++It) //this pattern is weird compared to:  for (int i = 1; i <= 10; ++i){...}
	{
		//this didn't work:
			//APlayerController* PlayerController = *It;
		//but this work1:
		APlayerController* PlayerController = Cast<APlayerController>(*It);
		//but this work2:
		ABlasterPlayerController* BlasterPlayerController = Cast<ABlasterPlayerController>(*It);

		if (PlayerController) BlasterPlayerController->OnMatchStateSet(MatchState, bIsTeamMatch); //this helper funciton do more than just set
	}
}

//this trigger hence ReceiveDamage and Healh <=0
//+Currently you could only receive damage from players of other team or yourself
//, so it could only reach GM::PlayerElimmed in those cases
//+ So in GM::PlayerElimmed you can simply compare whether AttackerController and Victim controller are the same and decide to give score to the Attacker!
void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* ElimininatedCharacter, ABlasterPlayerController* EliminatedController, ABlasterPlayerController* AttackerController)
{
//this is like a circle:
	if (ElimininatedCharacter == nullptr || AttackerController == nullptr) return;
	//you can in fact call this directly from char::ReceiveDamge~>Health < 0 ?
	ElimininatedCharacter->Elim(false); 

//extra things when we need to use AttackerPC:
	APlayerState_Blaster* PS_Elimmed = EliminatedController->GetPlayerState<APlayerState_Blaster>();
	APlayerState_Blaster* PS_Attacker = AttackerController->GetPlayerState<APlayerState_Blaster>();
	//GameState_AGameStateBase is direct member of AGameModeBase, hence AGameMode, so you can cast from GM::GameState, but I prefer to use this Get template for more convenient: 
	AGameState_Blaster* GameState_Blaster = GetGameState<AGameState_Blaster>();

	if (PS_Elimmed == nullptr || PS_Attacker == nullptr || GameState_Blaster == nullptr) return;

	//only when they're different should add Score for the Attacker:
	if (PS_Attacker != PS_Elimmed) //(*)
	{
		PS_Attacker->UpdateHUD_Score();

		//UPDATE: Shocking news, stephen make GM::PlayerElimmed VITUAL
		//, and override it in TeamGM::PlayerElimmed
		//, calling super:: and add the code above!
		// However I would need to Cast to PS_Attacker again, so I'm to tire of it:
		//Access the GameState and call the CONTEXTUAL function:
			//if (bIsTeamMatch)
			//{
			//	if (PS_Attacker->GetTeam() == ETeam::ET_RedTeam)
			//	{
			//		if(GameState_Blaster) GameState_Blaster->UpdateHUDRedTeamScore();
			//	}
			//	if (PS_Attacker->GetTeam() == ETeam::ET_BlueTeam)
			//	{
			//		if (GameState_Blaster) GameState_Blaster->UpdateHUDBlueTeamScore();
			//	}
			//}


	//INSERT1: backup the array, make a copy, note that elements are copied pointer but they still point to the same DATA as the ORIGINAL array's elements :D :
		TArray<APlayerState_Blaster*> TopScorePlayerStates_BeforeUpdate = GameState_Blaster->TopStorePlayerStates;

		//Update the GS::Array_TopScore, the arra can change after this call: 
		GameState_Blaster->UpdateTopScorePlayerStates(PS_Attacker);
		TArray<APlayerState_Blaster*>& TopScorePlayerStates_AfterUpdate = GameState_Blaster->TopStorePlayerStates;

	//INSERT2: compare and make decision, use this wise trick avoiding checking all conditions: the only possible to gain 'more' crown is the attack character, but there could be more than one character being removed the crown
		
	//remove the crown of char that appear in array1 , but not appar in array2:
		for (auto& PlayerState : TopScorePlayerStates_BeforeUpdate) //PS appear in array1
		{
			if (TopScorePlayerStates_AfterUpdate.Contains(PlayerState) == false) //but not appear in array2
			{
				ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(PlayerState->GetPawn());

				if(BlasterCharacter) BlasterCharacter->MulticastRemoveCrown();
			}
		}

		//show the crown for all appear in array2: a little bit overkill LOL, but you can exclude the case that it appear in both array1 and array2, because it must be ALREADY shown previously LOL:
			//option1:
		for (auto& PlayerState : TopScorePlayerStates_AfterUpdate) //PS appear in array2
		{
			if (PlayerState == nullptr) continue;

			if (TopScorePlayerStates_BeforeUpdate.Contains(PlayerState) == false) //but not appear in array1
			{
				ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(PlayerState->GetPawn());

				if (BlasterCharacter) BlasterCharacter->MulticastShowCrown();
			}
			else {} //do nothing: if it appear both in array1 and array2 then it must be shown already beforehand
		}
			//option2: the only possible to gain 'more' crown is the attack character, hence if you accept the fact that you can send 'REDUDANT MulticastRPC' then you can simply do AttackCharacter->MulticastShowCrown()
			//I think mine is better so I wont bother to follow stephen, mine is 'Net performance-wise'!!
	}
	//if you consider killing yourself is a defeat - then simply move it out of (*), yes I did
    PS_Elimmed->UpdateHUD_Defeat();

	//for WBP_ElimAnnounce: when a player is elimmed, is the best time to give ElimAnnounce to all players, you can go to GM::OnMatchStateSet to copy some same code if you want: F[X]Iterator, FConst[X]Iterator
	FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();
	for (It; It; It++)
	{
		ABlasterPlayerController* PC =Cast<ABlasterPlayerController>( *It);
		if (PC)
		{
			PC->ClientSetHUD_ElimAnnounce(PS_Attacker, PS_Elimmed);
		}
	}
}

void ABlasterGameMode::RequestRespawn(ABlasterCharacter* ElimininatedCharacter, ABlasterPlayerController* EliminatedController)
{
	if (ElimininatedCharacter)
	{
		//I see this many times, but first time got a chance to use it:
		ElimininatedCharacter->Reset();
		ElimininatedCharacter->Destroy();
	}

	//RestartPlayer(EliminatedController); //easy option.

	if (EliminatedController == nullptr) return;

	TArray<AActor*> ActorArray;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), ActorArray);

	if (ActorArray.Num() > 0)
	{
		int32 RandomNum = FMath::RandRange(0, ActorArray.Num() - 1);

		RestartPlayerAtPlayerStart(EliminatedController, ActorArray[RandomNum] );
	}
}

//this function currently trigger in the server:
void ABlasterGameMode::HandleLeaveGameRequest(APlayerState_Blaster* LeavingPlayerState)
{
	if (LeavingPlayerState == nullptr) return;
//TODO1: remove LeavingPlayerState from GS::TopScorePlayerStates array (if he leads),
	AGameState_Blaster* BlasterGameState = GetGameState<AGameState_Blaster>();
	if (GameState && BlasterGameState->TopStorePlayerStates.Contains(LeavingPlayerState))
	{
		BlasterGameState->TopStorePlayerStates.Remove(LeavingPlayerState);
	}

//TODO2: Call Char::Elim(true)
	//NOTE: Elim() is also called in the chain: Char::ReceiveDamage~>GM::PlayerElimmed~>Char::Elim() + other things
	//meaning we need to adapt it for the TimerCallback_Elim() to either call SpawnRequest or MSS::DestroySession() correctly, I know now we want to call MSS::DestroySession(), but we must handle case when player elimmed rather press Leave button:
	//go and add 'bPlayerLeavingGame' parameter for Elim(true/false)_[where we pass in] && MultiCastElim()_[where we set __ = In__ ]:
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(LeavingPlayerState->GetPawn());

	if (BlasterCharacter)
	{
		//this will help to set bPlayerLeavingGame = true and execute the else of TimerCallback_Elim to execute ::DestroySesion/LIKE (rather than meet the if and execute Respawn)
		//you should access 'Char::bIsEliminated ==false' before trying to call it again, this case; you fail the DestroySession in next step in TimerCallback_Elim, so that you dont play ElimMontage and so on again LOL: 
		BlasterCharacter->Elim(true); //self-handled, previously it is called in server in ReceiveDamage chain
	}
}

float ABlasterGameMode::CalculateDamage(AController* AttackController, AController* VictimController, const float& BaseDamage)
{
	return BaseDamage;
}
