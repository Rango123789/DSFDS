// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/Pickup.h"
#include "Blaster/WeaponTypes.h"
#include "Pickup_Ammo.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API APickup_Ammo : public APickup
{
	GENERATED_BODY()

public:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
protected:
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType = EWeaponType::EWT_AssaultRifle; //what weapon type this Ammo is meant for

	UPROPERTY(EditAnywhere)
	uint32 AmmoAmount = 30; //how much ammo of that type this pickup holds
public:
	uint32 GetAmmoAmmount() { return AmmoAmount; } //no need
	EWeaponType GetWeaponType() { return WeaponType; } //no need
};
