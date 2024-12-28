// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/CharacterOverlay_UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Animation/WidgetAnimation.h"

void UCharacterOverlay_UserWidget::SetHealthPercent(float InPercent)
{
	if(ProgressBar_Health) ProgressBar_Health->SetPercent(InPercent);
}

void UCharacterOverlay_UserWidget::SetHealthText(const FString& InString)
{
	if(TextBlock_Health) TextBlock_Health->SetText( FText::FromString(InString) );
}

void UCharacterOverlay_UserWidget::SetShieldPercent(float InPercent)
{
	if (ProgressBar_Shield) ProgressBar_Shield->SetPercent(InPercent);
}

void UCharacterOverlay_UserWidget::SetShieldText(const FString& InString)
{
	if (TextBlock_Shield) TextBlock_Shield->SetText(FText::FromString(InString));
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

void UCharacterOverlay_UserWidget::SetThrowGrenadeText(const int& InThrowGrenade)
{
	FString InString = FString::FromInt(InThrowGrenade);
	if (TextBlock_ThrowGrenade) TextBlock_ThrowGrenade->SetText(FText::FromString(InString));
}

void UCharacterOverlay_UserWidget::SetRedTeamScoreText(const int& InRedTeamScore)
{
	FString InString = FString::FromInt(InRedTeamScore);
	if (TextBlock_RedTeamScore) TextBlock_RedTeamScore->SetText(FText::FromString(InString));
}

void UCharacterOverlay_UserWidget::SetBlueTeamScoreText(const int& InBlueTeamScore)
{
	FString InString = FString::FromInt(InBlueTeamScore);
	if (TextBlock_BlueTeamScore) TextBlock_BlueTeamScore->SetText(FText::FromString(InString));
}

void UCharacterOverlay_UserWidget::PlayWBPAnimation_PingWarning()
{
	if (WBPAnimation_PingWarning && Image_PingWarning)
	{
		//it will play from the start so I dont think Image_PingWarning->SetOpacity(1.f); is needed? Unless it is different RenderOpacity? 
		// yep they're 2 different TIREs of the Opacity:
		//UImage : UWidget
		//UWidget::RenderOpacity (of parent part)  vs UImage::SetOpacity ( OpacityAndColor.A )
		//the reason why we need this is that we did use Image_PingWarning->SetOpacity(0.f); below
		//Also if we let WBPAnim work on top of ColorAndOpacity.A then only in this case we dont need it here, but still we need it in Stop___()
		//UPDATE: You dont need this any more, use bRestoreState = true so that it will reset back to previous state

		Image_PingWarning->SetOpacity(1.f);
		PlayAnimation(WBPAnimation_PingWarning , 0.f, 8, EUMGSequencePlayMode::Forward, 1.f);
	}
}

void UCharacterOverlay_UserWidget::StopWBPAnimation_PingWarning()
{
	if (WBPAnimation_PingWarning && Image_PingWarning)
	{
		//Stephen add this if check, but I dont think we need it right LOL? We can simply stop it no matter it is playing or not right?
		if (IsAnimationPlaying(WBPAnimation_PingWarning))
		{
			//UPDATE: You dont need this any more, use bRestoreState = true so that it will reset back to previous state
			//NO! it doesn't work well with "StopAnimation()" LOL, it work when it is stop naturally :D

			//I make a change on the same var! no it didn't work well neither!
			//Image_PingWarning->RenderOpacity=0.f;
			Image_PingWarning->SetOpacity(0.f);

			StopAnimation(WBPAnimation_PingWarning);

		}

		//this will fix it stop when RenderOpacity doesn't fall at '0'
		//if you do 'Image_PingWarning->RenderOpacity = 0;' instead, then you need to add Image_PingWarning->SetOpacity(1.f); above!
	}
}