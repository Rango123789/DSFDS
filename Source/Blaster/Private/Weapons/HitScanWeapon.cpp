// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HitScanWeapon.h"
#include <Kismet/GameplayStatics.h>
#include "Characters/BlasterCharacter.h"
#include "Particles/ParticleSystemComponent.h"
#include "CharacterComponents/LagCompensationComponent.h"
#include "PlayerController/BlasterPlayerController.h"
#include "Blaster/Blaster.h"

//HitTarget will be passed from Combat::DoLineTrace():: center screen -> forwards, everything is already setup, we need only to use it:
//UPDATE: We will pass either 'HitTarget_DoLineTrace1' || f('HitTarget_DoLineTrace1') from outside
// = this will be handled right in TIRE1, meaning we're gonna to re-organize TIRE1
void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	if (WeaponMesh==nullptr || GetWorld()==nullptr) return;

	Super::Fire(HitTarget); //play FireAnimation_inWeapon --> FireSound + FireParticle in it
                            //Spawn ACasing
//STAGE1: Do Line Trace
	FHitResult HitResult;
	FTransform MuzzleFlashSocketTransform = WeaponMesh->GetSocketTransform(FName("MuzzleFlash"));
	FVector Start = MuzzleFlashSocketTransform.GetLocation();
	FVector End = Start + (HitTarget - Start) * 1.25f;

	//GetWorld()->LineTraceSingleByChannel( HitResult, Start, End, ECollisionChannel::ECC_Visibility );
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_SkeletalMesh);

//STAGE2: check if it hit any ABasterCharacter_instance and apply damage on him

	//FVector BeamEnd = End; //if hit something we change it inside the next if
	FVector BeamEnd = HitTarget; //if hit something we change it inside the next if

	if (HitResult.bBlockingHit)
	{
		if(HitResult.GetActor()) BeamEnd = HitResult.ImpactPoint;

		ABlasterCharacter* HitBlasterCharacter = Cast<ABlasterCharacter>(HitResult.GetActor());

		ABlasterCharacter* AttackBlasterCharacter = Cast<ABlasterCharacter>(GetOwner());
	      
		ABlasterPlayerController* AttackController = nullptr;

		if(AttackBlasterCharacter) AttackController = Cast<ABlasterPlayerController>(AttackBlasterCharacter->GetController());
		//if it hit BlasterCharacter, apply damage on him
		if (HitBlasterCharacter && AttackBlasterCharacter && AttackController)
		{
		
		//if not use serverside request, default to old code
			if (!bUseServerSideRewind)
			{
				//Go and check the name of the head bone (also 'head') that the head capsule in PA based on:
				float DamageToApply = HitResult.BoneName == FName("head") ? Damage_HeadShot : Damage;

				if (HasAuthority()){
					UGameplayStatics::ApplyDamage(
						HitBlasterCharacter, //this will trigger HitBlasterCharacter::ReceiveDamage()
						DamageToApply,
						AttackController,    //this will be important at where you receive it for purpose
						this,UDamageType::StaticClass());
				}
			}

			if (bUseServerSideRewind)
			{
				//if using ServerRewind, but the attacking char is controlled by the server, we can simply default back to the old code, no need server rewind:
				if (HasAuthority() && AttackBlasterCharacter->IsLocallyControlled())
				{
					//Go and check the name of the head bone (also 'head') that the head capsule in PA based on:
					float DamageToApply = HitResult.BoneName == FName("head") ? Damage_HeadShot : Damage;

					if (HasAuthority()) {
						UGameplayStatics::ApplyDamage(
							HitBlasterCharacter, //this will trigger HitBlasterCharacter::ReceiveDamage()
							DamageToApply,
							AttackController,    //this will be important at where you receive it for purpose
							this, UDamageType::StaticClass());
					}
				}
				//if using ServerRewind, shooting char is not controlled by the server, we now use ServerSideRewind:  headshot handle in lagcomp
				if (!HasAuthority() && AttackBlasterCharacter->IsLocallyControlled())
				{
					//GetServerTime_Synched() call from anywhere are the same anywhere
					//but AttackController->RTT / 2 should be called from LocallyControlled device, not in the server that it give '0', however look the condition above! we're currently in CD device!:
					float HitTime = AttackController->GetServerTime_Synched() - ((AttackController->RTT) * RTTFactor);
					if (AttackBlasterCharacter->GetLagComponent())
					{
						AttackBlasterCharacter->GetLagComponent()->ServerScoreRequest(
							Start, 
							HitTarget, 
							HitBlasterCharacter, 
							HitTime, 
							this);
					}
				}
			}
		}

		//Apply HitSound + HitParticle on Whatever it hit (henc no need if(Char) )
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, HitResult.ImpactPoint);
			//our particl isn't symetric, hence this ImpactNormal.Rotation() rather than ZeroRotator is much better:
		UGameplayStatics::SpawnEmitterAtLocation( this, HitParticle, HitResult.ImpactPoint,
			HitResult.ImpactNormal.Rotation(), //rather than ZeroRotator
			true
		);
	}

