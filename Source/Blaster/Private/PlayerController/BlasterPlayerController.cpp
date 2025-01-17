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
#include "GameState/GameState_Blaster.h"
#include "CharacterComponents/CombatComponent.h"
#include <EnhancedInputSubsystems.h>
#include <EnhancedInputComponent.h>
#include "HUD/UserWidget_ReturnToMainMenu.h"
#include "HUD/UserWidget_ElimAnnounce.h"

ABlasterPlayerController::ABlasterPlayerController()
{
	if (GetWorld()) UE_LOG(LogTemp, Warning, TEXT("PC,  Constructor Time: %f "), GetWorld()->GetTimeSeconds())
}

void ABlasterPlayerController::Server_ReportPingStatus_Implementation(bool bHighPing)
{
	OnReportPingStatus_Delegate.Broadcast(bHighPing);
}

void ABlasterPlayerController::BeginPlay()
{
	//if (GetWorld()) UE_LOG(LogTemp, Warning, TEXT("PC,  BeginPlay Time: %f "), GetWorld()->GetTimeSeconds())

	Super::BeginPlay();

	//PollInit(); //must before other code, we need ==null to pass the condition inside for first time

	////OPTIONAL, but recommend to try to initalize them
	//BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	//if (BlasterHUD)
	//{
	//	CharacterOverlay_UserWidget = BlasterHUD->GetCharacterOverlay_UserWidget();
	//	UserWidget_Announcement = BlasterHUD->GetUserWidget_Announcement();
	//}

	//get DATA from GameMode for clients_PC + server_PC accidentally:
	ServerCheckMatchState(); //GOLDEN4 to propogated DATA from GM to clients

	//setup Input in PlayerController: basiscally the same: the question is which one take precedence when Char is still alive, priority affect anything, is it additive?
	if (IMC_Blaster_PC)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
			GetLocalPlayer());

		if (Subsystem)
		{
			Subsystem->AddMappingContext(IMC_Blaster_PC, 0);
		}
	}
}

void ABlasterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(InputComponent);

	if (EnhancedInputComp)
	{
		EnhancedInputComp->BindAction(IA_ReturnToMainMenu, ETriggerEvent::Triggered, this, &ThisClass::Input_ReturnToMainMenu);
	}
}

void ABlasterPlayerController::Input_ReturnToMainMenu(const FInputActionValue& Value)
{

	BlasterHUD = BlasterHUD == nullptr ? GetHUD<ABlasterHUD>() : BlasterHUD;

	if (BlasterHUD && BlasterHUD->GetUserWidget_ReturnToMainMenu())
	{
		if (bIsRetunWidgetOpen == false)
		{
			BlasterHUD->GetUserWidget_ReturnToMainMenu()->MenuSetup();
		}
		else
		{
			BlasterHUD->GetUserWidget_ReturnToMainMenu()->MenuTearDown();
		}
		//this is an interesting statement LOL:
		bIsRetunWidgetOpen = !bIsRetunWidgetOpen;
	}


}
 
void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateHUDTime(); //locally set to trigger per second only

	//update Delta_ServerMinusClient every 5s. factorize this into 'CheckTimeSync()'
	UpdateServerClient_Delta_Periodically(DeltaTime);

	CheckPing(DeltaTime);
}

void ABlasterPlayerController::CheckPing(float DeltaTime)
{
	//no need to check ping and give warning if it is the server, there is no lag there!
	if (HasAuthority()) return;

	//if you want to start to count again aftter it finish PingAnimation, then add this If()
	//; otherwise you can remove it and follow stephen!
	//you can access check on UserWidget::IsAnimationPlaying(WBPAnimation_PingWarning) instead so that you dont need to create an extra bHighPingAnimationPlaying. 
	//; the reward isn't very attactive for effort nor performance
	//; so I simply create an extra one LOL
	//if(bHighPingAnimationPlaying == false) 
	RunningTime_HighPing += DeltaTime;

	if (RunningTime_HighPing >= CheckPingFrequency)
	{
		//PlayerState is public member of PC, or you can use GetPlayerState<T>() if you want:
		//GetPing() return ping/4 is OBSOLETE, so I dont use it
		//if(PlayerState) UE_LOG(LogTemp, Warning, TEXT("Ping: %f"), PlayerState->GetPingInMilliseconds())
		//if(true) //test for now
		if (PlayerState && PlayerState->GetPingInMilliseconds() >= HighPingThreshold)
		{
			bHighPingAnimationPlaying = true;
			RunningTime_HighPing = 0; //you can let it here, mean you continue to check until Ping get high, or put it just outside to really test it in the next CheckPingFrequency

			StartHighPingWarning();

			//this is the best place to call it without needing to create extra accumilating time:
			//call from a client[/server itself] and execute in the server - that is all we need currently
			Server_ReportPingStatus(true);
		}
		else Server_ReportPingStatus(false);
	}

	if (bHighPingAnimationPlaying == true)
	{
		RunningTime_HighPingAnimation += DeltaTime;
		if (RunningTime_HighPingAnimation > PingWarningDuration)
		{
			bHighPingAnimationPlaying = false;
			RunningTime_HighPingAnimation = 0;

			StopHighPingWarning();
		}
	}
}

