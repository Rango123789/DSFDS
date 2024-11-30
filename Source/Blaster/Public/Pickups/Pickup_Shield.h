// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/Pickup.h"
#include "Pickup_Shield.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API APickup_Shield : public APickup
{
	GENERATED_BODY()
	
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	UPROPERTY(EditAnywhere)
	float ShieldAmount = 50.f;
};
