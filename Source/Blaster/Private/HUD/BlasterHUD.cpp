// Fill out your copyright notice in the Description page of Project Settings.
#include "HUD/BlasterHUD.h"
#include "HUD/CharacterOverlay_UserWidget.h"
#include "HUD/UW_Announcement.h"
#include "PlayerController/BlasterPlayerController.h"
#include "HUD/UserWidget_ReturnToMainMenu.h"
#include "HUD/UserWidget_ElimAnnounce.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h" //not sure we need this?
#include "Blueprint/WidgetLayoutLibrary.h" //need it for UWidgetLayoutLibrary::SlotAsPanelSlot()

ABlasterHUD::ABlasterHUD()
{
	if (GetWorld()) UE_LOG(LogTemp, Warning, TEXT("HUD,  Constructor Time: %f "), GetWorld()->GetTimeSeconds())
}

void ABlasterHUD::BeginPlay()
{
	if (GetWorld()) UE_LOG(LogTemp, Warning, TEXT("HUD,  BeginPlay Time: %f "), GetWorld()->GetTimeSeconds())

	Super::BeginPlay();

	SetupBlasterHUD();
}

void ABlasterHUD::CreateAndAddElimAnnounce(const FString& AttackPlayer, const FString& ElimmedPlayer)
{
	ABlasterPlayerController* PlayerController = Cast<ABlasterPlayerController>(GetOwningPlayerController());

	//CreateWidget:
	UUserWidget_ElimAnnounce* ElimAnnounce = nullptr;
	if (PlayerController && ElimAnnounce_Class)
	{
		//you can in fact create a local member if you want:
		ElimAnnounce = CreateWidget<UUserWidget_ElimAnnounce>(PlayerController, ElimAnnounce_Class);
	}

	//Set value and AddToViewport: we can simply add the new message directly to Viewport, but for the old messages we move them up move their slot in Canvas Panel up will move them up)
	if (ElimAnnounce)
	{
		ElimAnnounce->SetTextBlock_Elim(AttackPlayer, ElimmedPlayer);
		ElimAnnounce->AddToViewport();
	}

	//as long as we didn't add the new message yet, the array is still holding old messages upto last last frame (not including the newly-created message in this frame yet)
	//let's iterate over the array and push up 'not-yet-destroyed message' up:
	for (auto Message : UserWidget_ElimAnnounce_Array)
	{
		if (Message == nullptr || Message->HorizontalBox_Elim == nullptr || Message->TextBlock_Elim == nullptr) continue;

		//OPTION1: apparently OBSOLETE and somehow not working any more, even if stephen code in UE5.0 work, or may be still working but not normally LOL: it still compile well and work normally
			//UCanvasPanelSlot* CanvasPanelSlot1 = UWidgetLayoutLibrary::SlotAsCanvasSlot(Message->HorizontalBox_Elim);
		//OPTION2: interesting question the text block is in HBox, if I move one slot up, the other will follow right? :D
		//UCanvasPanelSlot* CanvasPanelSlot2 = Cast<UCanvasPanelSlot>(Message->TextBlock_Elim->Slot);
		UCanvasPanelSlot* CanvasPanelSlot2 = Cast<UCanvasPanelSlot>(Message->HorizontalBox_Elim->Slot); //it sitll compile well and work normally LOL
		if (CanvasPanelSlot2)
		{
			FVector2D CurrentPosition = CanvasPanelSlot2->GetPosition();
			FVector2D NewPosition = { CurrentPosition.X , CurrentPosition.Y - CanvasPanelSlot2->GetSize().Y };

			CanvasPanelSlot2->SetPosition(NewPosition);

		}
	}

	//Store the just-created widget object to the array, now it just includes the newly-created message
	//And set a TimerCall to destroy it too:
	if (ElimAnnounce)
	{

		//We can in fact create local vars of them, and you can in fact pass in "value" for callback at the time you do TimerDelagate.BindUFunction( , value1, value2, ...), but NOT when .SetTimer
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelagate;
		//not work:
			//TimerDelagate.BindUFunction(this, &ThisClass::TimerCallback_RemoveElimAnnounce, UserWidget_ElimAnnounce_Array, true);
		//work:
		TimerDelagate.BindUFunction(this, 
			FName("TimerCallback_RemoveElimAnnounce"), 
			ElimAnnounce, //first value for the callback
			true //second value (test) for the callback
		);

		GetWorldTimerManager().SetTimer(
			TimerHandle,
			TimerDelagate,
			TimerDelay_RemoveElimAnnounce,
			false
		);

		//you should do this final, as it may invilidate some iterator or pointer to element of array, but in fact we didn't use any such a thing here, so I dont it will change any thing lol:
		UserWidget_ElimAnnounce_Array.Add(ElimAnnounce);
	}
}

