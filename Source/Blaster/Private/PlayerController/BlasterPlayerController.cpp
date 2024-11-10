// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerController/BlasterPlayerController.h"
#include "HUD/BlasterHUD.h"
#include "HUD/CharacterOverlay_UserWidget.h"
#include "HUD/UW_Announcement.h"
#include "Characters/BlasterCharacter.h"
#include <PlayerStates/PlayerState_Blaster.h>
#include "GameModes/BlasterGameMode.h"
#include <Net/UnrealNetwork.h>
#include "Kismet/GameplayStatics.h"

ABlasterPlayerController::ABlasterPlayerController()
{
	if (GetWorld()) UE_LOG(LogTemp, Warning, TEXT("PC,  Constructor Time: %f "), GetWorld()->GetTimeSeconds())
}

void ABlasterPlayerController::BeginPlay()
{
	if (GetWorld()) UE_LOG(LogTemp, Warning, TEXT("PC,  BeginPlay Time: %f "), GetWorld()->GetTimeSeconds())

//move this to AplayerController:
	Super::BeginPlay();


	//OPTIONAL, but recommend to try to initalize them
	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	if (BlasterHUD)
	{
		CharacterOverlay_UserWidget = BlasterHUD->GetCharacterOverlay_UserWidget();
		UserWidget_Announcement = BlasterHUD->GetUserWidget_Announcement();
	}

	//get DATA from GameMode for clients_PC + server_PC accidentally:
	ServerCheckMatchState(); //GOLDEN4 to propogated DATA from GM to clients

}
  
void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateHUDTime(); //locally set to trigger per second only

	//update Delta_ServerMinusClient every 5s. factorize this into 'CheckTimeSync()'
	AccumilatingTime += DeltaTime;
	if (IsLocalController() && AccumilatingTime > TimeSynchFrequency)
	{
		//this in turn call Client_ReportReqest. Finally update Delta_ServerMinusClient every 5s.
		Server_RequestServerTime(GetWorld()->GetTimeSeconds());
		AccumilatingTime = 0;
	}

}

//I decide to give the same name, why not!
void ABlasterPlayerController::OnMatchStateSet(const FName& InMatchState)
{
	MatchState = InMatchState;

	if (MatchState == MatchState::WaitingToStart)
	{
		if (GetWorld()) UE_LOG(LogTemp, Warning, TEXT("PC::MatchStage_propogate::WaitingToStart,  Time: %f "), GetWorld()->GetTimeSeconds())
	//this 4 lines is my style:
		//BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
		//if (BlasterHUD == nullptr) return;

		//UserWidget_Announcement = UserWidget_Announcement == nullptr ? BlasterHUD->GetUW_Announcement() : UserWidget_Announcement;
		//if (UserWidget_Announcement == nullptr) return;
	//	not early for server but early for clients:
		//UserWidget_Announcement->AddToViewport();
	}

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
}

void ABlasterPlayerController::OnRep_MatchState()
{
	//it is not an enum so we can't use switch lol
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
}
//this is executed on server, which is the only place GetGameMode() return valid
void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));//GetWorld()->GetAuthoGameMode() are the same
	if (BlasterGameMode)
	{
		LevelStartingTime = BlasterGameMode->GetLevelStartingTime();
		WarmUpTime = BlasterGameMode->GetWarmUpTime();
		MatchTime = BlasterGameMode->GetMatchTime();
		MatchState = BlasterGameMode->GetMatchState(); 
	}
	ClientCheckMatchState(LevelStartingTime, WarmUpTime, MatchTime, MatchState);

	// Only Do this if you didn't do it in BlasterHUD, which I did, so I dont need, calling it here is no different than calling in PC::BeginPlay as this will be in turn called in BeginPlay, and UE5.2 didn't work in PC::BeginPlay, so it wont work for me :) PC constructor is done before BlasterHUD. hence PC::BeginPlay is done before PC::HUD I guess
		// if(MatchState=WaitingToStart && BlasterHUD) CreateWidget + AddToViewport WBP_Announcement 
}
//this is executed on owning client, who sent the ServerRPC above in this case, and help clients get values from GameMode:
void ABlasterPlayerController::ClientCheckMatchState_Implementation(float InLevelStartingTime, float InWarmUpTime, float InMatchTime, FName InMatchName)
{
	LevelStartingTime = InLevelStartingTime;
	WarmUpTime = InWarmUpTime;
	MatchTime = InMatchTime;
	MatchState = InMatchName;

	//In worst case, the client joint 'after' the point MG::OnMatchStateSet()~>InProgress trigger, that it wont have those code update to clients at all, where the server PC will surely be updated, and possibly the only one, hence we dont add this line in Server___ above  
	OnMatchStateSet(MatchState); //PC:: _propogate()

	// Only Do this if you didn't do it in BlasterHUD, which I did, so I dont need, calling it here is no different than calling in PC::BeginPlay as this will be in turn called in BeginPlay, and UE5.2 didn't work in PC::BeginPlay, so it wont work for me :) PC constructor is done before BlasterHUD. hence PC::BeginPlay is done before PC::HUD I guess
	// if(MatchState=WaitingToStart && BlasterHUD) CreateWidget + AddToViewport WBP_Announcement 
}


