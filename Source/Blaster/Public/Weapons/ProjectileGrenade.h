// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Projectile.h"
#include "ProjectileGrenade.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileGrenade : public AProjectile
{
	GENERATED_BODY()
public:
	AProjectileGrenade();
protected:
	void virtual BeginPlay() override;
	
};