void ABlasterPlayerController::ClientSetHUD_ElimAnnounce_Implementation(APlayerState_Blaster* AttackPlayer, APlayerState_Blaster* ElimmedPlayer)
{
//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;

	//because you didn't create it at first in BlasterHUD now, so this line will stop you LOL:
	/*UserWidget_ElimAnnounce = UserWidget_ElimAnnounce == nullptr ? BlasterHUD->GetUserWidget_ElimAnnounce() : UserWidget_ElimAnnounce;
	if (UserWidget_ElimAnnounce == nullptr) return;*/

//back to main business:
	//get the PlayerState associate with this current PC: (to be element of PC array)
	APlayerState_Blaster* ThisPlayer = GetPlayerState<APlayerState_Blaster>() ;

	if(ThisPlayer == nullptr || AttackPlayer == nullptr || ElimmedPlayer == nullptr) return;

	const FString& ThisPlayerName = ThisPlayer->GetPlayerName(); //in fact no need LOL
	const FString& AttackPlayerName = AttackPlayer->GetPlayerName();
	const FString& ElimmedPlayerName = ElimmedPlayer->GetPlayerName();

//OPTION1: just print same message [Player1] kill [Player2] for all players LOL, I think this is not bad at all! 
	//UserWidget_ElimAnnounce->SetTextBlock_Elim(AttackPlayerName, ElimmedPlayerName);

//OPTION2: print message consider Pronouns LOL - UPDATE, we now use 'CreateAndAddElimAnnounce' instead of access and call UW::SetTextBlock_Elim , all same job + extra has been done in new function:
	//there is 3 killing cases: This Player didn't get involved at all, this Player is the killer, this player is the victim
	if (ThisPlayer == AttackPlayer && ThisPlayer != ElimmedPlayer)
	{
		BlasterHUD->CreateAndAddElimAnnounce("You", ElimmedPlayerName);
	}
	else if (ThisPlayer != AttackPlayer && ThisPlayer == ElimmedPlayer)
	{
		BlasterHUD->CreateAndAddElimAnnounce(AttackPlayerName, "You");
	}
	else if (ThisPlayer != AttackPlayer && ThisPlayer != ElimmedPlayer)
	{
		BlasterHUD->CreateAndAddElimAnnounce(AttackPlayerName, ElimmedPlayerName);
	}
	//there is 2 suicide cases: this player kill himself, other player kill himself
	else if (AttackPlayer == ElimmedPlayer && ThisPlayer == AttackPlayer)
	{
		BlasterHUD->CreateAndAddElimAnnounce("You", "yourself");
	}
	else if (AttackPlayer == ElimmedPlayer && ThisPlayer != AttackPlayer)
	{
		BlasterHUD->CreateAndAddElimAnnounce(AttackPlayerName, "themself");
	}
}

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
		SetHUDShield(BlasterCharacter->GetShield(), BlasterCharacter->GetMaxShield());

		if (BlasterCharacter->GetCombatComponent())
		{
			BlasterCharacter->GetCombatComponent()->CheckAndSetHUD_CarriedAmmo();
			BlasterCharacter->GetCombatComponent()->CheckAndSetHUD_ThrowGrenade();
		}
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

	//for crown stuff LOL:
	AGameState_Blaster* GameState_Blaster = GetWorld()->GetGameState<AGameState_Blaster>();
	if (PlayerState_Blaster && GameState_Blaster)
	{
		if (GameState_Blaster->TopStorePlayerStates.Contains(PlayerState_Blaster))
		{
			if(BlasterCharacter && HasAuthority()) BlasterCharacter->MulticastShowCrown();
		}
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

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController, MatchState);
	DOREPLIFETIME(ABlasterPlayerController, bIsTeamMatch);
}

