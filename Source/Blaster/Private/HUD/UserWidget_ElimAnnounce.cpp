// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/UserWidget_ElimAnnounce.h"
#include <Components/HorizontalBox.h>
#include <Components/TextBlock.h>
#include "PlayerStates/PlayerState_Blaster.h"

void UUserWidget_ElimAnnounce::SetTextBlock_Elim(const FString& AttackPlayer, const FString& ElimmedPlayer)
{
	if (TextBlock_Elim)
	{
		FString Message = AttackPlayer + FString(" eliminated ") + ElimmedPlayer;

		TextBlock_Elim->SetText( FText::FromString(Message) );
	}

}
