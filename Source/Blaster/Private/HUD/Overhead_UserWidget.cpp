// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/Overhead_UserWidget.h"
#include "Components/TextBlock.h"

//use this function to get the net role in string of a pawn and then in turn pass it into the next function to actually display it:

void UOverhead_UserWidget::ShowPlayerNetRole(AActor* InActor)
{
	if (InActor == nullptr) return;

	FString LocalRole;
	FString RemoteRole;

	ENetRole NetRole_Local  = InActor->GetLocalRole();
	ENetRole NetRole_Remote = InActor->GetRemoteRole();

	switch (NetRole_Local)
	{
	case ROLE_None: LocalRole = TEXT("None");
		break;
	case ROLE_SimulatedProxy: LocalRole = TEXT("SimulatedProxy");
		break;
	case ROLE_AutonomousProxy: LocalRole = TEXT("AutonomousProxy");
		break;
	case ROLE_Authority: LocalRole = TEXT("Authority");
		break;
	case ROLE_MAX: LocalRole = TEXT("MAX");
		break;
	default: LocalRole = TEXT("None");
		break;
	}

	switch (NetRole_Remote)
	{
	case ROLE_None: RemoteRole = TEXT("None");
		break;
	case ROLE_SimulatedProxy: RemoteRole = TEXT("SimulatedProxy");
		break;
	case ROLE_AutonomousProxy: RemoteRole = TEXT("AutonomousProxy");
		break;
	case ROLE_Authority: RemoteRole = TEXT("Authority");
		break;
	case ROLE_MAX: RemoteRole = TEXT("MAX");
		break;
	default: RemoteRole = TEXT("None");
		break;
	}
	
	LocalRole  = FString("Local Role: ")  + LocalRole;
	RemoteRole = FString("Remote Role: ") + RemoteRole;

	SetTextContent(LocalRole, LocalRole_TextBlock);
	SetTextContent(RemoteRole,RemoteRole_TextBlock);
}

void UOverhead_UserWidget::SetTextContent(FString TextToDisplay, UTextBlock* TextBlock)
{
	if (TextBlock == nullptr) return;
	TextBlock->SetText(FText::FromString(TextToDisplay));
}

//void UOverhead_UserWidget::NativeDestruct()
//{
//	//this will remove the widget from BlasterCharacter as it travels to new level?
//	RemoveFromParent();
//	Super::NativeDestruct();
//}

//Use this sub helper function to set content of TextBlock: indepenent step