void ABlasterPlayerController::PollInit()
{
	if (BlasterHUD == nullptr)
	{
		BlasterHUD = GetHUD<ABlasterHUD>();
		if (BlasterHUD) BlasterHUD->SetupBlasterHUD();
	}
}

void ABlasterPlayerController::UpdateServerClient_Delta_Periodically(float DeltaTime)
{
	AccumilatingTime += DeltaTime;
	if (IsLocalController() && AccumilatingTime > TimeSynchFrequency)
	{
		//this in turn call Client_ReportReqest. Finally update Delta_ServerMinusClient every 5s.
		Server_RequestServerTime(GetWorld()->GetTimeSeconds());
		AccumilatingTime = 0;
	}
}

//I decide to give the same name, why not!
void ABlasterPlayerController::OnMatchStateSet(const FName& InMatchState, bool InIsTeamMatch)
{
	MatchState = InMatchState;
	bIsTeamMatch = InIsTeamMatch; //call it here mean it set server times, but in fact to the same value so no worry

	if (MatchState == MatchState::InProgress)
	{
		//DoActionY()
		if (bIsTeamMatch) InitTeamScores();
		else HideTeamScores();

		//DoActionX()
		HandleMatchHasStarted(); 
	}
	else if (MatchState == MatchState::CoolDown)
	{
		HandleCoolDown();
	}
}

void ABlasterPlayerController::OnRep_MatchState()
{
	//it is not an enum so we can't use switch lol
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::CoolDown)
	{
		HandleCoolDown();
	}
}

void ABlasterPlayerController::OnRep_bIsTeamMatch()
{
	//DoActionY()
	if (bIsTeamMatch) InitTeamScores();
	else HideTeamScores();
}

//Name this function the same name as GameMode::HandleMatchHasStarted()<~InProgress <~GM::OnMatchStateSet()
void ABlasterPlayerController::HandleMatchHasStarted()
{
	//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;

	//by my way, upto this point they should be all valid, so check the point is enough, no need recheck
	CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;

	//Stephen gotta do it in PC::PollInit, but I do it here:
	if (CharacterOverlay_UserWidget) CharacterOverlay_UserWidget->AddToViewport();
	

	//gotta HIDE, not destroy for re-use after match, the UW_Announcement first, but anyway, it only render at the end of the frame, so I put it here:
	UserWidget_Announcement = UserWidget_Announcement == nullptr ? BlasterHUD->GetUserWidget_Announcement() : UserWidget_Announcement;
	if (UserWidget_Announcement == nullptr) return;

	UserWidget_Announcement->SetVisibility(ESlateVisibility::Hidden);
}

//need to remove WBP_Overlay from world, Hidden is not enough I guess, because if we start new game, we create a new one, so it will be duplicated if we dont remove it:
//set WBP_Announce back to visible to reuse it, we need to change text to match of course - we do it from outside dont worry, because we "re-use" so we can simply use SetHUD_X again with different context, hell yeah!I guess do it in thw TOTAL wrapper SetHUDTime() is a good idea
// though you can in fact create yet another WBP_CoolDown if you want:
void ABlasterPlayerController::HandleCoolDown()
{
	//BlasterHUD will be null in any simulated proxy, no matter what time or stage it is, where you want this bDisableMostInput is set to true in all proxies, hence you must do it INDEPDENTLY from BlasterHUD, you dont want to put this after the if (BlasterHUD == nullptr) return; LOL
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
	if (BlasterCharacter) BlasterCharacter->SetDisableMostInput(true);
	
	//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;

	//by my way, upto this point they should be all valid, so check the point is enough, no need recheck
	CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;
	//if (CharacterOverlay_UserWidget == nullptr) return; //you can use the style any more, they're now meant to be independent
		//Now Match is ended we need to remove it from world:
	if (CharacterOverlay_UserWidget) CharacterOverlay_UserWidget->RemoveFromParent();

	//gotta HIDE, not destroy for re-use after match, the UW_Announcement first, but anyway, it only render at the end of the frame, so I put it here:
	UserWidget_Announcement = UserWidget_Announcement == nullptr ? BlasterHUD->GetUserWidget_Announcement() : UserWidget_Announcement;
		//Turn it back on, change context from outside, to reuse it:
	if (UserWidget_Announcement) UserWidget_Announcement->SetVisibility(ESlateVisibility::Visible);
	//this is a bit overkill, as we already got BlasterHUD+WBP_ reference above, but any way this is my decision:
	SetHUDAnnounceAndInfo();
}

