// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PlayerState_Blaster.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API APlayerState_Blaster : public APlayerState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnRep_Score() override;
	UFUNCTION()
	void OnRep_Defeat();
	
	void UpdateHUD_Score();
	void UpdateHUD_Defeat();

protected:
	virtual void BeginPlay() override;

	class ABlasterCharacter* BlasterCharacter;
	class ABlasterPlayerController* BlasterPlayerController;

	UPROPERTY(ReplicatedUsing = OnRep_Defeat);
	int Defeat{};
public:
	int GetDefeat() { return  Defeat; }
};
