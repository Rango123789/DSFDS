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

void APickupSpawnPoint::OnDestroyed_Callback(AActor* DestroyedActor)
{
	StartTimer_SpawnPickup();
}

void APickupSpawnPoint::SpawnPickup()
{
	if (GetWorld() && PickupClasses.Num() > 0)
	{
		uint32 RandomNum = FMath::RandRange(0, PickupClasses.Num() - 1);

		FActorSpawnParameters Params;
		Params.Owner = this; //this make option1 possible!

		//Pickup =  //only option2 need it
		GetWorld()->SpawnActor<APickup>(
			PickupClasses[RandomNum],
			GetActorTransform(),
			Params
		);

		//this is 'bind a function of this class to a delegate of external class':
		//the impefection: if the Pickup overlap with Char immediately right after spawned, this line may not even succeed
		//may be because Pickup turn INVALID/null even before it reach this line (it can't pass the check - VERIFIED, I test it) 
		//Solution: we can delay the possible APick::OnSphereOverlap by set a Timer in Pickup::BeginPlay:: OnComponenentOverlap.AddDynamic
		//if (Pickup)
		//{
		//	Pickup->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::OnDestroyed_Callback);
		//	UE_LOG(LogTemp, Warning, TEXT("Pass pickup check"))
		//}
	}
}
