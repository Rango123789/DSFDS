// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UserWidget_ElimAnnounce.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UUserWidget_ElimAnnounce : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetTextBlock_Elim(const FString& AttackPlayer, const FString& ElimmedPlayer);
	//not sure why stephen did this, we dont even need to touch it? well because want to move this parent box instead of the text itself, anyway HBox are absolutely OPTIONAL from beginning!
	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* HorizontalBox_Elim;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextBlock_Elim;
protected:
};
