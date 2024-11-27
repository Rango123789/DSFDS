// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterComponents/BuffComponent.h"
#include "Characters/BlasterCharacter.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	//this time i test doing it here to see if it will actually change in the HOSTING actor: 
	// IT WORK TOO! but this is not recommeded, let's do it in HOSTING actor
	SetIsReplicated(true);
}

//void UBuffComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//	GetLifetimeReplicatedProps(OutLifetimeProps);
//}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

void UBuffComponent::PickHealth(float InHealthAmount, float InHealingTime)
{
	//PlanA: increase Health (and CheckAndUpdateHUD_Health): NOT USED

	//planB: make it heal Char over time:
	bIsHealing = true; //not it pass bOuter in Tick and reach RampUpHealth()

	HealingRate = InHealthAmount / InHealingTime; //this will be constant until pick a new Health Pickup

	//Whenever you pick a new Health pickup, this will increase:
	AmountToHeal += InHealthAmount; //+= is in case you pick a lot of Health pickups LOL!
}

void UBuffComponent::RampUpHealth(float DeltaTime)
{
	//DOOR1: bIsHealing, because of Door3 it becomes optional!
	if (Character == nullptr || bIsHealing == false) return;

	//DOOR2: you shoudn't increase health for a dying char LOL:
	if (Character->GetIsEliminated()) return; //I forget this line

	//DOOR3: Optional, stop healing when Health reach max during process:
	if (AmountToHeal <= 0 || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bIsHealing = false;
		AmountToHeal = 0.f; //this is recommended I think
		return;
	}

	float HealThisFrame = DeltaTime * HealingRate;

	//This trigger OnRep_Health() per Net Update lol (may be OKAY), But I think we should use "one-second" technique to reduce the Net replication LOL: you must create "a shadow member" in this BuffComponent for this purpose - WORK TO DO1 (anyway stephen didn't bother to do it)
	// And consequencely 'PlayReactMontage()' - need to fix this side effect: 
	// simply add 'if ( Health < Health_LastFrame) PlayReactMontage()' will fix it!
	Character->SetHealth( 
		FMath::ClampAngle(Character->GetHealth() + HealThisFrame,
		0, Character->GetMaxHealth())
	); 

	AmountToHeal -= HealThisFrame;

	//you can use "one-sec" technique to avoid it update HUD per frame! - WORK TO DO1 (anyway stephen didn't bother to do it)
	//the other idea is to reduce the HealingTime, but it will increase the level of incurracy to heal in last frame LOL, so be careful
	Character->CheckAndUpdateHUD_Health();
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsHealing)
	{
		RampUpHealth(DeltaTime);
	}
}
