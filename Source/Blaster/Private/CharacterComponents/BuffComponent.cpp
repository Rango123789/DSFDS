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
	AmountToHeal = InHealthAmount; //+= is more correct in case you pick a lot of Health pickups LOL
	HealingTime = InHealingTime;
	RemainingAmount += AmountToHeal; //+= is in case you pick a lot of Health pickups LOL!
}

void UBuffComponent::RampUpHealth(float DeltaTime)
{
	if (Character == nullptr) return;

	if (Character->GetIsEliminated()) return; //I forget this line

	if (RemainingAmount <= 0)
	{
		bIsHealing = false;
		RemainingAmount = 0.f; //clamp it back to Zero in worst case LOL
		return;
	}

	//This line must go before the next line, keeping both AmountToHealth and HealingTime const is the way make it work as expected LOL:
	float AmountHeal_thisframe = DeltaTime * AmountToHeal / HealingTime;

	//this will cause imperfection and slow speed at the end:
	//AmountToHeal -= DeltaTime * AmountToHeal / HealingTime;
	//this is the correct way to go, meaning create yet another extra var LOL:
	RemainingAmount -= DeltaTime * AmountToHeal / HealingTime;

	//INCORRECT approachA: When it move to this line, AmmountToHeal has been changed lol, so if you use this, you will lose an amount of heal exactly 'HealingTime' when the process finish LOL:
	// NOW become correct when AmountToHeal = const, due to creating 'RemainingAmount'
	//Character->AddHealth(DeltaTime * AmountToHeal / HealingTime);

	//CORRECT approachA:
	Character->AddHealth(AmountHeal_thisframe);

	//you can use "one-sec" technique to avoid it update HUD per frame!
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
