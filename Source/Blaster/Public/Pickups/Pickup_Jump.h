// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/Pickup.h"
#include "Pickup_Jump.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API APickup_Jump : public APickup
{
	GENERATED_BODY()

public:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
protected:
	UPROPERTY(EditAnywhere)
	float JumpVelocity =2500.f; //= 2.5m/s
	UPROPERTY(EditAnywhere)
	float JumpingTime = 15.f;


public:

};
