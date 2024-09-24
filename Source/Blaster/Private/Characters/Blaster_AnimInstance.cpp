// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Blaster_AnimInstance.h"
#include "Characters/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"


void UBlaster_AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation(); //it is in fact empty LOL

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());

	//if (BlasterCharacter) CharacterMovement = BlasterCharacter->GetCharacterMovement(); //fail
}

void UBlaster_AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds); //it is in fact empty LOL
	
	if(BlasterCharacter == nullptr) BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	if (BlasterCharacter == nullptr) return;

	bIsInAir = 	BlasterCharacter->GetCharacterMovement()->IsFalling();

	GroundSpeed= BlasterCharacter->GetVelocity().Size2D(); //rather than: BlasterCharacter->GetCharacterMovement()->GetVelocity()
	                                 // can also do: BlasterCharacter->GetCharacterMovement()->Velocity;

	FVector CurrentInputAcceleration = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration();
	//Stephen, but what if it is < 0? 
	bIsAccelerating = CurrentInputAcceleration.Size() > 0.f ? true : false;
	////me:
	//bIsAccelerating = CurrentInputAcceleration.Size() == 0.f ? false : true; //the 0.f is important here LOL, '0' is stupid

	bEquippedWeapon = BlasterCharacter->IsWeaponEquipped();

	bIsCrouched = BlasterCharacter->bIsCrouched;

	bIsAiming = BlasterCharacter->IsAming();
}
