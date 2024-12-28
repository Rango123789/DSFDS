// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Blaster/Team.h"
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
	UPROPERTY()
	class ABlasterCharacter* BlasterCharacter;
	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController;

	UPROPERTY(ReplicatedUsing = OnRep_Defeat);
	int Defeat{};

	/*
	* Team
	*/
	UPROPERTY(ReplicatedUsing = OnRep_Team)
	ETeam Team = ETeam::ET_NoTeam;

	//Stephen use this to solve client part that that can't change color when GameStart (meaning they can't change PS::Team forever, that is the only time we sort the GS::PlayerArray into 2 teams)
	UFUNCTION()
	void OnRep_Team();

public:
	int GetDefeat() { return  Defeat; }
	void SetTeam(ETeam InTeam) { Team = InTeam; }
	ETeam GetTeam() { return Team; }
};
