// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Blaster_AnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UBlaster_AnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
//I only need to go to AnimInstance.h, search 'Native' and copy them all LOL:
    // Native initialization override point
	virtual void NativeInitializeAnimation() override;
	// Native update override point. It is usually a good idea to simply gather data in this step and 
	// for the bulk of the work to be done in NativeThreadSafeUpdateAnimation.
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

//JUST OR FUN, I dont use it now:
	// Native thread safe update override point. Executed on a worker thread just prior to graph update 
	// for linked anim instances, only called when the hosting node(s) are relevant
	//virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class ABlasterCharacter* BlasterCharacter;

	//class UCharaceterMovementComponent* CharacterMovement; //fail for technical reasons: TObjectPtr, inline, whatever

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float GroundSpeed{};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsInAir{false};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsAccelerating; //THIS IS NEW! - SEE explain below

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bEquippedWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsCrouched;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsAiming;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float YawOffset{};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Lean{};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FRotator CharacterRotationLastFrame;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FRotator CharacterRotation;

	FRotator DeltaRotation; //used here, but no use in ABP
	float DeltaYaw; //no need any more

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float AO_Yaw; //DeltaYawSinceStopMovingJumping
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float AO_Pitch; //the Pitch of ControlRotation<-Camera
};

/*Now you may be wondering why we're using this (bIsAccelaring) rather than the speed.
And when we talk about accelerating from the anim instance point of view, we're not talking about the rate of change of velocity.
If you know physics, you know that that's the technical definition of acceleration.
What I'm talking about is whether or not we're adding input we're pressing the keys to move. */
