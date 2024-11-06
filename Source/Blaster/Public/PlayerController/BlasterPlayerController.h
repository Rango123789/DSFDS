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

	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(int InScore);
	void SetHUDDefeat(int InDefeat);

	void SetHUDAmmo(int InAmmo);

	void SetHUDCarriedAmmo(int InCarriedAmmo);

	void SetHUDTimeLeft(int32 MatchTimeLeft);

	virtual void OnPossess(APawn* InPawn) override;


protected:
	virtual void BeginPlay() override;

	//HUD and its Overlay widget (move from Character)
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;
	UPROPERTY()
	class UCharacterOverlay_UserWidget* CharacterOverlay_UserWidget;

	UPROPERTY(EditAnywhere)
	float TimeLeft = 120.f; //Call it MatchTime is also OKAY

public:

};