/*****GOLDEN4 group*****/
//this is executed on server, which is the only place GetGameMode() return valid
void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	//Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));//GetWorld()->GetAuthoGameMode() are the same
	if (BlasterGameMode)
	{
		LevelStartingTime = BlasterGameMode->GetLevelStartingTime();
		WarmUpTime = BlasterGameMode->GetWarmUpTime();
		MatchTime = BlasterGameMode->GetMatchTime();
		MatchState = BlasterGameMode->GetMatchState(); 
		ColdDownTime = BlasterGameMode->GetCoolDownTime();
	}
	ClientCheckMatchState(LevelStartingTime, WarmUpTime, MatchTime, ColdDownTime, MatchState);
	// Only Do this if you didn't do it in BlasterHUD, which I did, so I dont need, calling it here is no different than calling in PC::BeginPlay as this will be in turn called in BeginPlay, and UE5.2 didn't work in PC::BeginPlay, so it wont work for me :) PC constructor is done before BlasterHUD. hence PC::BeginPlay is done before PC::HUD I guess
		// if(MatchState=WaitingToStart && BlasterHUD) CreateWidget + AddToViewport WBP_Announcement 
}

//this is executed on owning client, who sent the ServerRPC above in this case, and help clients get values from GameMode:
void ABlasterPlayerController::ClientCheckMatchState_Implementation(float InLevelStartingTime, float InWarmUpTime, float InMatchTime, float InCoolDownTime, FName InMatchName)
{
	LevelStartingTime = InLevelStartingTime;
	WarmUpTime = InWarmUpTime;
	MatchTime = InMatchTime;
	MatchState = InMatchName;
	ColdDownTime = InCoolDownTime;
	//In worst case, the client joint 'after' the point MG::OnMatchStateSet()~>InProgress trigger, that it wont have those code update to clients at all, where the server PC will surely be updated, and possibly the only one, hence we dont add this line in Server___ above  
	OnMatchStateSet(MatchState, bIsTeamMatch); //PC:: _propogate()

	// Only Do this if you didn't do it in BlasterHUD, which I did, so I dont need, calling it here is no different than calling in PC::BeginPlay as this will be in turn called in BeginPlay, and UE5.2 didn't work in PC::BeginPlay, so it wont work for me :) PC constructor is done before BlasterHUD. hence PC::BeginPlay is done before PC::HUD I guess
	// if(MatchState=WaitingToStart && BlasterHUD) CreateWidget + AddToViewport WBP_Announcement 
}

//I think separate them into 2 cases is 'OPTIONAL', I reckon Delta ~ 0 if the one call Server_RequestServerTime is the server itself! but it is perfect to separate them:
float ABlasterPlayerController::GetServerTime_Synched()
{
	if (HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();
	}
	else
	{
		return GetWorld()->GetTimeSeconds() + Delta_ServerMinusServer;
	}
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
	//you can store this RoundTripTime if you want, or you can use 'PC::PlaterState::GetPing() * 4'
	//this will be updated every 5s as setup outside:
	//if it is PC an HasAuthority (either controlling server char or associate with some clients' char ), we expect TimeOfClientWhenRequesting = GetWorld()->GetTimeSeconds(), RTT=0, Delta=0
	RTT = GetWorld()->GetTimeSeconds() - TimeOfClientWhenRequesting;

	//This Delta already consider RRT/2: 
	Delta_ServerMinusServer =  (TimeOfServerWhenReceivedRequest + RTT/ 2) - GetWorld()->GetTimeSeconds() ;
}

/*SETHUD group:*/
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

void ABlasterPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;
	//UE_LOG(LogTemp, Warning, TEXT("BasterHUD valid"));

	CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;
	if (CharacterOverlay_UserWidget == nullptr) return;
	//UE_LOG(LogTemp, Warning, TEXT("CharacterOverlay_UserWidget valid"));

