// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/Pickup.h"
#include "Pickup_Speed.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API APickup_Speed : public APickup
{
	GENERATED_BODY()
public:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
protected:
	UPROPERTY(EditAnywhere)
	float WalkSpeed = 1400.f;
	UPROPERTY(EditAnywhere)
	float CrouchSpeed = 800.f;
	UPROPERTY(EditAnywhere)
	float SpeedingTime = 6.f;

public:

};
