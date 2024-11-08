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
	virtual void Tick(float DeltaTime) override;

	void UpdateHUDTime();

	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(int InScore);
	void SetHUDDefeat(int InDefeat);

	void SetHUDAmmo(int InAmmo);

	void SetHUDCarriedAmmo(int InCarriedAmmo);

	void SetHUDTimeLeft(int32 MatchTimeLeft);

	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION(Server, Reliable)
	void Server_RequestServerTime(float TimeOfClientWhenRequesting);
	UFUNCTION(Client, Reliable)
	void Client_ReportRequestBackToRequestingClient(float TimeOfClientWhenRequesting, float TimeOfServerWhenReceivedRequest);

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
	class ABlasterHUD* BlasterHUD;
	UPROPERTY()
	class UCharacterOverlay_UserWidget* CharacterOverlay_UserWidget;

	UPROPERTY(EditAnywhere)
	float MatchTime = 120.f; //Call it TimeLeft is also OKAY

	uint32 TimeLeftInt_LastSecond = 0;
public:

};
