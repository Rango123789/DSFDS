// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/BlasterPlayerController.h"
#include "HUD/BlasterHUD.h"
#include "HUD/CharacterOverlay_UserWidget.h"

void ABlasterPlayerController::BeginPlay()
{
	//move this to AplayerController:

		BlasterHUD = Cast<ABlasterHUD>(GetHUD());
		if(BlasterHUD) CharacterOverlay_UserWidget = BlasterHUD->GetCharacterOverlay_UserWidget();
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;		
	if (BlasterHUD == nullptr) return;

	CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;
	if (CharacterOverlay_UserWidget == nullptr) return;

//Back to main business:
	CharacterOverlay_UserWidget->SetHealthPercent(Health / MaxHealth);

	FString Text = FString::FromInt(Health) + FString(" / ") + FString::FromInt(MaxHealth);
	CharacterOverlay_UserWidget->SetHealthText(Text);
}
