// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_Announcement.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UUserWidget_Announcement : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetAnnounceText(const FString& InString);
	void SetWarmUpTimeText(const FString& InString);
	void SetInfoText(const FString& InString);
protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextBlock_Announcement;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextBlock_WarmUpTime;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextBlock_Info;

public:

};
