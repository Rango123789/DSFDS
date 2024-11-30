// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/Pickup_Speed.h"
#include <Characters/BlasterCharacter.h>
#include "CharacterComponents/BuffComponent.h"

void APickup_Speed::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* OverlapCharacter = Cast<ABlasterCharacter>(OtherActor);

	if (OverlapCharacter && OverlapCharacter->GetCombatComponent())
	{
		OverlapCharacter->GetBuffComponent()->PickSpeed(WalkSpeed, CrouchSpeed, SpeedingTime);

		Destroy(); //this will trigger Child::Destroyed() in all clients and play the sound - not sure we should let non-controlling device hearing it lol?
	}
}
