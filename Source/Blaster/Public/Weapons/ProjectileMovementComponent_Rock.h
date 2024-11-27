// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ProjectileMovementComponent_Rock.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UProjectileMovementComponent_Rock : public UProjectileMovementComponent
{
	GENERATED_BODY()

public:
	//no need to override this at all, what stephen does doesn't make any sense:
		virtual EHandleBlockingHitResult HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining) override;
	//this is enough:
	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice = 0.f, const FVector& MoveDelta = FVector::ZeroVector) override;
};
