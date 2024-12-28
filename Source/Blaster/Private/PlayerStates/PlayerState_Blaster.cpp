// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerStates/PlayerState_Blaster.h"
#include <Characters/BlasterCharacter.h>
#include <PlayerController/BlasterPlayerController.h>
#include <Net/UnrealNetwork.h>

void APlayerState_Blaster::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APlayerState_Blaster , Defeat);
	DOREPLIFETIME(APlayerState_Blaster, Team);
}

//if you look down we keep re-check them back, so this BeginPlay is no longer needed LOL
void APlayerState_Blaster::BeginPlay()
{
	if (GetWorld()) UE_LOG(LogTemp, Warning, TEXT("PS,  BeginPlay Time: %f "), GetWorld()->GetTimeSeconds())

	BlasterCharacter = GetPawn<ABlasterCharacter>(); //no need
	BlasterPlayerController = Cast<ABlasterPlayerController>(GetPlayerController());

	//we can test this without medicine1+2 and see how it goes:
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDScore(GetScore()); //medicine2
		BlasterPlayerController->SetHUDDefeat(Defeat); //medicine2
	}
}

void APlayerState_Blaster::OnRep_Team()
{
}

//this part will only help for client part, the server must handle itself from where you change the Score
// - it must be in Char::Elim, so the char also has 'BlasterPlayerController' from there
// - so it is not a big deal!
//However you need to access PlayerState_Blaster from Char as well
// - it is not a big deal neither as it has the PlayerController (PlayerState is a direct member of Controller). so PlayerController connect everything :)
// - not to mention, you have APawn::GetPlayerState() as well - so no need the intermediate!

//Stephen do this [GetChar -> GetController -> SetHUDScore(Score)]

//you need to check Attacker and Elimmed are different guys befoe give him score+1 lol
//this is meant to be called from the server, that is where it is appropriate in Char 
//that is in Char::Elim or GameMode (because of the 'circle')
void APlayerState_Blaster::UpdateHUD_Score()
{
	SetScore(GetScore() + 1);

	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(GetPlayerController()) : BlasterPlayerController;

	BlasterPlayerController->SetHUDScore(GetScore());
}

void APlayerState_Blaster::OnRep_Score()
{
	Super::OnRep_Score(); //in fact empty

	//No need, you can directly access PC from PS
	//BlasterCharacter = BlasterCharacter == nullptr ? GetPawn<ABlasterCharacter>() : BlasterCharacter;

	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(GetPlayerController()) : BlasterPlayerController;
	//SetHUDScore is correct - as it doesn't +1
	if(BlasterPlayerController) BlasterPlayerController->SetHUDScore(GetScore());
}

void APlayerState_Blaster::UpdateHUD_Defeat()
{
	Defeat += 1;

	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(GetPlayerController()) : BlasterPlayerController;

	BlasterPlayerController->SetHUDDefeat(Defeat);
}

void APlayerState_Blaster::OnRep_Defeat()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(GetPlayerController()) : BlasterPlayerController;

	//SetHUDScore is correct - as it doesn't +1
	if (BlasterPlayerController) BlasterPlayerController->SetHUDDefeat(Defeat);
}


