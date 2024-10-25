// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Projectile.h"
#include "AProjectileBullet.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AAProjectileBullet : public AProjectile
{
	GENERATED_BODY()
public:
	virtual void OnBoxHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

};