//Name this function the same name as GameMode::HandleMatchHasStarted()<~InProgress <~GM::OnMatchStateSet()
void ABlasterPlayerController::HandleMatchHasStarted()
{
	//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;

	//by my way, upto this point they should be all valid, so check the point is enough, no need recheck
	CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;
	if (CharacterOverlay_UserWidget == nullptr) return;
	//Stephen gotta do it in PC::PollInit, but I do it here:
	CharacterOverlay_UserWidget->AddToViewport();

	//gotta HIDE, not destroy for re-use after match, the UW_Announcement first, but anyway, it only render at the end of the frame, so I put it here:
	UserWidget_Announcement = UserWidget_Announcement == nullptr ? BlasterHUD->GetUserWidget_Announcement() : UserWidget_Announcement;
	if (UserWidget_Announcement == nullptr) return;

	UserWidget_Announcement->SetVisibility(ESlateVisibility::Hidden);
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


//help the server char to setup Enhanced Input, as it doesn't have a natural first spawn with the system due to bDelayed = true for adding WarmUpTime:
	if (BlasterCharacter) BlasterCharacter->SetupEnhancedInput_IMC();

//OPTION1 to delay HUD to be shown during WarmUpTime:
		//BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
		//if (BlasterHUD == nullptr) return;
		//CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;
		//if (CharacterOverlay_UserWidget == nullptr) return;

		//CharacterOverlay_UserWidget->AddToViewport();
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

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController, MatchState);
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
	float HealthPercent = FMath::Clamp(Health / MaxHealth, 0.f, 1.f);
	if (CharacterOverlay_UserWidget) CharacterOverlay_UserWidget->SetHealthPercent(HealthPercent);

	FString Text = FString::FromInt(Health) + FString(" / ") + FString::FromInt(MaxHealth);
	if (CharacterOverlay_UserWidget) CharacterOverlay_UserWidget->SetHealthText(Text);
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
void ABlasterPlayerController::SetHUDMatchTimeLeft(int32 InTimeLeft)
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
	if(CharacterOverlay_UserWidget) CharacterOverlay_UserWidget->SetMatchTimeLeftText(text);
}

void ABlasterPlayerController::SetHUDWarmUpTimeLeft(int32 InTimeLeft)
{
//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;

	UserWidget_Announcement = UserWidget_Announcement == nullptr ? BlasterHUD->GetUserWidget_Announcement() : UserWidget_Announcement;
	if (UserWidget_Announcement == nullptr) return;
//Back to main business:
		//READY:
	int min = FMath::FloorToInt(InTimeLeft / 60.f); //must be Floor
	int sec = FMath::CeilToInt(InTimeLeft - min * 60.f); //Round - ceil - floor upto you!

	FString text = FString::FromInt(min) + FString(" : ") + FString::FromInt(sec);
	//text = FString::Printf(TEXT("%02d : %02d") , min , sec) - if you want 01 : 07 format!
	
		//CALL:
	if (UserWidget_Announcement) UserWidget_Announcement->SetWarmUpTimeLeftText(text);
}

//the turn ON/OFF of WBP_Announce and WBP_Overlay already done externally, partly in PC::OnMatchStateSet(), so you dont have to worry about it here:
void ABlasterPlayerController::UpdateHUDTime()
{
//You can factorize these code into 'SetHUDTime()' if you want
//TimeLeft will be converted to int by C++ rule, so THE Math::CeilToInt is totally OPTIONAL
//we doing these so that we ONLY call SetHUDTimeLeft(TimeLeft) per second - not per frame, isn't it amazing?
//note that you can create 2 separate TimeLeft + 2 separate TimeLeftInt_LastSecond if you can merge them, however stephen has a very nice way to do them together so I follow.

//PART1: counting TimeLeft depends on which state we're in
	float TimeLeft;
	if (MatchState == MatchState::WaitingToStart)
	{
		//TimeLeft is meant for WarmUpTimeLeft:
		TimeLeft = WarmUpTime - (GetServerTime_Synched() - LevelStartingTime);
	}
	else if (MatchState == MatchState::InProgress)
	{
		//Timeleft is meant for MatchTimeLeft:
		TimeLeft = MatchTime - (GetServerTime_Synched() - LevelStartingTime - WarmUpTime); //synched
	       //TimeLeft = FMath::CeilToInt(MatchTime - GetWorld()->GetTimeSeconds() - LevelStartingTime); //NOT synched
	}

	//FMath::CeilToInt() = OPTIONAL, so that you see the MAX number longer a bit, if you dont do this line so change the type o TimeLeft above to uint32 instead for auto-conversion: 
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

//PART2: 'per-second' technique
	if (SecondsLeft != TimeLeftInt_LastSecond)
	{
		if (MatchState == MatchState::WaitingToStart) SetHUDWarmUpTimeLeft(SecondsLeft);

		else if (MatchState == MatchState::InProgress) SetHUDMatchTimeLeft(SecondsLeft);
	}
	TimeLeftInt_LastSecond = SecondsLeft;

}
