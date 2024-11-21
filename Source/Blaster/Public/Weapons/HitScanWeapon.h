// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override;
protected:
	//We create it here becase both SMG and Shotgun need it:
	FVector RandomEndWithScatter(const FVector& Start, const FVector& HitTarget);
	UPROPERTY(EditAnywhere)
	float DistanceToSphere = 800.f;
	UPROPERTY(EditAnywhere)
	float SphereRadius = 75.f;
	UPROPERTY(EditAnywhere)
	bool bUseScatter = false;

	//Damage is now with Weapon itself, no projectile can help it any more LOL
	UPROPERTY(EditAnywhere)
	float Damage = 15.f;

	//HitSound and HitParticle now with Weapon as well:
	UPROPERTY(EditAnywhere)
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere)
	UParticleSystem* HitParticle;

	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticle; //for when moving
	//to store temp object by spawning BeamParticle for a reason rather than removing it (in fact may not even need to remove it)
	//we can in fact create a local variable of this type instead:
	//UPROPERTY()
	//UParticleSystemComponent* BeamParticleComponent; 

	//Which every BP_child need it, pick it, otherwise leave it as "none":
	UPROPERTY(EditAnywhere)
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere)
	UParticleSystem* FireParticle;

public:

};
