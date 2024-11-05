// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Blaster_AnimInstance.h"
#include "Characters/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "CharacterComponents/CombatComponent.h"
#include "Weapons/Weapon.h"
#include <Kismet/KismetSystemLibrary.h>

void UBlaster_AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation(); //it is in fact empty LOL

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());

	//if (BlasterCharacter) CharacterMovement = BlasterCharacter->GetCharacterMovement(); //fail
	/*UKismetSystemLibrary::BoxTraceSingle(ETraceTypeQuery::);*/

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
	bIsEliminated = BlasterCharacter->GetIsEliminated();

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
	if (EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh() && bEquippedWeapon)
	{

		//you must name the socket in weapon exactly like this "LeftHandSocket_InWeapon": 		
		const FTransform RightHandSocket_Transform_InWeapon_WorldSpace
			= EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket_InWeapon"));

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

		if (BlasterCharacter->IsLocallyControlled() && BlasterCharacter->GetCombatComponent()) {  //Stephen does this as he can't get valid HitPoint for other non-controlling players
			//DRAW [muzzle -> local forward ], because the socket direcition align with hosting actor, so you can even ROUGHLY use EquippedWeapon->GetLocation() and EquippedWeapon->GetLocation() + GetForwardVEctor() * 10000
			const FTransform MuzzleFlashTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"));
			FVector ForwardVector = UKismetMathLibrary::GetForwardVector(MuzzleFlashTransform.GetRotation().Rotator());
			FVector Start = MuzzleFlashTransform.GetLocation();
			FVector End = Start + ForwardVector * 80000.f;

			DrawDebugLine(GetWorld(), Start, End, FColor::Red);

			//DRAW [muzzle -> impactpoint/LIKE] , can re-use 'Start' above:
			FHitResult HitResult;
			FVector  EndTrace = BlasterCharacter->GetCombatComponent()->DoLineTrace_UnderCrosshairs(HitResult);

			DrawDebugLine(GetWorld(), Start, HitResult.ImpactPoint, FColor::Orange);

			//READ what we need for ABP:
				//SHOCKING news: the GetSocketTransform can use to get either socket or bone transform by entering their names!
			const FTransform RightHandBone_Transform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("hand_r"));
			//you can also use: (HitResult.ImpactPoint - RightHandBone_Transform.GetLocation() ).Rotation() 
			WantedRotation_ForRightHandBone = UKismetMathLibrary::FindLookAtRotation(
				RightHandBone_Transform.GetLocation(),
				RightHandBone_Transform.GetLocation() + RightHandBone_Transform.GetLocation() - EndTrace // instead of 'HitResult.ImpactPoint'
			);
			bIsLocallyControlled = true; //if enter this, then it must be true and this 'fact' will never be changed nor it need to change it back to false!

			//I have to assign it value here as I did do DoLineTrace here instead of CombatComponent LOL
			//Next time I will do it in CombatComponent instead I think LOL
			if (HitResult.GetActor() && HitResult.GetActor()->Implements<UInteractWithCrossHairsInterface>()) //also 'Cast<T>(UObject/Actor)'
			{
				BlasterCharacter->GetCombatComponent()->SetCrosshairsColor(FLinearColor::Red);
			}
			else
			{
				BlasterCharacter->GetCombatComponent()->SetCrosshairsColor(FLinearColor::White);
			}
		}
	}
	bShouldRotateRootBone = BlasterCharacter->GetShouldRotateRootBone();

	//bIsEliminated = BlasterCharacter->GetIsEliminated();

	CharacterState = BlasterCharacter->GetCharacterState();
}




		////For my own perpose
		////FRotator RootRotation = BlasterCharacter->GetMesh()->GetBoneQuaternion(FName("root")).Rotator();
		//FRotator RootRotation = BlasterCharacter->GetMesh()->GetSocketTransform(FName("root")).GetRotation().Rotator();

		//if (bReset == false)
		//{
		//	bReset = true;
		//	WantedRootRotation = {RootRotation.Pitch, RootRotation.Yaw - AO_Yaw_LastFrame, RootRotation.Roll};
		//}
		//else
		//{
		//	bReset = false;
		//	WantedRootRotation = { RootRotation.Pitch, RootRotation.Yaw + AO_Yaw_LastFrame, RootRotation.Roll };
		//}
		//AO_Yaw_LastFrame = AO_Yaw;










