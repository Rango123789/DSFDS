// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/CharacterOverlay_UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UCharacterOverlay_UserWidget::SetHealthPercent(float InPercent)
{
	if(ProgressBar_Health) ProgressBar_Health->SetPercent(InPercent);
}

void UCharacterOverlay_UserWidget::SetHealthText(const FString& InString)
{
	if(TextBlock_Health) TextBlock_Health->SetText( FText::FromString(InString) );
}

void UCharacterOverlay_UserWidget::SetScoreText(const int& InScore)
{
	FString InString = FString("Score: ") + FString::FromInt(InScore);
	if (TextBlock_Score) TextBlock_Score->SetText(FText::FromString(InString));
}

void UCharacterOverlay_UserWidget::SetDefeatText(const int& InDefeat)
{
	FString InString = FString("Defeat: ") + FString::FromInt(InDefeat);
	if (TextBlock_Defeat) TextBlock_Defeat->SetText(FText::FromString(InString));
}

void UCharacterOverlay_UserWidget::SetAmmoText(const int& InAmmo)
{
	FString InString = FString("Ammo: ") + FString::FromInt(InAmmo);
	if (TextBlock_Ammo) TextBlock_Ammo->SetText(FText::FromString(InString));
}

void UCharacterOverlay_UserWidget::SetCarriedAmmoText(const int& InCarriedAmmo)
{
	FString InString = FString("| CarriedAmmo: ") + FString::FromInt(InCarriedAmmo);
	if (TextBlock_CarriedAmmo) TextBlock_CarriedAmmo->SetText(FText::FromString(InString));
}

void UCharacterOverlay_UserWidget::SetMatchTimeLeftText(const FString& InString)
{
	if (TextBlock_MatchTimeLeft) TextBlock_MatchTimeLeft->SetText(FText::FromString(InString));
}



