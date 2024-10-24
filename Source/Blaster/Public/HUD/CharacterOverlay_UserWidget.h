// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay_UserWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UCharacterOverlay_UserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetHealthPercent(float InPercent);
	void SetHealthText(const FString& InString);
protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextBlock_Health;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ProgressBar_Health;

private:

};
