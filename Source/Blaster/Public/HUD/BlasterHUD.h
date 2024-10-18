// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"


USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

	UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;
	float ExpandFactor;
	FLinearColor Color;
};


UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()
public:
	void virtual DrawHUD() override;
	void DrawCrosshair(UTexture2D* InTexture, FVector2D ViewportSize, FVector2D ExpandOffset);
protected:

private:
	FHUDPackage HUDPackage; //this will be assigned AWeapon::values via Character::Combat (as this is where you can access both EquippedWeapon and PlayerController()->GetHUD();

	UPROPERTY(EditAnywhere)
	float MaxExpand = 10.f;

public:
	void SetHUDPackage(const FHUDPackage& InHUDPackage){ HUDPackage = InHUDPackage; }

};
