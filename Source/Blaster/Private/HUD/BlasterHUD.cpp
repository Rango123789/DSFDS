// Fill out your copyright notice in the Description page of Project Settings.
#include "HUD/BlasterHUD.h"

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
