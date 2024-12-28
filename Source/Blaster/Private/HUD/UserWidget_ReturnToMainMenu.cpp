// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/UserWidget_ReturnToMainMenu.h"
#include "Components/Button.h"
#include "MultiplayerSession_GameSubsystem.h"
#include "GameFramework/GameModeBase.h"
#include <Characters/BlasterCharacter.h>

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
	//now will be moved (i.e delayed) to  TimerCallback_Elim[/like] with SpawnRequestion
		//if(MSS) MSS->DestroySession(); 
	//now we do this first instead: access Char, calling Char::ServerLeaveGameRequest()
	if (GetWorld() && GetWorld()->GetFirstPlayerController())
	{
		ABlasterCharacter* BlasterCharacter
			= Cast<ABlasterCharacter>( GetWorld()->GetFirstPlayerController()->GetCharacter() );
		if (BlasterCharacter)
		{
			//you must bind it before call the chain lol: you dont need to RemoveDynamic for this, because if Char is left then its DATA goes with it too, if not, then let it bound, it is fine:
			if(!BlasterCharacter->OnSendingDestroySessionRequestDelegate_Char.IsBound())
			BlasterCharacter->OnSendingDestroySessionRequestDelegate_Char.AddDynamic(this, &ThisClass::OnSendingDestroySessionRequestDelegate_Char_callback);
			//this is the whole chain:
			BlasterCharacter->ServerLeaveGameRequest();
		}
	}
}

//at first it should be in TimerCallback_Elim(), but we decide to create FDelegate from char and broadcast it back here
//so that Char class dont even need to know about this WBP_Return widget nor need to include this class type there:
void UUserWidget_ReturnToMainMenu::OnSendingDestroySessionRequestDelegate_Char_callback()
{
	//no matter the outcome, it will trigger the OnDestroySessionComplete_Multiplayer below
	if(MSS) MSS->DestroySession(); 
}

//this is self-handled when it fails already:
void UUserWidget_ReturnToMainMenu::OnDestroySessionComplete_Multiplayer(bool bWasSuccessful)
{
	if (bWasSuccessful == false)
	{
		//must allow player another chance if he fails lol:
		Button_ReturnButton->SetIsEnabled(true);
	}

	if (bWasSuccessful == true)
	{
		if (MSS) MSS->OnDestroySessionCompleteDelegate_Multiplayer.RemoveDynamic(this, &ThisClass::OnDestroySessionComplete_Multiplayer);

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
