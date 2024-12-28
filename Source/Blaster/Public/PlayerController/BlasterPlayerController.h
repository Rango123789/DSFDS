// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReportPingDelegate, bool, bHighPing);

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:	
	ABlasterPlayerController(); //for testing
	virtual void Tick(float DeltaTime) override;
	void CheckPing(float DeltaTime);
	void UpdateServerClient_Delta_Periodically(float DeltaTime);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void PollInit();

	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(int InScore);
	void SetHUDDefeat(int InDefeat);

	void SetHUDAmmo(int InAmmo);
	void SetHUDCarriedAmmo(int InCarriedAmmo);
	void SetHUDThrowGrenade(int InThrowGrenade);

	void SetHUDRedTeamScore(int InRedTeamScore);
	void SetHUDBlueTeamScore(int InBlueTeamScore);
	//to be call in X::BeginPlay() or like when BP_BlasterGM is selected
	void HideTeamScores();
	//to be call in X::BeginPlay() or like when BP_TeamBlasterGM is selected
	void InitTeamScores();


	void UpdateHUDTime();
	void SetHUDMatchTimeLeft(int32 MatchTimeLeft);
	void SetHUDWarmUpTimeLeft(int32 InTimeLeft);

	void SetHUDAnnounceAndInfo();

	//if this run in the server, only the CD receives it, so to broadcast message to all devices we must interate over the whole array of PCs and call this ClientRPC per each PC: similar to what we did in GM::OnMatchStateSet()
	UFUNCTION(Client, Reliable)
	void ClientSetHUD_ElimAnnounce(class APlayerState_Blaster* AttackPlayer, class APlayerState_Blaster* ElimmedPlayer);

	virtual void OnPossess(APawn* InPawn) override;
	virtual void ReceivedPlayer() override; //Synched with server clock as soon as possible in here

//synching time:
	UPROPERTY(EditAnywhere)
	float TimeSynchFrequency = 5.f;
	float AccumilatingTime = 0; //when reach 5s, call 

	float GetServerTime_Synched(); //Synched with server world clock
		//they will be update every 5s, we expect all PCs in the server have them "ZERO" :
	float Delta_ServerMinusServer = 0; //it will be different soon in clients
	float RTT = 0; //it will be different soon in clients



//HighPingWarning:
	void StartHighPingWarning();
	void StopHighPingWarning();

	float RunningTime_HighPing = 0.f;
	
	UPROPERTY(EditAnywhere)
	float PingWarningDuration = 5.f;

	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f; //in miliseconds

	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f;

		//this is to stop the WBP if it is playing enough PingWarningDuration:
	float RunningTime_HighPingAnimation = 0.f;

	bool bHighPingAnimationPlaying = false;

	//for server-side rewind purpose:
	FOnReportPingDelegate OnReportPingStatus_Delegate;

	UFUNCTION(Server, Reliable)
	void Server_ReportPingStatus(bool bHighPing);


protected:
	virtual void BeginPlay() override;

	//for any input can be done in PC rather than in Char as usual that it can be absent:
	virtual void SetupInputComponent() override;

	//this is separate IMC for PC
	UPROPERTY(EditAnywhere)
	class UInputMappingContext* IMC_Blaster_PC;

	UPROPERTY(EditAnywhere)
	class UInputAction* IA_ReturnToMainMenu;

	//important bool to know it is open or collapsed:
	bool bIsRetunWidgetOpen = false;

	void Input_ReturnToMainMenu(const struct FInputActionValue& Value);


	//HUD and its Overlay widget (move from Character)
	UPROPERTY()
	TObjectPtr<class ABlasterHUD> BlasterHUD;


	UPROPERTY()
	class UCharacterOverlay_UserWidget* CharacterOverlay_UserWidget = nullptr;

	UPROPERTY()
	class UUserWidget_Announcement* UserWidget_Announcement = nullptr;

	UPROPERTY()
	class UUserWidget_ElimAnnounce* UserWidget_ElimAnnounce = nullptr;

	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;

	//new, to be propogated from GM:
	float LevelStartingTime = 0.f;

	//new, to be propogated from GM:
	float WarmUpTime = 0.f;

	//old, I set it back to 0, so that it will be propogated from GM naturally
	float MatchTime = 0.f; //120.f

	//new, to be propogated from GM:
	float ColdDownTime = 0.f;

	uint32 TimeLeftInt_LastSecond = 0;

	//to be propogated from GM
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;
	UFUNCTION()
	void OnRep_MatchState();

	//to be propogated from GM
	UPROPERTY(ReplicatedUsing = OnRep_bIsTeamMatch)
	bool bIsTeamMatch;
		//I feel like we can hitchhike with OnRep_MatchState()? avoid this need?
		//also we currently only need bIsMatchTeam to be replicated to clients, nothing more!
	    //no we need it because 'DoActionY in OnRep_X is risky'
	UFUNCTION()
	void OnRep_bIsTeamMatch();


	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	//Stephen call this ClientJoinMidGame() LOL
	UFUNCTION(Client, Reliable)
	void ClientCheckMatchState(float InLevelStartingTime, float InWarmUpTime, float InMatchTime , float InCoolDownTime, FName InMatchName);

	UFUNCTION(Server, Reliable)
	void Server_RequestServerTime(float TimeOfClientWhenRequesting);
	UFUNCTION(Client, Reliable)
	void Client_ReportRequestBackToRequestingClient(float TimeOfClientWhenRequesting, float TimeOfServerWhenReceivedRequest);

public:
	//stephen set InIsTeamMatch = false by default, but really this is absolute redudant as we will call and pass GM::bIsTeamMatch at the time we call it there, so LOL, you dont have to do this: 
	void OnMatchStateSet(const FName& InMatchState, bool InIsTeamMatch = false); //more than set, so I do it in .cpp
	void HandleMatchHasStarted(); 
	void HandleCoolDown();
	FName GetMatchState() { return MatchState; }

};
