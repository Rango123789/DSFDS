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

	//This function follow the required signature of AActor::OnDestroyed delegate, we in fact dont need to use the param, but we must follow the signature anyway, so this callback will simply wrap around the KEY function: StartTimer_SpawnPickup() above:
	UFUNCTION()
	void OnDestroyed_Callback(AActor* DestroyedActor);

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

	//Option2: we bind callback/StartTimer_SpawnPickup of APickupSpawnPoint to APickup::OnDestroyed delegate, hence need reference to the spawned pickup: 
	//UPROPERTY()
	//class APickup* Pickup;


public:	
	virtual void Tick(float DeltaTime) override;

};
