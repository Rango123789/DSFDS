// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/UW_Announcement.h"
#include "Components/TextBlock.h"

void UUserWidget_Announcement::SetAnnounceText(const FString& InString)
{
	if (TextBlock_Announcement) TextBlock_Announcement->SetText(FText::FromString(InString));
}

void UUserWidget_Announcement::SetWarmUpTimeLeftText(const FString& InString)
{
	if (TextBlock_WarmUpTimeLeft) TextBlock_WarmUpTimeLeft->SetText(FText::FromString(InString));
}

void UUserWidget_Announcement::SetInfoText(const FString& InString)
{
	if (TextBlock_Info) TextBlock_Info->SetText(FText::FromString(InString));
}