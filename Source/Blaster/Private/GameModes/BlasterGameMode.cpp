// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/BlasterGameMode.h"
#include "Characters/BlasterCharacter.h"
#include "PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

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

	//RestartPlayer(EliminatedController); //easy option.

	if (EliminatedController == nullptr) return;

	TArray<AActor*> ActorArray;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), ActorArray);

	if (ActorArray.Num() > 0)
	{
		int RandomNum = FMath::RandRange(0, ActorArray.Num() - 1);

		RestartPlayerAtPlayerStart(EliminatedController, ActorArray[RandomNum] );
	}
}