//Back to main business:
	float ShieldPercent = FMath::Clamp(Shield / MaxShield, 0.f, 1.f);
	if (CharacterOverlay_UserWidget) CharacterOverlay_UserWidget->SetShieldPercent(ShieldPercent);

	FString Text = FString::FromInt(Shield) + FString(" / ") + FString::FromInt(MaxShield);
	if (CharacterOverlay_UserWidget) CharacterOverlay_UserWidget->SetShieldText(Text);
}

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

void ABlasterPlayerController::SetHUDThrowGrenade(int InThrowGrenade)
{
	//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;

	CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;
	if (CharacterOverlay_UserWidget == nullptr) return;
	//Back to main business:
	CharacterOverlay_UserWidget->SetThrowGrenadeText(InThrowGrenade);
}


void ABlasterPlayerController::SetHUDRedTeamScore(int InRedTeamScore)
{
	//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;

	CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;
	if (CharacterOverlay_UserWidget == nullptr) return;
	//Back to main business:
	FString InString = FString::FromInt(InRedTeamScore);
	CharacterOverlay_UserWidget->SetRedTeamScoreText(InString);
}

void ABlasterPlayerController::SetHUDBlueTeamScore(int InBlueTeamScore)
{
	//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;

	CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;
	if (CharacterOverlay_UserWidget == nullptr) return;
	//Back to main business:
	FString InString = FString::FromInt(InBlueTeamScore);
	CharacterOverlay_UserWidget->SetBlueTeamScoreText(InString);
}

void ABlasterPlayerController::HideTeamScores()
{
	//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;

	CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;
	if (CharacterOverlay_UserWidget == nullptr) return;

	//Back to main business:
	CharacterOverlay_UserWidget->SetRedTeamScoreText(FString());
	CharacterOverlay_UserWidget->SetBlueTeamScoreText(FString());
	CharacterOverlay_UserWidget->SetTeamScoreSpacerText(FString());
}

void ABlasterPlayerController::InitTeamScores()
{
	//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;

	CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;
	if (CharacterOverlay_UserWidget == nullptr) return;

	//Back to main business:
	CharacterOverlay_UserWidget->SetRedTeamScoreText(FString("0"));
	CharacterOverlay_UserWidget->SetBlueTeamScoreText(FString("0"));
	CharacterOverlay_UserWidget->SetTeamScoreSpacerText(FString("|"));
}


//we calculate 'synched' timeleft from outside
void ABlasterPlayerController::SetHUDMatchTimeLeft(int32 InTimeLeft)
{
//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;

	CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;
	if (CharacterOverlay_UserWidget == nullptr) return;

	//optional for avoiding text reach 0 but is still shown in limbo transition in worst case, only = 0.f make sense as we only pass in "uint32 SecondsLeft" from outside so far, anyway I'm not sure this is even possible if we already handle the MatchState so well outside:
	if (InTimeLeft <= 0.f)
	{
		CharacterOverlay_UserWidget->SetMatchTimeLeftText(FString()); //empty
		return; //only make sense if we return early in this case
	}
	//Back to main business:
		//READY:
	int min = FMath::FloorToInt(InTimeLeft / 60.f); //must be Floor
	int sec = FMath::CeilToInt(InTimeLeft - min * 60.f); //Round - ceil - floor upto you!

	FString text = FString::FromInt(min) + FString(" : ") + FString::FromInt(sec);
	//text = FString::Printf(TEXT("%02d : %02d") , min , sec) - if you want 01 : 07 format!
	
	if(CharacterOverlay_UserWidget) CharacterOverlay_UserWidget->SetMatchTimeLeftText(text);
}

void ABlasterPlayerController::SetHUDWarmUpTimeLeft(int32 InTimeLeft)
{
//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;

	UserWidget_Announcement = UserWidget_Announcement == nullptr ? BlasterHUD->GetUserWidget_Announcement() : UserWidget_Announcement;
	if (UserWidget_Announcement == nullptr) return;

	//optional for avoiding text reach 0 but is still shown in limbo transition in worst case, only = 0.f make sense as we only pass in "uint32 SecondsLeft" from outside so far, anyway I'm not sure this is even possible if we already handle the MatchState so well outside:
	if (InTimeLeft <= 0.f)
	{
		UserWidget_Announcement->SetWarmUpTimeLeftText(FString()); //empty
		return; //only make sense if we return early in this case
	}
//Back to main business:
		//READY:
	int min = FMath::FloorToInt(InTimeLeft / 60.f); //must be Floor
	int sec = FMath::CeilToInt(InTimeLeft - min * 60.f); //Round - ceil - floor upto you!

	FString text = FString::FromInt(min) + FString(" : ") + FString::FromInt(sec);
	//text = FString::Printf(TEXT("%02d : %02d") , min , sec) - if you want 01 : 07 format!
	
	if (UserWidget_Announcement) UserWidget_Announcement->SetWarmUpTimeLeftText(text);
}

