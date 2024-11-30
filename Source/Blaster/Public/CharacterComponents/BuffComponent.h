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

	void PickShield(float InShieldAmount);

	void PickSpeed(float InWalkSpeed, float InCrouchSpeed, float InSpeedingTime);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetMovementSpeed(float InWalkSpeed, float InCrouchSpeed);

	FTimerHandle TimerHandle_Speed;
	UFUNCTION()
	void TimerCallback_Speed();

	void PickJump(float InJumpVelocity, float InJumpingTime);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetJumpVelocity(float InJumpVelocity);

	FTimerHandle TimerHandle_Jump;
	UFUNCTION()
	void TimerCallback_Jump();

protected:
/**FUNCTIONs**/
	virtual void BeginPlay() override;

/**DATA members**/
	class ABlasterCharacter* Character; //CombatComponent also have this

	friend class ABlasterCharacter;

	//for ramp up health over healing time in Tick:
	bool bIsHealing = false;
	float AmountToHeal = 0.f;
	float HealingRate = 0.f; // = InHealhAmount / HealingTime

	//for speed up due to Pickup_Speed:
	UPROPERTY(EditAnywhere)
	float MaxWalkSpeed_Backup;
	UPROPERTY(EditAnywhere)
	float MaxWalkSpeedCrouched_Backup;

	//for Jump due to Pickup_Speed:
	UPROPERTY(EditAnywhere)
	float JumpVelocity_Backup;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
