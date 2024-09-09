// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Overhead_UserWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UOverhead_UserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	class UTextBlock* DisplayText;
};
 