//Stephen didn't do this, But I like to do this to keep it consistent, this didn't need any parameter
void ABlasterPlayerController::SetHUDAnnounceAndInfo()
{
	//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;

	UserWidget_Announcement = UserWidget_Announcement == nullptr ? BlasterHUD->GetUserWidget_Announcement() : UserWidget_Announcement;
	//if (UserWidget_Announcement == nullptr) return;

	//Back to main business:
			//READY:
	FString AnnounceText;
	FString InfoText;

	//this is no need, WaitingToStart will default to hardcode text from WBP_Announce, nor did we call it anywhere yet when it reaches WaitingToStart LOL (OnMatchStateSet:: case WaitingToStart is emty), this only make sense when we call it there, which we didn't, Only me have this code :D
	// BUT i reckon it will come in handy later when we start a NEW match without re-seting BlasterHUD and its member widgets (if this is possible):
	if (MatchState == MatchState::WaitingToStart)
	{
		AnnounceText = FString("Match Starts in:");
		InfoText	 = FString("Fly Round: W,A,S,D");
		if (UserWidget_Announcement) UserWidget_Announcement->SetAnnounceText(AnnounceText);
	}

	//this check is also optional, as we call this function in HandleCoolDown(), that is already checked CoolDown already LOL:
	if (MatchState == MatchState::CoolDown)
	{
		AnnounceText = FString("New Match Starts in:");
		//empty for now, later we print the winner
		InfoText = FString();
//start to factorize for InfoText:
		if (bIsTeamMatch == false) GetTextInfo_CoolDownState_SoloMatch(InfoText);
		else GetTextInfo_CoolDownState_TeamMatch(InfoText);
//end factorize
		if (UserWidget_Announcement) UserWidget_Announcement->SetInfoText(InfoText);
	}
}

void ABlasterPlayerController::GetTextInfo_CoolDownState_SoloMatch(FString& InfoText)
{
	APlayerState_Blaster* ThisPlayerState = GetPlayerState<APlayerState_Blaster>();

	AGameState_Blaster* GameState_Blaster = GetWorld()->GetGameState<AGameState_Blaster>(); //UGameplayStatics::GetGameState(this);  
	TArray<APlayerState_Blaster*> PlayerStates_TopScore;
	if (GameState_Blaster) PlayerStates_TopScore = GameState_Blaster->TopStorePlayerStates;

	if (PlayerStates_TopScore.Num() == 0)
	{
		InfoText = FString("There is no winner.");
	}
	else if (PlayerStates_TopScore.Num() == 1)
	{
		if (ThisPlayerState == PlayerStates_TopScore[0]) InfoText = FString("You're winner.");
		else InfoText = FString("The winner is ") + PlayerStates_TopScore[0]->GetPlayerName();
	}
	else
	{
		InfoText = FString("The winner are:\n");
		for (int i = 0; i < PlayerStates_TopScore.Num(); i++)
		{
			InfoText.Append("    ");
			InfoText.Append(PlayerStates_TopScore[i]->GetPlayerName());
			InfoText.Append("\n");
		}
	}
}

//We dont need GS::PlayerArray to announce winner team, we only need GS::XTeamScore
//but if you decide to also print out the TopScore Player[s] you will need it again
void ABlasterPlayerController::GetTextInfo_CoolDownState_TeamMatch(FString& InfoText)
{
	APlayerState_Blaster* ThisPlayerState = GetPlayerState<APlayerState_Blaster>();

	AGameState_Blaster* GameState_Blaster = GetWorld()->GetGameState<AGameState_Blaster>(); //UGameplayStatics::GetGameState(this);  
	if (GameState_Blaster == nullptr) return;

		//no need here any more, will need if you decide to also print out the TopScore Player[s]:
		TArray<APlayerState_Blaster*> PlayerStates_TopScore;
		PlayerStates_TopScore = GameState_Blaster->TopStorePlayerStates;
	

	uint32 RedTeamScore = GameState_Blaster->RedTeamScore;
	uint32 BlueTeamScore = GameState_Blaster->BlueTeamScore;

	if (RedTeamScore == BlueTeamScore && RedTeamScore == 0)
	{
		InfoText = "No winner!\n";
	}
	else if (RedTeamScore == BlueTeamScore && RedTeamScore != 0)
	{
		InfoText = "Teams are tied!\n";
	}
	else if (RedTeamScore > BlueTeamScore)
	{
		InfoText = "Red Team is winner;\n";
	}
	else if (RedTeamScore < BlueTeamScore)
	{
		InfoText = "Blue Team is winner;\n";
	}
}


