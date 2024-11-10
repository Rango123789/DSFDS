// Fill out your copyright notice in the Description page of Project Settings.

#include "GameModes/BlasterGameMode.h"
#include "Characters/BlasterCharacter.h"
#include "PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include <PlayerStates/PlayerState_Blaster.h>

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
	CountDownTime = WarmUpTime; //NO NEED
}

void ABlasterGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//now we CountDown from CountDownTime to zero and actively call StartMatch() so that the game now can OFFICALLY start, TimSeconds is now already CountDownTime :D :D

	////OPTION1: dont care abou the STARTING time, locally work, not sycnh!
	//if (MatchState == MatchState::WaitingToStart)
	//{
	//	CountDownTime -= DeltaTime;

	//	if (CountDownTime <= 0.f) StartMatch();
	//}

	//OPTION2: use StartingTime and GetWorld()->GetTimeSeconds() - better to adapt to sycnhing solution later on:
	if (MatchState == MatchState::WaitingToStart)
	{
		CountDownTime = WarmUpTime - (GetWorld()->GetTimeSeconds() - LevelStartingTime);

		//after StartMatch, if succeed, next time you're no long in ::WaitingToStart and you dont have to worry this code will run again at all :D :D
		if (CountDownTime <= 0.f)
		{
			//I add this to fix the bug - not fix neither
			bDelayedStart = false; 

			StartMatch();
		}
	}
}

//will trigger whenever GM::MatchState change
void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	if (MatchState == MatchState::WaitingToStart)
	{
		if (GetWorld()) UE_LOG(LogTemp, Warning, TEXT("GM::MatchStage::WaitingToStart,  Time: %f "), GetWorld()->GetTimeSeconds())
	}

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

		PlayerController->OnMatchStateSet(MatchState); //this helper funciton do more than just set
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

	if (PS_Elimmed == nullptr || PS_Attacker == nullptr) return;

	//only when they're different should add Score for the Attacker:
	if (PS_Attacker != PS_Elimmed)
	{
		PS_Attacker->UpdateHUD_Score();
		//if you consider killing yourself is a defeat - then simply move it out of this if!
		PS_Elimmed->UpdateHUD_Defeat();
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


