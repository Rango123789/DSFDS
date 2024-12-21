// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/UserWidget_ReturnToMainMenu.h"
#include "Components/Button.h"
#include "MultiplayerSession_GameSubsystem.h"
#include "GameFramework/GameModeBase.h"

bool UUserWidget_ReturnToMainMenu::Initialize()
{
	if(Super::Initialize() == false) return false; //conventional line

	//MenuSetup(); //do it outside when you create the widget also okay, but dont do it unless we have to

	//if(Button_ReturnButton) Button_ReturnButton->OnClicked.AddDynamic(this, &ThisClass::OnClicked_ReturnButton);

	return true; //conventional line
}

//stephen didn't use this so far:
void UUserWidget_ReturnToMainMenu::NativeDestruct()
{

	Super::NativeDestruct();
}

void UUserWidget_ReturnToMainMenu::MenuSetup()
{
	if (GetGameInstance() == nullptr) return;
//side things:
	//you can in fact, add here or from the hosting class creating it also okay:
	//becareful you dont want to add it gain when you keep press Q again and again (perhaps it may render a lot version of this widget object onto the screen, reduce performance)
	AddToViewport();

	//we doing these on TIRE0 UserWidget (hence all of its sub widgets will be under effect too)
	SetVisibility(ESlateVisibility::Visible);  //it should be Visible by default, but anyway
	SetIsFocusable(true); //it should be true by default, but anyway

	//stephen bind callback for Button::OnClick here:
	if (Button_ReturnButton && !Button_ReturnButton->OnClicked.IsBound()) Button_ReturnButton->OnClicked.AddDynamic(this, &ThisClass::OnClicked_ReturnButton);

	//MOVE the binding callback up here:
	if(MSS == nullptr) MSS =  GetGameInstance()->GetSubsystem<UMultiplayerSession_GameSubsystem>();
	if (MSS)
	{
		MSS->OnDestroySessionCompleteDelegate_Multiplayer.AddDynamic(this, &ThisClass::OnDestroySessionComplete_Multiplayer);
	}
//conventional things: InputMode , Cursor
	if (GetWorld() && Button_ReturnButton)
	{
		PC = PC == nullptr? GetWorld()->GetFirstPlayerController() : PC; 
		//the reason I choose this is that you may also need to press 'Q' again to stay (where user input require at least GameOnly)
		FInputModeGameAndUI InputModeGameAndUI;
			//me: a widget gain focus mean you can press ENTER/like to activate its OnClicked/LIKE funtion! but it doesn't mean that you can't click it to activate it instead LOL
		InputModeGameAndUI.SetWidgetToFocus(Button_ReturnButton->TakeWidget()); //optional, not compatible type, need conversion? yes UWidget::TakeWidget() will do it!
			//stephen:
		//InputModeGameAndUI.SetWidgetToFocus(TakeWidget()); //focus on TIRE0, if TIRE0 only has one button TIRE1 then it will auto-focus on that only TIRE1 I guess

		if (PC)
		{
			PC->SetInputMode(InputModeGameAndUI);
			PC->SetShowMouseCursor(true);
		}
	}
}

//this only for the case for when player press Q again, not for the case player click on the button:
//meaning you CAN NOT call this in NativeDestruct LOL
void UUserWidget_ReturnToMainMenu::MenuTearDown()
{
//side things:
	RemoveFromParent(); //OR ESlateVisibility = hidden will also do!

	if (MSS)
	{
		MSS->OnDestroySessionCompleteDelegate_Multiplayer.RemoveDynamic(this, &ThisClass::OnDestroySessionComplete_Multiplayer);
	}

//main things:
	if (GetWorld())
	{
		PC = PC == nullptr ? GetWorld()->GetFirstPlayerController() : PC;

		//because stephen plan to call this MenuTearDown when we press Q again and continue the game, so:
		FInputModeGameAndUI InputModeGameOnly;

		if (PC)
		{
			PC->SetInputMode(InputModeGameOnly);
			PC->SetShowMouseCursor(false);
		}
	}
}

