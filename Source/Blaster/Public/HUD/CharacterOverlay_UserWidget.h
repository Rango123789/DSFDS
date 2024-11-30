// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay_UserWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UCharacterOverlay_UserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetHealthPercent(float InPercent);
	void SetHealthText(const FString& InString);
	void SetShieldPercent(float InPercent);
	void SetShieldText(const FString& InString);

	void SetScoreText(const int& InScore);
	void SetDefeatText(const int& InDefeat);
	void SetAmmoText(const int& InAmmo);
	void SetCarriedAmmoText(const int& InCarriedAmmo);
	void SetMatchTimeLeftText(const FString& InString);
	void SetThrowGrenadeText(const int& InThrowGrenade);
protected:

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ProgressBar_Health;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextBlock_Health;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ProgressBar_Shield;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextBlock_Shield;


	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextBlock_Score;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextBlock_Defeat;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextBlock_Ammo;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextBlock_CarriedAmmo;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextBlock_MatchTimeLeft;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextBlock_ThrowGrenade;
private:



};
