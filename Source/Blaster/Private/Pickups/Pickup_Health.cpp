   // Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/Pickup_Health.h"
#include <Characters/BlasterCharacter.h>
#include "CharacterComponents/BuffComponent.h"
//#include "NiagaraComponent.h"
//#include <NiagaraFunctionLibrary.h>

APickup_Health::APickup_Health()
{
	//setup the extra Niagara Comp:
	//NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("NiagaraComp");
	//NiagaraComponent->SetupAttachment(RootComponent);

}

void APickup_Health::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* OverlapCharacter = Cast<ABlasterCharacter>(OtherActor);

	if (OverlapCharacter && OverlapCharacter->GetCombatComponent())
	{
		OverlapCharacter->GetBuffComponent()->PickHealth(HealthAmount, HealingTime);

		Destroy(); //this will trigger Child::Destroyed() in all clients and play the sound - not sure we should let non-controlling device hearing it lol?
	}
}

//now you can remove this if you want
void APickup_Health::Destroyed()
{
	Super::Destroyed(); //play Picksound, can select a different one from BP_child to match

	////spawn extra Niagara asset when destroyed():
	//UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, NiagaraSystem_SpawnedWhenDestroyed, GetActorLocation()); //autodestroy
}