//the turn ON/OFF of WBP_Announce and WBP_Overlay already done externally, partly in PC::OnMatchStateSet(), so you dont have to worry about it here:
void ABlasterPlayerController::UpdateHUDTime()
{
//You can factorize these code into 'SetHUDTime()' if you want
//TimeLeft will be converted to int by C++ rule, so THE Math::CeilToInt is totally OPTIONAL
//we doing these so that we ONLY call SetHUDTimeLeft(TimeLeft) per second - not per frame, isn't it amazing?
//note that you can create 2 separate TimeLeft + 2 separate TimeLeftInt_LastSecond if you can merge them, however stephen has a very nice way to do them together so I follow.

//PART1: counting TimeLeft depends on which state we're in
	float TimeLeft{};
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
	else if (MatchState == MatchState::CoolDown)
	{
		//Timeleft is meant for CoolDownTimeLeft:
		TimeLeft = MatchTime - (GetServerTime_Synched() - LevelStartingTime - WarmUpTime - ColdDownTime);
	}


	//FMath::CeilToInt() = OPTIONAL, so that you see the MAX number longer a bit, if you dont do this line so change the type o TimeLeft above to uint32 instead for auto-conversion: 
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	//this is OPTIONAL (can even REDUDANT), Stephen say: as Server::PC::GetServerTime() subject to [send RPC to itself - execute RPC on itself, and subject to correction RTT/2, that may cause a little 'UNNECESSARY' inaccuracy] 
	//Hence stephen decide to directly get the TimeLeft = GM::CountingDownTime, directly from GM.
	//However I see that 'GetServerTime()' does to the If (HasAuthority()) check to avoid sending RPC already, so i'm not sure this is necessary any more, NOR did I see anything wrong if I dont do this at all, anyway just follow stephen HERE :)
	//UNLESS: LevelStartingTime are not const from clients? = this is incorrect, it is propogated from GM, unless the propogate is late per first frame, but anyway i dont think this is the case.
	if (HasAuthority())
	{
		//the hosting function is called per Second so we should cast this GameMode!
		BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;

		//Stephen says, we do need to add 'LevelStartingTime, but we already directly get from GM, and already add LevelStarttime from there, you're sure you're not mistaken? :D :D 
		SecondsLeft = FMath::CeilToInt(BlasterGameMode->GetCountingDownTime() ); //GM::CountingDownTime, subject to be re-used [WarmUpTime/Matchtime->0] from GM::Tick()
	}

//PART2: 'per-second' technique
	if (SecondsLeft != TimeLeftInt_LastSecond)
	{
		if (MatchState == MatchState::WaitingToStart) SetHUDWarmUpTimeLeft(SecondsLeft);

		else if (MatchState == MatchState::InProgress) SetHUDMatchTimeLeft(SecondsLeft);

		//you can merge as ||__ with first if, it re-use the same WBP
		else if (MatchState == MatchState::CoolDown) SetHUDWarmUpTimeLeft(SecondsLeft);

	}
	TimeLeftInt_LastSecond = SecondsLeft;
}

void ABlasterPlayerController::StartHighPingWarning()
{
	//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;

	CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;
	if (CharacterOverlay_UserWidget == nullptr) return;

	//Back to main business:
	CharacterOverlay_UserWidget->PlayWBPAnimation_PingWarning();
	//UE_LOG(LogTemp, Warning, TEXT("pass PC::StartHighPingWarning "))
}

void ABlasterPlayerController::StopHighPingWarning()
{
	//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;

	CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;
	if (CharacterOverlay_UserWidget == nullptr) return;
	//Back to main business:
	CharacterOverlay_UserWidget->StopWBPAnimation_PingWarning();
}