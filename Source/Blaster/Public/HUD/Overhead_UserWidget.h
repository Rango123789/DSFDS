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
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* LocalRole_TextBlock;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* RemoteRole_TextBlock;

	//Stephen use APawn, but the function is from AActor!
	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(AActor* InActor);

	UFUNCTION(BlueprintCallable)
	void SetTextContent(FString TextToDisplay , UTextBlock* TextBlock);

	////I dont think this is needed LOL
	//virtual void NativeDestruct() override;

};
 