// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ProjectileMovementComponent_Rock.h"

UProjectileMovementComponent::EHandleBlockingHitResult UProjectileMovementComponent_Rock::HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	//this will return WHATEVER, but it can't escape the hosting function as universal rule, so dont worry :D :D 
	//We thought this function wont change any outcome, but in fact it has this line 'SubTickTimeRemaining = TimeTick * (1.f - Hit.Time);'
	//that will change the 'SubTickTimeRemaining' value bound to external reference (you see it is T& __)
	//hence calling super is in fact possibly important!
	Super::HandleBlockingHit( Hit, TimeTick, MoveDelta, SubTickTimeRemaining);

	//this line always be execited, and this is the FINAL return after escape the hosting function:
	return EHandleBlockingHitResult::AdvanceNextSubstep;
}

void UProjectileMovementComponent_Rock::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	//do nothing so that Super:: dont have chance to call and alter Actor's velocity (i.e Movement)
}
