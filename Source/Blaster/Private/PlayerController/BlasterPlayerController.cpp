// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/BlasterPlayerController.h"
#include "HUD/BlasterHUD.h"
#include "HUD/CharacterOverlay_UserWidget.h"
#include "Characters/BlasterCharacter.h"
#include <PlayerStates/PlayerState_Blaster.h>

void ABlasterPlayerController::BeginPlay()
{
	//move this to AplayerController:

		BlasterHUD = Cast<ABlasterHUD>(GetHUD());
		if(BlasterHUD) CharacterOverlay_UserWidget = BlasterHUD->GetCharacterOverlay_UserWidget();

}

//this is for respawning - if needed - only KEY vars from Char need this
void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	//OPTION1: this work ->I forget to call Super:: LOL, this prove that the order of PC::OnPossess and Char::BeginPlay has been changed when GameStart and respawn the char.
		//ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
		//if (BlasterCharacter) BlasterCharacter->UpdateHUD_Health();

	//OPTION2: So this has to work, need only to access char::Health and MaxHealth for PC::SetHUDHealth( , ) right here
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if (BlasterCharacter)
	{
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
	}

	//Optional so far:
	APlayerState_Blaster* PlayerState_Blaster = GetPlayerState<APlayerState_Blaster>();
	if (BlasterCharacter && PlayerState_Blaster)
	{
		SetHUDScore(PlayerState_Blaster->GetScore()); //medicine3
		//SetHUDDefeat(PlayerState_Blaster->GetDefeat()); //medicine3
	}
}

void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	//not sure we need to add 'IsLocalController()', well only owning client need to show their HUD right ?
	if (IsLocalController())
	{
		Server_RequestServerTime(GetWorld()->GetTimeSeconds());
	}
}
  
void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateHUDTime();

	//update Delta_ServerMinusClient every 5s. factorize this into 'CheckTimeSync()'
	AccumilatingTime += DeltaTime;
	if (IsLocalController() && AccumilatingTime > TimeSynchFrequency)
	{
		//this in turn call Client_ReportReqest. Finally update Delta_ServerMinusClient every 5s.
		Server_RequestServerTime(GetWorld()->GetTimeSeconds());
		AccumilatingTime = 0;
	}
}

//I think separate them into 2 cases is 'OPTIONAL', I reckon Delta ~ 0 if the one call Server_RequestServerTime is the server itself! but it is perfect to separate them:
float ABlasterPlayerController::GetServerTime_Synched()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + Delta_ServerMinusServer;
}

//We need to call this function some time so that it upates Deltatime (but dont need so often, as Deltatime is BASE-independent and shouldn't change)
//This only run in the server, GetWorld()->GetTimeSeconds() return TimeOfServer:
void ABlasterPlayerController::Server_RequestServerTime_Implementation(float TimeOfClientWhenRequesting)
{
	float TimeOfServer_WhenReceived = GetWorld()->GetTimeSeconds();
	Client_ReportRequestBackToRequestingClient(TimeOfClientWhenRequesting, TimeOfServer_WhenReceived);
}

//this only run in the owning client (if called from the server) , GetWorld()->GetTimeSeconds() return TimeOfOwningClient:
//In case the Server itself is the one call Server_RequestServerTime the DeltaTime naturaly "~ZERO"
//but anyway it doesn't matter as when GetServerTime_Synched we will take it into account for this case too
void ABlasterPlayerController::Client_ReportRequestBackToRequestingClient_Implementation(float TimeOfClientWhenRequesting, float TimeOfServerWhenReceivedRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientWhenRequesting;

	//This Delta already consider RRT/2: 
	Delta_ServerMinusServer =  (TimeOfServerWhenReceivedRequest + RoundTripTime / 2) - GetWorld()->GetTimeSeconds() ;
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;		
	if (BlasterHUD == nullptr) return;
	//UE_LOG(LogTemp, Warning, TEXT("BasterHUD valid"));

	CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;
	if (CharacterOverlay_UserWidget == nullptr) return;
	//UE_LOG(LogTemp, Warning, TEXT("CharacterOverlay_UserWidget valid"));

//Back to main business:
	CharacterOverlay_UserWidget->SetHealthPercent(Health / MaxHealth);

	FString Text = FString::FromInt(Health) + FString(" / ") + FString::FromInt(MaxHealth);
	CharacterOverlay_UserWidget->SetHealthText(Text);
}

//the pattern is the same as SetHUDHealth above
void ABlasterPlayerController::SetHUDScore(int InScore)
{
//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;

	CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;
	if (CharacterOverlay_UserWidget == nullptr) return;
//Back to main business:
	CharacterOverlay_UserWidget->SetScoreText(InScore);
}

void ABlasterPlayerController::SetHUDDefeat(int InDefeat)
{
//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;

	CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;
	if (CharacterOverlay_UserWidget == nullptr) return;
//Back to main business:
	CharacterOverlay_UserWidget->SetDefeatText(InDefeat);
}

void ABlasterPlayerController::SetHUDAmmo(int InAmmo)
{
	//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;

	CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;
	if (CharacterOverlay_UserWidget == nullptr) return;
	//Back to main business:
	CharacterOverlay_UserWidget->SetAmmoText(InAmmo);
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int InCarriedAmmo)
{
	//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;

	CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;
	if (CharacterOverlay_UserWidget == nullptr) return;
	//Back to main business:
	CharacterOverlay_UserWidget->SetCarriedAmmoText(InCarriedAmmo);
}

//we calculate timeleft from outside
void ABlasterPlayerController::SetHUDTimeLeft(int32 InTimeLeft)
{
	//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;

	CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;
	if (CharacterOverlay_UserWidget == nullptr) return;
	//Back to main business:
	
		//READY:
	int min = FMath::FloorToInt(InTimeLeft / 60.f); //must be Floor
	int sec = FMath::CeilToInt(InTimeLeft - min * 60.f); //Round - ceil - floor upto you!

	FString text = FString::FromInt(min) + FString(" : ") + FString::FromInt(sec);
	//text = FString::Printf(TEXT("%02d : %02d") , min , sec) - if you want 01 : 07 format!
	
		//CALL:
	CharacterOverlay_UserWidget->SetTimeLeftText(text);
}

void ABlasterPlayerController::UpdateHUDTime()
{
	//You can factorize these code into 'SetHUDTime()' if you want
	//TimeLeft will be converted to int by C++ rule, so THE Math::CeilToInt is totally OPTIONAL
	//we doing these so that we ONLY call SetHUDTimeLeft(TimeLeft) per second - not per frame, isn't it amazing?
	//GetTimeSeconds = Accumilating time from GameStart - HERE
	//GetDeltaSeconds = DeltaTime - not HERE

	//uint32 TimeLeft = FMath::CeilToInt(MatchTime - GetWorld()->GetTimeSeconds()); //NOT synched
	uint32 TimeLeft = FMath::CeilToInt(MatchTime - GetServerTime_Synched());  //Synched
	if (TimeLeft != TimeLeftInt_LastSecond)
	{
		SetHUDTimeLeft(TimeLeft);
	}
	TimeLeftInt_LastSecond = TimeLeft;
}
