// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/BlasterPlayerController.h"
#include "HUD/BlasterHUD.h"
#include "HUD/CharacterOverlay_UserWidget.h"
#include "Characters/BlasterCharacter.h"
#include <PlayerStates/PlayerState_Blaster.h>

void ABlasterPlayerController::BeginPlay()
{
	//move this to AplayerController:

		BlasterHUD = Cast<ABlasterHUD>(GetHUD());
		if(BlasterHUD) CharacterOverlay_UserWidget = BlasterHUD->GetCharacterOverlay_UserWidget();
}

//this is for respawning - if needed - only KEY vars from Char need this
void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	//OPTION1: this work ->I forget to call Super:: LOL, this prove that the order of PC::OnPossess and Char::BeginPlay has been changed when GameStart and respawn the char.
		//ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
		//if (BlasterCharacter) BlasterCharacter->UpdateHUD_Health();

	//OPTION2: So this has to work, need only to access char::Health and MaxHealth for PC::SetHUDHealth( , ) right here
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if (BlasterCharacter)
	{
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
	}

	//Optional so far:
	APlayerState_Blaster* PlayerState_Blaster = GetPlayerState<APlayerState_Blaster>();
	if (BlasterCharacter && PlayerState_Blaster)
	{
		SetHUDScore(PlayerState_Blaster->GetScore()); //medicine3
		//SetHUDDefeat(PlayerState_Blaster->GetDefeat()); //medicine3
	}
}

void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//can factorize them into SetHUDTimeLeft, let it receive "TimeLeft" instead!
	TimeLeft -= DeltaTime;
	int min = FMath::FloorToInt(TimeLeft / 60.f); //must be Floor
	int sec = FMath::CeilToInt(TimeLeft - min * 60.f); //Round - ceil - floor upto you!

	FString text = FString::FromInt(min) + FString(" : ") + FString::FromInt(sec);
	
	//text = FString::Printf(TEXT("%02d : %02d") , min , sec) - if you want 01 : 07 format!

	SetHUDTimeLeft(TimeLeft);
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;		
	if (BlasterHUD == nullptr) return;
	//UE_LOG(LogTemp, Warning, TEXT("BasterHUD valid"));

	CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;
	if (CharacterOverlay_UserWidget == nullptr) return;
	//UE_LOG(LogTemp, Warning, TEXT("CharacterOverlay_UserWidget valid"));

//Back to main business:
	CharacterOverlay_UserWidget->SetHealthPercent(Health / MaxHealth);

	FString Text = FString::FromInt(Health) + FString(" / ") + FString::FromInt(MaxHealth);
	CharacterOverlay_UserWidget->SetHealthText(Text);
}

//the pattern is the same as SetHUDHealth above
void ABlasterPlayerController::SetHUDScore(int InScore)
{
//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;

	CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;
	if (CharacterOverlay_UserWidget == nullptr) return;
//Back to main business:
	CharacterOverlay_UserWidget->SetScoreText(InScore);
}

void ABlasterPlayerController::SetHUDDefeat(int InDefeat)
{
//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;

	CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;
	if (CharacterOverlay_UserWidget == nullptr) return;
//Back to main business:
	CharacterOverlay_UserWidget->SetDefeatText(InDefeat);
}

void ABlasterPlayerController::SetHUDAmmo(int InAmmo)
{
	//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;

	CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;
	if (CharacterOverlay_UserWidget == nullptr) return;
	//Back to main business:
	CharacterOverlay_UserWidget->SetAmmoText(InAmmo);
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int InCarriedAmmo)
{
	//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;

	CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;
	if (CharacterOverlay_UserWidget == nullptr) return;
	//Back to main business:
	CharacterOverlay_UserWidget->SetCarriedAmmoText(InCarriedAmmo);
}

void ABlasterPlayerController::SetHUDTimeLeft(int32 InTimeLeft)
{
	//this 4 lines is my style:
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr) return;

	CharacterOverlay_UserWidget = CharacterOverlay_UserWidget == nullptr ? BlasterHUD->GetCharacterOverlay_UserWidget() : CharacterOverlay_UserWidget;
	if (CharacterOverlay_UserWidget == nullptr) return;
	//Back to main business:
	
		//READY:
	InTimeLeft -= GetWorld()->GetDeltaSeconds();
	int min = FMath::FloorToInt(InTimeLeft / 60.f); //must be Floor
	int sec = FMath::CeilToInt(InTimeLeft - min * 60.f); //Round - ceil - floor upto you!

	FString text = FString::FromInt(min) + FString(" : ") + FString::FromInt(sec);
	//text = FString::Printf(TEXT("%02d : %02d") , min , sec) - if you want 01 : 07 format!
	
		//CALL:
	CharacterOverlay_UserWidget->SetTimeLeftText(text);
}

