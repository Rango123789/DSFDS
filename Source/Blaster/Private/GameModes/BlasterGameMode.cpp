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
			//APlayerController* Controller = *It;
		//but this work1:
		APlayerController* Controller = Cast<APlayerController>(*It);
		//but this work2:
		ABlasterPlayerController* PlayerController = Cast<ABlasterPlayerController>(*It);

		if(PlayerController) PlayerController->OnMatchStateSet(MatchState); //this helper funciton do more than just set
	}
}

//this trigger hence ReceiveDamage and Healh <=0
void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* ElimininatedCharacter, ABlasterPlayerController* EliminatedController, ABlasterPlayerController* AttackerController)
{
//this is like a circle:
	if (ElimininatedCharacter == nullptr || AttackerController == nullptr) return;
	//you can in fact call this directly from char::ReceiveDamge~>Health < 0 ?
	ElimininatedCharacter->Elim(); //

//extra things when we need to use AttackerPC:
	APlayerState_Blaster* PS_Elimmed = EliminatedController->GetPlayerState<APlayerState_Blaster>();
	APlayerState_Blaster* PS_Attacker = AttackerController->GetPlayerState<APlayerState_Blaster>();
	//GameState_AGameStateBase is direct member of AGameModeBase, hence AGameMode, so you can cast from GM::GameState, but I prefer to use this Get template for more convenient: 
	AGameState_Blaster* GameState_Blaster = GetGameState<AGameState_Blaster>();

	if (PS_Elimmed == nullptr || PS_Attacker == nullptr) return;

	//only when they're different should add Score for the Attacker:
	if (PS_Attacker != PS_Elimmed) //(*)
	{
		PS_Attacker->UpdateHUD_Score();

		//Update the GS::Array_TopScore, only when ...: 
		if(GameState_Blaster) GameState_Blaster->UpdatePlayerStates_TopScore(PS_Attacker);
	}
	//if you consider killing yourself is a defeat - then simply move it out of (*), yes I did
    PS_Elimmed->UpdateHUD_Defeat();
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

void ABlasterGameMode::HandleLeaveGameRequest(APlayerState_Blaster* LeavingPlayerState)
{

}


