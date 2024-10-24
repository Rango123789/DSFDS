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