void ABlasterHUD::TimerCallback_RemoveElimAnnounce(UUserWidget_ElimAnnounce* Message, bool TestBool)
{
	if (Message)
	{
		Message->RemoveFromParent();

		//you need to remove it from the array, not destroy itself, that element pointer still exist and point to INVALID data LOL:
		//after add or remove, the array may change its element memory location, but the fact there value are in fact hence it wont cause any issue dont worry!
		UserWidget_ElimAnnounce_Array.Remove(Message);
		
		//doing this here is NOT appropriate as said above: we MAY NEED to use the "UserWidget_ElimAnnounce" member instead, if the Message can't survice after Timer start :) , however this lesson prove that it does survive if it is a parameter of the next function call 
			//Message->ConditionalBeginDestroy(); // OR ->BeginDestroy()
			//Message = nullptr; //only need when it is a member, local member/param will auto-destroy anyway
	}
}

void ABlasterHUD::SetupBlasterHUD()
{
	//APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	ABlasterPlayerController* PlayerController = Cast<ABlasterPlayerController>(GetOwningPlayerController());
	//factorize them into one function if you use Stephen WAY1
	////WAY1 remove this, WAY2 keep this
	if (CharacterOverlay_Class && PlayerController)
	{
		CharacterOverlay_UserWidget = CreateWidget<UCharacterOverlay_UserWidget>(PlayerController, CharacterOverlay_Class);
	}

	//if(CharacterOverlay_UserWidget) CharacterOverlay_UserWidget->AddToViewport();

	if (PlayerController && Announcement_Class)
	{
		UserWidget_Announcement = CreateWidget<UUserWidget_Announcement>(PlayerController, Announcement_Class);
	}
		//why did I do this so early? well in fact the default text is empty I guess
	if (UserWidget_Announcement) UserWidget_Announcement->AddToViewport();

	//I should create the widget object as soon as possible, but I'm not gonna AddToViewport() here (unless I set visibility to HIDDEN), i PLAN to press Q and call UserWidget_ReturnToMainMenu::MenuSetup() that in turn call 'AddToViewport()', yeah!
	if (PlayerController && ReturnToMainMenu_Class)
	{
		UserWidget_ReturnToMainMenu = CreateWidget<UUserWidget_ReturnToMainMenu>(PlayerController, ReturnToMainMenu_Class);
	}

//MUST move for purpose
	//if (PlayerController && ElimAnnounce_Class)
	//{
	//	UserWidget_ElimAnnounce = CreateWidget<UUserWidget_ElimAnnounce>(PlayerController, ElimAnnounce_Class);
	//}

	////Sooner or later somebody will kill somebody else soon, so why not set default text to empty and add it immediately and we dont have to worry about this step any more, yeah!
	//if (UserWidget_ElimAnnounce) UserWidget_ElimAnnounce->AddToViewport();
}

void ABlasterHUD::PollInit_HUD()
{
	if (UserWidget_Announcement == nullptr)
	{
		SetupBlasterHUD();
	}
}
void ABlasterHUD::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//PollInit_HUD();

}
void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();
	
	//we let this outside because this is shared code, hence improve perfomance
	FVector2D ViewportSize;
	if(GEngine) GEngine->GameViewport->GetViewportSize(ViewportSize);

	float Expand = HUDPackage.ExpandFactor * MaxExpand;

	FVector2D ExpandOffset{};
	DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportSize, ExpandOffset);

	ExpandOffset = { -Expand, 0 };
	DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportSize, ExpandOffset);

	ExpandOffset = { +Expand, 0 };
	DrawCrosshair(HUDPackage.CrosshairsRight, ViewportSize, ExpandOffset);

	ExpandOffset = { 0 , -Expand };
	DrawCrosshair(HUDPackage.CrosshairsTop, ViewportSize, ExpandOffset);

	ExpandOffset = { 0 , +Expand };
	DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportSize, ExpandOffset);
}

void ABlasterHUD::DrawCrosshair(UTexture2D* InTexture, FVector2D ViewportSize , FVector2D ExpandOffset)
{
	if (InTexture == nullptr) return;

	FVector2D CenterLocation{ ViewportSize.X / 2.f, ViewportSize.Y / 2.f };

	FVector2D DrawLocation = CenterLocation - FVector2D{ InTexture->GetSizeX() / 2.f , InTexture->GetSizeY() / 2.f };

	DrawTexture(
		InTexture,       
		DrawLocation.X + ExpandOffset.X,   
		DrawLocation.Y + ExpandOffset.Y,
		InTexture->GetSizeX(),   
		InTexture->GetSizeY(),
		0.f, 0.f, 1.f, 1.f,
		HUDPackage.Color
	);
}




/* U,V isn't what you thought it works LOL
void ABlasterHUD::DrawCrosshair(UTexture2D* InTexture, FVector2D ViewportSize)
{
	if (InTexture == nullptr) return;

	DrawTexture(
		InTexture,        //Texture2D to draw 
		ViewportSize.X / 2.f,   //Location of local top-left corner of texture in conventional viewport coordinates (you know)
		ViewportSize.Y / 2.f,
		InTexture->GetSizeX(),   //TextureSize, Not ViewportSize! because it alwas know ViewportSize LOL
		InTexture->GetSizeY(),
		0.5f,              //TextureU
		0.5f,              //TextureV
		1.f,              //TextureUWidth
		1.f				  //TextureUHeight
	);
}
*/
