// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/Pickup.h"
#include "Pickup_Health.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API APickup_Health : public APickup
{
	GENERATED_BODY()
public:
	APickup_Health();

	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void Destroyed() override;

protected:
/**FUNCTIONs**/

/**DATA members**/
	//UPROPERTY(VisibleAnywhere)
	//class UNiagaraComponent* NiagaraComponent; //to hold long-term Health System

	//UPROPERTY(EditAnywhere)
	//class UNiagaraSystem* NiagaraSystem_SpawnedWhenDestroyed; //spawned when Destroyed()

	UPROPERTY(EditAnywhere)
	float HealthAmount = 50.f;

	//Stephen plan to heal the char overtime for more fun, hence this var:
	UPROPERTY(EditAnywhere)
	float HealingTime = 3.f;


public:

};
