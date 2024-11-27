// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();

	//virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void PickHealth(float InHealthAmount, float InHealingTime);

	void RampUpHealth(float DeltaTime);

protected:
/**FUNCTIONs**/
	virtual void BeginPlay() override;

/**DATA members**/
	class ABlasterCharacter* Character; //CombatComponent also have this

	friend class ABlasterCharacter;

	//for ramp up health over healing time in Tick:
	bool bIsHealing = false;
	float AmountToHeal = 0.f;
	float RemainingAmount = 0.f;

	float HealingTime = 0.f;


public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
