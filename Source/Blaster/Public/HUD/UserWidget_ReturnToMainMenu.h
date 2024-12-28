// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UserWidget_ReturnToMainMenu.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UUserWidget_ReturnToMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	//follow the standard pattern like we did in Menu class in Plugin module:
		//when the button appear you need to ShowCursor, enable UI+ InputMode (*)
	UFUNCTION(BlueprintCallable)
	void MenuSetup();
	//when player press widget button to leave the level, you do whatever appropriate for the travelled-to level (here StarterMap, so you dont even need to make changes on it as it already appropriate LOL) 
	    //but if player press Q input again to stay, you need to UNDO (*)
	UFUNCTION(BlueprintCallable)
	void MenuTearDown();

	UFUNCTION()
	void OnSendingDestroySessionRequestDelegate_Char_callback();

protected:
	//think of it as BeginPlay(), but whether it can trigger move betwen level I'm not sure (but what I'm sure is that if we call MenuTearDown()/RemoveFromParent() it wont be called any more, simply because the WidgetObject could be destroyed after it, unless we did create a reference to store the return of CreateWidget() at first place; in any case, RemoveFromParent() is enough to stop Initalize() from re-calling I guess even if the widget object is still behind the scene - not AddtoViewport, not happen!)
	//UPDATE: I'm not sure about this when say bind callbacks in it could be too early LOL
	//BUT i check it in Menu class, it is totally fine!
	virtual bool Initialize() override;

	//trigger when we travel from one level to another (shouldn't happen again if it is already RemoveFromParent)
	virtual void NativeDestruct() override;



	UFUNCTION()
	void OnClicked_ReturnButton();

	//I add _Multiplayer to mean this callback will be bound to delegate object from MSSubsystem class
	//similar to the way I did but from Menu::callback!
	UFUNCTION()
	void OnDestroySessionComplete_Multiplayer(bool bWasSuccessful);

	UPROPERTY(meta = (BindWidget))
	class UButton* Button_ReturnButton;

	//better create it to be re-used many times:
	UPROPERTY()
	class UMultiplayerSession_GameSubsystem* MSS;

	UPROPERTY()
	APlayerController* PC;

};





