// Fill out your copyright notice in the Description page of Project Settings.
#include "HUD/BlasterHUD.h"
#include "HUD/CharacterOverlay_UserWidget.h"
#include "HUD/UW_Announcement.h"
#include "PlayerController/BlasterPlayerController.h"
#include "HUD/UserWidget_ReturnToMainMenu.h"

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
