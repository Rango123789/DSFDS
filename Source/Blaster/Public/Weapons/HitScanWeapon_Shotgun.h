// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/HitScanWeapon.h"
#include "HitScanWeapon_Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHitScanWeapon_Shotgun : public AHitScanWeapon
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override;

protected:
	UPROPERTY(EditAnywhere)
	int32 NumOfPellets = 10;

};