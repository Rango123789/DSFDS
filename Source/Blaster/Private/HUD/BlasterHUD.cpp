// Fill out your copyright notice in the Description page of Project Settings.
#include "HUD/BlasterHUD.h"

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();
	
	//we let this outside because this is shared code, hence improve perfomance
	FVector2D ViewportSize;
	if(GEngine) GEngine->GameViewport->GetViewportSize(ViewportSize);

	DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportSize);
	DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportSize);
	DrawCrosshair(HUDPackage.CrosshairsRight, ViewportSize);
	DrawCrosshair(HUDPackage.CrosshairsTop, ViewportSize);
	DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportSize);
}

void ABlasterHUD::DrawCrosshair(UTexture2D* InTexture, FVector2D ViewportSize)
{
	if (InTexture == nullptr) return;

	FVector2D CenterLocation{ ViewportSize.X / 2.f, ViewportSize.Y / 2.f };

	FVector2D DrawLocation = CenterLocation - FVector2D{ InTexture->GetSizeX() / 2.f , InTexture->GetSizeY() / 2.f };

	//shocking news: if you use 0.5, 0.5 , 1, 1 = you dont even need to calculate DrawLocation :D :D
	//next time dont need to do it any more :D :D
	DrawTexture(
		InTexture,        //Texture2D to draw 
		DrawLocation.X,   //Location of local top-left corner of texture in conventional viewport coordinates (you know)
		DrawLocation.Y,
		InTexture->GetSizeX(),   //TextureSize, Not ViewportSize! because it alwas know ViewportSize LOL
		InTexture->GetSizeY(),
		0.f,              //TextureU
		0.f,              //TextureV
		1.f,              //TextureUWidth
		1.f				  //TextureUHeight
	);
}
