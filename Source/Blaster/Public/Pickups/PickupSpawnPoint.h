// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

UCLASS()
class BLASTER_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APickupSpawnPoint();

	//this would need to be called from APickup in option1:
	void StartTimer_SpawnPickup();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void SpawnPickup();

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class APickup>> PickupClasses;

	UPROPERTY(EditAnywhere)
	float MinDelayTime = 2.f;
	UPROPERTY(EditAnywhere)
	float MaxDelayTime = 6.f;

	FTimerHandle TimerHandle;

public:	
	virtual void Tick(float DeltaTime) override;

};
