// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Blaster_AnimInstance.h"
#include "Characters/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "CharacterComponents/CombatComponent.h"
#include "Weapons/Weapon.h"

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

	EquippedWeapon = BlasterCharacter->GetEquippedWeapon(); //NEW

	bIsCrouched = BlasterCharacter->bIsCrouched;

	bIsAiming = BlasterCharacter->IsAming();
	bIsFiring = BlasterCharacter->IsAFiring();

//Offset Yaw for strafing:
	FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());

    FRotator Delta1 = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation , AimRotation );
	    //this store the whole FRotator, this fix! = in fact it has .GetNormalized() inside
	DeltaRotation = FMath::RInterpTo(DeltaRotation, Delta1, DeltaSeconds, 5.f);
	YawOffset = DeltaRotation.Yaw;
//Leaning Angle for leaning:
//Leaning Angle for leaning:

	CharacterRotationLastFrame = CharacterRotation; 
	CharacterRotation = BlasterCharacter->GetActorRotation();

	const FRotator Delta2 = (CharacterRotation - CharacterRotationLastFrame).GetNormalized();
	const float Target = Delta2.Yaw / DeltaSeconds; 
	Lean = FMath::FInterpTo(Lean, Target, DeltaSeconds, 5.f); 

	//Lean = FMath::Clamp(Lean, -180.f, 180.f); //why not -180, to 180?
	Lean = FMath::Clamp(Lean, -90.f, 90.f); //why not -180, to 180?

	AO_Yaw = BlasterCharacter->GetAO_Yaw();
	AO_Pitch = BlasterCharacter->GetAO_Pitch();

	TurningInPlace = BlasterCharacter->GetTurningInPlace();

	//do the if so that you dont have to check it inside, and also return soon for performance
	//the last one ('bEuippedWeapon') could be redudant, better double-kill than left over? :D :D
	if(EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh()  && bEquippedWeapon ) 
	{

		//you must name the socket in weapon exactly like this "LeftHandSocket_InWeapon": 		
		const FTransform RightHandSocket_Transform_InWeapon_WorldSpace 
			= EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket_InWeapon"), ERelativeTransformSpace::RTS_World); //this socket is in WeaponMesh of the Weapon, not not the Character Mesh

		FVector OutLocation; //Location to be relative to "Right_Hand bone"
		FRotator OutRotation; //Rotation to be relative to "Right_Hand bone"

		//you must check Character's skeleton for the actual name of 'Right_Hand bone':
		BlasterCharacter->GetMesh()->TransformToBoneSpace(
			FName("hand_r"),
			RightHandSocket_Transform_InWeapon_WorldSpace.GetLocation(),
			RightHandSocket_Transform_InWeapon_WorldSpace.GetRotation().Rotator(),
			OutLocation,
			OutRotation
		);

		LefttHandSocket_Transform_InWeapon.SetLocation(OutLocation);
		LefttHandSocket_Transform_InWeapon.SetRotation(FQuat(OutRotation));

	}
}

//USkeletalMeshComponent : public USkinnedMeshComponent 
// USkinnedMeshComponent::GetSocketTransform()

	//UE_LOG(LogTemp, Warning, TEXT("YawOffset: %f"), YawOffset);
	//    //this ONLY store Yaw, this look Equivalent, but it doesn't fix LOL
	//DeltaYaw = FMath::FInterpTo(DeltaYaw, Delta1.Yaw, DeltaSeconds, 5.f);
	//YawOffset = DeltaYaw;
	//       //Add this line will make it work by this way, not test yet but trust me!
	//if (YawOffset > 180) YawOffset = YawOffset - 360;
	//if (YawOffset < -180) YawOffset = YawOffset + 360;

	//    //at first:
	//YawOffset = Delta1.Yaw; //this is the first problem

	//    //my try
	//YawOffset = FMath::FInterpTo(YawOffset, Delta1.Yaw, DeltaSeconds, 5.f); 
	//		//Add this line will make it work by this way, not test yet but trust me!
	//if (YawOffset > 180) YawOffset = YawOffset - 360;
	//if (YawOffset < -180) YawOffset = YawOffset + 360;














