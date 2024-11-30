// Fill out your copyright notice in the Description page of Project Settings.

#include "Pickups/PickupSpawnPoint.h"
#include "Pickups/Pickup.h"

APickupSpawnPoint::APickupSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;

}

void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	
	//Spawn for FIRST time:
	StartTimer_SpawnPickup();
}

void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickupSpawnPoint::StartTimer_SpawnPickup()
{
	float DelayTime = FMath::RandRange(MinDelayTime, MaxDelayTime);

	GetWorldTimerManager().SetTimer(TimerHandle, this, &ThisClass::SpawnPickup, DelayTime);
}

void APickupSpawnPoint::SpawnPickup()
{
	
	if (GetWorld() && PickupClasses.Num() > 0)
	{
		uint32 RandomNum = FMath::RandRange(0, PickupClasses.Num() - 1);

		FActorSpawnParameters Params;
		Params.Owner = this; //this make option1 possible!

		GetWorld()->SpawnActor<APickup>(
			PickupClasses[RandomNum],
			GetActorTransform(),
			Params
		);
	}
}
