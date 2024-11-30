// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

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
	void UpdateServerClient_Delta_Periodically(float DeltaTime);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void PollInit();

	void UpdateHUDTime();

	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(int InScore);
	void SetHUDDefeat(int InDefeat);

	void SetHUDAmmo(int InAmmo);

	void SetHUDCarriedAmmo(int InCarriedAmmo);

	void SetHUDThrowGrenade(int InThrowGrenade);

	void SetHUDMatchTimeLeft(int32 MatchTimeLeft);

	void SetHUDWarmUpTimeLeft(int32 InTimeLeft);

	void SetHUDAnnounceAndInfo();

	virtual void OnPossess(APawn* InPawn) override;

	virtual void ReceivedPlayer() override; //Synched with server clock as soon as possible
	float GetServerTime_Synched(); //Synched with server world clock
	
	float Delta_ServerMinusServer = 0; //it will be different soon
	float AccumilatingTime = 0; //when reach 5s, call 
	UPROPERTY(EditAnywhere)
	float TimeSynchFrequency = 5.f;


protected:
	virtual void BeginPlay() override;

	//HUD and its Overlay widget (move from Character)
	UPROPERTY()
	class ABlasterHUD* BlasterHUD = nullptr;
	UPROPERTY()
	class UCharacterOverlay_UserWidget* CharacterOverlay_UserWidget = nullptr;

	UPROPERTY()
	class UUserWidget_Announcement* UserWidget_Announcement = nullptr;

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

	//old, to be propogated from Gm
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

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
	void OnMatchStateSet(const FName& InMatchState); //more than set, so I do it in .cpp
	void HandleMatchHasStarted(); 
	void HandleCoolDown();
	FName GetMatchState() { return MatchState; }

};
