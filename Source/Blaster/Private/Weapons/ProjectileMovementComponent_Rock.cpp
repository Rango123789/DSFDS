// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ProjectileMovementComponent_Rock.h"

void UProjectileMovementComponent_Rock::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	//do nothing so that Super:: dont have chance to call and alter Actor's velocity (i.e Movement)
}
