// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HitScanWeapon.h"
#include <Kismet/GameplayStatics.h>
#include "Kismet/KismetMathLibrary.h" //RandomUnitVector
#include "Characters/BlasterCharacter.h"
#include "Particles/ParticleSystemComponent.h"
//HitTarget will be passed from Combat::DoLineTrace():: center screen -> forwards, everything is already setup, we need only to use it:
void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	if (WeaponMesh==nullptr || GetWorld()==nullptr) return;

	Super::Fire(HitTarget); //play FireAnimation_inWeapon --> FireSound + FireParticle in it
                            //Spawn ACasing
//STAGE1: Do Line Trace
	FHitResult HitResult;
	FTransform MuzzleFlashSocketTransform = WeaponMesh->GetSocketTransform(FName("MuzzleFlash")); //if you get socket by name instead, you have a chance to check whether it is valid or not, but anyway If it's not valid then I will see it LOL, and I will make sure the name match, so I dont follow Stephen's way

	FVector Start = MuzzleFlashSocketTransform.GetLocation();
	FVector End{};

	if (bUseScatter)
	{
		End = RandomEndWithScatter(Start, HitTarget);
	}
	else 
	{
		End = Start + (HitTarget - Start) * 1.25f; //1.25 to make sure it hit the target, rather than just hit the surface that 50/50 you will fail!
	}

	GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECollisionChannel::ECC_Visibility	
	);
//STAGE2: check if it hit any ABasterCharacter_instance and apply damage on him

	FVector BeamEnd = End; //if hit something we change it inside the next if

	if (HitResult.bBlockingHit)
	{
		if(HitResult.GetActor()) BeamEnd = HitResult.ImpactPoint;

		  ABlasterCharacter* HitBlasterCharacter = Cast<ABlasterCharacter>(HitResult.GetActor());

		  ABlasterCharacter* AttackBlasterCharacter = Cast<ABlasterCharacter>(GetOwner());
	       
		  //if it hit BlasterCharacter, apply damage on him
		  if (HitBlasterCharacter)
		  {
			  //I think it is okay to pass in a null AttackController LOL, who know when the attacker will after he shoot someone lol:
			  //shocking new: stephen create this outside and check on this, make simulated proxies can't even play their HitSound and HitParticle, well it make sense because all simulated proxies will have their PC = nullptr from ever :D 
			  //so dont check it with the cosmetic group, only check - if needed - with the DoAction requiring an PC as parameter
			  AController* AttackController = AttackBlasterCharacter->GetController(); //it only require AController, no point creating bigger one and include extra header LOL

			  //Review, HitScanWeapon::Fire is currently run in all devices -> hence we need to add 'HasAuthority()' for ApplyDamge line, the rest code is cosmetic we leave it as 'FREE'!
			  //Review, Projectile::OnHit is only set to run the server (for bullet) ->ApplyDamage is only in the server naturally - just to compare, not relevant here:
			  if (HasAuthority())
			  {
				  UGameplayStatics::ApplyDamage(
					  HitBlasterCharacter, //this will trigger HitBlasterCharacter::ReceiveDamage()
					  Damage,
					  AttackController,    //this will be important at where you receive it for purpose
					  this,
					  UDamageType::StaticClass()
				  );
			  }
		  }

		  //Apply HitSound + HitParticle on Whatever it hit (henc no need if(Char) )
		  UGameplayStatics::PlaySoundAtLocation(this, HitSound, HitResult.ImpactPoint);
				//our particl isn't symetric, hence this ImpactNormal.Rotation() rather than ZeroRotator is much better LOL:
		  UGameplayStatics::SpawnEmitterAtLocation(
			  this,
			  HitParticle,
			  HitResult.ImpactPoint,
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
	UGameplayStatics::SpawnEmitterAtLocation(
		GetWorld(),
		FireParticle,
		MuzzleFlashSocketTransform,
		true
	);
}

//review, in case it didn't hit anything in first DOlineTrace it return end = 80.000, So dont worry
FVector AHitScanWeapon::RandomEndWithScatter(const FVector& Start, const FVector& HitTarget)
{
	//this is where sphere center should be, from which we + RandomUnitVector * (0->R)
	FVector SphereCenterEnd = Start + (HitTarget - Start).GetSafeNormal() * DistanceToSphere;

	float RandomRadius = FMath::RandRange(0.f, SphereRadius);
	FVector RandomPointWithinSphere = SphereCenterEnd + UKismetMathLibrary::RandomUnitVector() * RandomRadius;

	//we dont stop at RandomPointWithinSphere that is 800.f = 8m only, we extend it so that it makes more sense
	//Because this will be used for SMG (not just Shotgun, So stephen and I temporary trace it as far as End point in DoLineTrace_Crosshairs LOL, I JUST define TRACE_LENGTH 80000 in WeaponTypes.h:
	FVector ActualEnd = Start + (RandomPointWithinSphere - Start).GetSafeNormal() * TRACE_LENGTH; 

	DrawDebugSphere(GetWorld(), SphereCenterEnd, SphereRadius, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), RandomPointWithinSphere, 4.f, 12, FColor::Blue, true); //you can draw point instead if you want, but I think snall sphere look more cool, and we will remove it anyway so It will save me sometime not drawing point lol!
	DrawDebugLine(GetWorld(), Start, ActualEnd, FColor::Cyan, true);

	return ActualEnd;
}
