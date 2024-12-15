// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/ProjectileWeapon.h"
#include "Weapons/Projectile.h"
#include "Characters/BlasterCharacter.h"

//this already consider serverside rewind - break 'HasAuthority' at the outermost tire:
void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

//these code can be run in all devices, no problem, note that the transform subject to be different by ping between server and clients, but is not a problem:
	FTransform MuzzleFlashSocket_Transform_InWeapon = WeaponMesh->GetSocketTransform(FName("MuzzleFlash"));

	FVector SpawnLocation = MuzzleFlashSocket_Transform_InWeapon.GetLocation();

	FVector FacingDirection = (HitTarget - SpawnLocation);

	FRotator SpawnRotation = FacingDirection.Rotation(); //accept ROLL = 0 -> YAW & PTICH

	FActorSpawnParameters SpawnParams;

	//stephen: SpawnParams.Owner = GetOwner() too, what the heck? is that sensible? :D :D
	SpawnParams.Owner = this; //the Owner of the to-be-spawn projectile is the ProectileWeapon - make sense!

	//GetOwnwer() of this weapon is the Character holding it.
	SpawnParams.Instigator = Cast<APawn>(GetOwner()); //this is downcast, but is needed as GetOwner() return AActor*

	//NEW EXTRA code for safe, nothing more: you can in fact re-use 'SpawnParams.Instigator' instead so that you dont even need to include and Cast to the bigger child again :D 
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : OwnerCharacter;

//but this spawning code is important, this is where we need to work on:
	if (!bUseServerSideRewind)
	{
		GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
	}

	if (bUseServerSideRewind)
	{
		if (HasAuthority() && OwnerCharacter->IsLocallyControlled())
		{
			GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
		}

		if (!HasAuthority() && OwnerCharacter->IsLocallyControlled())
		{
			GetWorld()->SpawnActor<AProjectile>(ProjectileClass_Rewind, SpawnLocation, SpawnRotation, SpawnParams);
		}
	}




//this is what we usually do, but no more applicable, we need to adapt it a bit, unlike 'JUST' apply damage, we can decide to apply damage or not, where for SPAWNING projectile in case you use server side rewind spawn all non-replicated projectiles locally, not only you need to spawn it in the DC but also in the rest devices for cosmetic effects, you can't just spawn in DC alone right? :) :)
	//if (!bUseServerSideRewind)
	//{
	//	GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
	//}

	//if (bUseServerSideRewind)
	//{
	//	if (HasAuthority() && OwnerCharacter->IsLocallyControlled())
	//	{
	//		GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
	//	}

	//	if (!HasAuthority() && OwnerCharacter->IsLocallyControlled())
	//	{
	//		GetWorld()->SpawnActor<AProjectile>(ProjectileClass_Rewind, SpawnLocation, SpawnRotation, SpawnParams);
	//	}
	//}
}


//the OLD version without considering serside rewind:
//void AProjectileWeapon::Fire(const FVector& HitTarget)
//{
//	Super::Fire(HitTarget);
//	//this HOSTING function is called in all machine, but we only let the server spawn the Projectile, this block only run in the server
//	if (HasAuthority()) 
//	{
//		FTransform MuzzleFlashSocket_Transform_InWeapon = WeaponMesh->GetSocketTransform(FName("MuzzleFlash"));
//
//		FVector SpawnLocation = MuzzleFlashSocket_Transform_InWeapon.GetLocation();
//	
//		FVector FacingDirection = (HitTarget - SpawnLocation);
//
//		FRotator SpawnRotation = FacingDirection.Rotation(); //accept ROLL = 0 -> YAW & PTICH
//
//		FActorSpawnParameters SpawnParams;
//
//		//stephen: SpawnParams.Owner = GetOwner() too, what the heck? is that sensible? :D :D
//		SpawnParams.Owner = this; //the Owner of the to-be-spawn projectile is the ProectileWeapon - make sense!
//
//		//GetOwnwer() of this weapon is the Character holding it.
//		SpawnParams.Instigator = Cast<APawn>(GetOwner()); //this is downcast, but is needed as GetOwner() return AActor*
//
//		GetWorld()->SpawnActor<AProjectile>(ProjectileClass,SpawnLocation,SpawnRotation,SpawnParams);
//	}
//}