//This is how we spawn a beam of a Particle Asset:
	//it must be played out here so that it will be spawned no matter it hit something or not
	UParticleSystemComponent* Component =	UGameplayStatics::SpawnEmitterAtLocation(this, BeamParticle, Start, FRotator(),  true);
	//this "Target" match the name in the BeamParticle asset to be selected from UE5
	if(Component) Component->SetVectorParameter(FName("Target"), BeamEnd);

//this part componensate the SMG didn't have any animation to play FireSound and HitPArticle for it, it need to be played in any cases so it is here:
	UGameplayStatics::PlaySoundAtLocation(this, FireSound, MuzzleFlashSocketTransform.GetLocation());
	//Just try to spawn where the Socket is, i.e use MuzzleFlash Transform,if it is wrong in direction then simply go the SMG mesh and rotate it to match, but I'm sure it matches already, I got that feeling LOL:
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),FireParticle,MuzzleFlashSocketTransform,true);
}

//MOVE TO PARENT:
//review, in case it didn't hit anything in first DOlineTrace it return end = 80.000, So dont worry
//this expect you to pass in 'HitTarget_DoLineTrace_CrossHairs' to work expected
//FVector AHitScanWeapon::RandomEndWithScatter(const FVector& HitTarget)
//{
//	//can find 'Start' itself, so now you can remove the parameter:
//	FTransform MuzzleFlashSocketTransform = WeaponMesh->GetSocketTransform(FName("MuzzleFlash")); 
//	FVector Start = MuzzleFlashSocketTransform.GetLocation();
//
//	//this is where sphere center should be, from which we + RandomUnitVector * (0->R)
//	FVector SphereCenterEnd = Start + (HitTarget - Start).GetSafeNormal() * DistanceToSphere;
//
//	float RandomRadius = FMath::RandRange(0.f, SphereRadius);
//	FVector RandomPointWithinSphere = SphereCenterEnd + UKismetMathLibrary::RandomUnitVector() * RandomRadius;
//
//	//we dont stop at RandomPointWithinSphere that is 800.f = 8m only, we extend it so that it makes more sense
//	//Because this will be used for SMG (not just Shotgun, So stephen and I temporary trace it as far as End point in DoLineTrace_Crosshairs LOL, I JUST define TRACE_LENGTH 80000 in WeaponTypes.h:
//	FVector ActualEnd = Start + (RandomPointWithinSphere - Start).GetSafeNormal() * TRACE_LENGTH; 
//
//	DrawDebugSphere(GetWorld(), SphereCenterEnd, SphereRadius, 12, FColor::Red, true);
//	DrawDebugSphere(GetWorld(), RandomPointWithinSphere, 4.f, 12, FColor::Blue, true); //you can draw point instead if you want, but I think snall sphere look more cool, and we will remove it anyway so It will save me sometime not drawing point lol!
//	DrawDebugLine(GetWorld(), Start, ActualEnd, FColor::Cyan, true);
//
//	return ActualEnd;
//}
