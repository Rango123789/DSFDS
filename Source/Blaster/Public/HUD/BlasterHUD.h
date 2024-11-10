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
	ABlasterHUD(); //for testing
	void virtual DrawHUD() override;
	void DrawCrosshair(UTexture2D* InTexture, FVector2D ViewportSize, FVector2D ExpandOffset);
protected:
	/***functions***/
	virtual void BeginPlay() override;

/***data members****/
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UCharacterOverlay_UserWidget> CharacterOverlay_Class;

	UCharacterOverlay_UserWidget* CharacterOverlay_UserWidget;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget_Announcement> Announcement_Class;

	UUserWidget_Announcement* UserWidget_Announcement;

	
	FHUDPackage HUDPackage; //this will be assigned AWeapon::values via Character::Combat (as this is where you can access both EquippedWeapon and PlayerController()->GetHUD();

	UPROPERTY(EditAnywhere)
	float MaxExpand = 10.f;
private:

public:
	void SetHUDPackage(const FHUDPackage& InHUDPackage){ HUDPackage = InHUDPackage; }

	UCharacterOverlay_UserWidget* GetCharacterOverlay_UserWidget() { return CharacterOverlay_UserWidget; }
	UUserWidget_Announcement* GetUserWidget_Announcement() { return UserWidget_Announcement; }
};