//press Q isn't actually clicking anything lol, click after press Q will actually trigger this
//meaning you want to leave the current GameMap level:
void UUserWidget_ReturnToMainMenu::OnClicked_ReturnButton()
{
	//if (GetGameInstance() == nullptr) return;
//side things:
	//we need to disable button when this callback is reached, avoid player too worry and spam the button:
	Button_ReturnButton->SetIsEnabled(false);

//main things:
	//access MSSubsystem, you see you can access it anywhere immediately (as long as you include required file or add required module):
		//MSS = MSS == nullptr? GetGameInstance()->GetSubsystem<UMultiplayerSession_GameSubsystem>() : MSS; ////MOVE up to ...
	
	//we need to bind an extra callback of this UW_ReturnToMenu to MSSubsystem::OnDestroySessonDelegate / OnDestroySessonDelegate_Multiplayer, so that it will trigger when 'OS::OSInterface::DestroySession' response - i,e you can bind it right here if you want:
		//if(MSS) MSS->OnDestroySessionCompleteDelegate_Multiplayer.AddDynamic(this, &ThisClass::OnDestroySessionComplete_Multiplayer); //MOVE up to ...

	//call MSSubsystem::DestroySession() to quit (or directly access and call OS::OSInterface::DestroySession)
	if(MSS) MSS->DestroySession(); //no matter the outcome, it will trigger the OnDestroySessionComplete_Multiplayer below
}

void UUserWidget_ReturnToMainMenu::OnDestroySessionComplete_Multiplayer(bool bWasSuccessful)
{
	if (bWasSuccessful == false)
	{
		//must allow player another chance if he fails lol:
		Button_ReturnButton->SetIsEnabled(true);

		//if fails, try to send DestroySession request again: this is optional and up to you, but after all it is not recommended LOL, what if the player lose Net connection and it gives him an infinate loop LOL?
		//if (MSS) MSS->DestroySession();
	}

	if (bWasSuccessful == true)
	{
		//you should 'unbind' the callback when you succeed right? otherwise this is the consequence: this class will be removed from parent, potentially destroy this widget object too, where the GameInstance::MMS persist across level - not good!
		//note that there could be a second callback in Menu bind to this very same delegate from MSSubsystem LOL, isn't interesting? 2 callbacks from external classes to a REAL indepdendent class MSSubsystem!
		if (MSS) MSS->OnDestroySessionCompleteDelegate_Multiplayer.RemoveDynamic(this, &ThisClass::OnDestroySessionComplete_Multiplayer);

		//Ready all side things before come back to the StartMap: it need UIOnly inputmode, Show Cursor ; ...

		//Travel back to StartMap: if it is the server use GameMode to travel, if it is a client use PC to travel - not why this is recommended and classical practice lol:
		//the funny fact is that we use 'GM/PC::ReturnToMainMenu' to return to 'Default Map' set in Project Setting(currently StarterMap), rather than "OpenLevel / ServerTravel" and must specify the path/name of the level LOL, so yeah!
		AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();

		if (GameMode)
		{
			GameMode->ReturnToMainMenuHost();
		}
		else
		{
			PC = PC == nullptr ? GetWorld()->GetFirstPlayerController() : PC;

			//if (PC) PC->ClientReturnToMainMenu(); //get warning, obsolete
			if (PC) PC->ClientReturnToMainMenuWithTextReason(FText()); //now use this
		}
	}
}

//My first idea without considering it is the server or a client, and forget to disable button avoiding player too worry and spam the button LOL
//keep in mind that this function will be in Input_Q chain and trigger in CD only, so there is no chance it will trigger in more than device, so dont worry!
//void UUserWidget_ReturnToMainMenu::OnDestroySessionComplete_Multiplayer(bool bWasSuccessful)
//{
//	//if (GetGameInstance() == nullptr) return;
//	//MSS = MSS == nullptr ? GetGameInstance()->GetSubsystem<UMultiplayerSession_GameSubsystem>() : MSS;
//
//	if (bWasSuccessful)
//	{
//		//you should 'unbind' the callback when you succeed right? otherwise this is the consequence: this class will be removed from parent, potentially destroy this widget object too, where the GameInstance::MMS persist across level - not good!
//		if (MSS) MSS->OnDestroySessionCompleteDelegate_Multiplayer.RemoveDynamic(this, &ThisClass::OnDestroySessionComplete_Multiplayer);
//		
//		//Ready all side things before come back to the StartMap: it need UIOnly inputmode, Show Cursor ; ...
//
//		//Travel back to StartMap:
//
//	}
//	else
//	{
//		//if fails, try to send DestroySession request again:
//		if (MSS) MSS->DestroySession();
//	}
//}
