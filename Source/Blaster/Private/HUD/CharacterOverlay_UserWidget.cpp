// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/CharacterOverlay_UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UCharacterOverlay_UserWidget::SetHealthPercent(float InPercent)
{
	ProgressBar_Health->SetPercent(InPercent);
}

void UCharacterOverlay_UserWidget::SetHealthText(const FString& InString)
{
	TextBlock_Health->SetText( FText::FromString(InString) );
}
