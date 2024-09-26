// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Blaster_AnimInstance.h"
#include "Characters/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <Kismet/KismetMathLibrary.h>
//#include "Math/MathFwd.h"

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

//Offset Yaw for strafing:
	FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());

    FRotator RotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation , AimRotation ) ;
						  //= (MovementRotation - MovementRotation).GetNormalized();  can't use .Normalize() in the same line because it doesn't return FRotator
	   //if APawn::GetBaseAimRotation() && GetVelocity() are built-in replicated, then YawOffset will be replicated too as UNIVERSAL rule(as currently it is put in Tick-LIKE, and each Anim for each character will update every frame!)
	YawOffset = RotationDelta.Yaw;

//Leaning Angle for leaning:
	//the idea is when we rotate mouse -> Camera/Actor's Rotation change, we use this to decide the leaning angle:
	// - if Rotation Yaw = 90* , the BS_2D will rotate all the way to Lean_R animation (leaning 10 or 20 degree)
	//  - if Rotation Yaw = -90* , the BS_2D will rotate all the way to Lean_L animation (leaning 10 or 20 degree)

	CharacterRotationLastFrame = CharacterRotation; //OPTIONAL: set 'CharacterRotation' for first time in NativeInitializeAnimation if you want :D , if not the first Delta will goes wrong I guess, but you wont even notice it I think :D 
	CharacterRotation = BlasterCharacter->GetActorRotation();

	const FRotator Delta = (CharacterRotation - CharacterRotationLastFrame).GetNormalized();
	const float Target = Delta.Yaw / DeltaSeconds; // <=> how many Yaw degree it will rotate in a second, like Rotating Speed in physics
	Lean = FMath::FInterpTo(Lean, Target, DeltaSeconds, 5.f); //Stephen add 'const float Interpo = , but no need! 

	//if you Clamp it within [-90, 90] the Backward_Lean_R/L will never get effective? = incorrect!
	//it will reduce the "MAX leaning possible" from "10/20 degree" to "5/10 degree"? = verified!
	//So in case you rotate your mouse +/-180 GLOBALLY you will face in opposite direction, and you will lean 50% the Max LOCALLY
	
	//Lean = FMath::Clamp(Lean, -90.f, 90.f); //why not -180, to 180?
	Lean = FMath::Clamp(Lean, -90.f, 90.f); //why not -180, to 180?

	UE_LOG(LogTemp, Warning, TEXT("YawOffset: %d"), YawOffset);
}

















