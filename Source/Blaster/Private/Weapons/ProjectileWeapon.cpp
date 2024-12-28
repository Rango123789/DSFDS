// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/ProjectileWeapon.h"
#include "Weapons/Projectile.h"
#include "Characters/BlasterCharacter.h"

//this already consider serverside rewind - break 'HasAuthority' at the outermost tire:
void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	if (WeaponMesh == nullptr) return;

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


	AProjectile* SpawnedProjectile = nullptr;

	//default back to OLD code, replicated Projectile dont need to use rewind
	if (!bUseServerSideRewind && HasAuthority()) 
	{
		SpawnedProjectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
		 //I dont think there is any relevance to 'passing non-replicated pointer' rule? - see explain in the bottom
		SpawnedProjectile->Damage = Damage;
		SpawnedProjectile->Damage_HeadShot = Damage_HeadShot;

		//affect next STAGE:
		SpawnedProjectile->bUseServerSideRewind_TIRE2 = false;

		SpawnedProjectile->bServerHoldWeapon = true;
	}

	if (bUseServerSideRewind)
	{
		if (HasAuthority() )
		{
			if (OwnerCharacter->IsLocallyControlled()) 
			{
				//default back to OLD code, replicated Projectile dont need to use rewind
				SpawnedProjectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
				//stephen chaneg this to true to avoid 'double damage', anyway stephen way is wrong
				// I dont think there is any relevance to 'passing non-replicated pointer' rule? - see explain in the bottom
				SpawnedProjectile->Damage = Damage;
				SpawnedProjectile->Damage_HeadShot = Damage_HeadShot;

				//affect next STAGE:
				SpawnedProjectile->bUseServerSideRewind_TIRE2 = false; 

				SpawnedProjectile->bServerHoldWeapon = true;
			}
			else //spawn non-replicated projectile in the server if it doesn't controll the weapon, in this case one of the client will satify (**), the rest will satify the else of (**) for comstic effect (we dont let them ApplyDamage in stage2)
			{
				SpawnedProjectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass_Rewind, SpawnLocation, SpawnRotation, SpawnParams);

				//affect next STAGE:
				SpawnedProjectile->bUseServerSideRewind_TIRE2 = false;

				SpawnedProjectile->bServerHoldWeapon = false;
			}
		}

		if (!HasAuthority())
		{
			if (OwnerCharacter->IsLocallyControlled()) //(**)
			{
				
				SpawnedProjectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass_Rewind, SpawnLocation, SpawnRotation, SpawnParams);
				SpawnedProjectile->bUseServerSideRewind_TIRE2 = true;

				//this can be done in Projectile implementation (either in BeginPlay or Constructor I guess):
				SpawnedProjectile->TraceStart = SpawnedProjectile->GetActorLocation();
				SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitalSpeed_ProjectilePath;

				//only this case you really need it, other cases are 'consequences' (or optional if the intended damage is P::Damage or if they're set to be the same, which they should)
				SpawnedProjectile->Damage = Damage;
				SpawnedProjectile->Damage_HeadShot = Damage_HeadShot;

				SpawnedProjectile->bServerHoldWeapon = false; // in fact doesn't important at all
			}
			else
			{
				SpawnedProjectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass_Rewind, SpawnLocation, SpawnRotation, SpawnParams);
				SpawnedProjectile->bUseServerSideRewind_TIRE2 = false;
			}
		}
	}
}

void AProjectileWeapon::BeginPlay()
{
	Super::BeginPlay();

}

//FULL COMMENT version: this already consider serverside rewind - break 'HasAuthority' at the outermost tire:
//void AProjectileWeapon::Fire(const FVector& HitTarget)
//{
//	if (WeaponMesh == nullptr) return;
//
//	Super::Fire(HitTarget);
//
//	//these code can be run in all devices, no problem, note that the transform subject to be different by ping between server and clients, but is not a problem:
//
//	FTransform MuzzleFlashSocket_Transform_InWeapon = WeaponMesh->GetSocketTransform(FName("MuzzleFlash"));
//
//	FVector SpawnLocation = MuzzleFlashSocket_Transform_InWeapon.GetLocation();
//
//	FVector FacingDirection = (HitTarget - SpawnLocation);
//
//	FRotator SpawnRotation = FacingDirection.Rotation(); //accept ROLL = 0 -> YAW & PTICH
//
//	FActorSpawnParameters SpawnParams;
//
//	//stephen: SpawnParams.Owner = GetOwner() too, what the heck? is that sensible? :D :D
//	SpawnParams.Owner = this; //the Owner of the to-be-spawn projectile is the ProectileWeapon - make sense!
//
//	//GetOwnwer() of this weapon is the Character holding it.
//	SpawnParams.Instigator = Cast<APawn>(GetOwner()); //this is downcast, but is needed as GetOwner() return AActor*
//
//	//NEW EXTRA code for safe, nothing more: you can in fact re-use 'SpawnParams.Instigator' instead so that you dont even need to include and Cast to the bigger child again :D 
//	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : OwnerCharacter;
//
//	//but this spawning code is important, this is where we need to work on:
//		//UPDATE: weapon is not char, only one weapon per device, but 3 chars per device
//		//there is in fact only "one weapon" in each device, only one char holding it in each device
//		//So if it HasAuthority() && IsLocallyControlled() then there is no other proxy that HasAuthority() that is !IsLocallyControlled() , because there is only one weapon per device!!!!
//		// in short, there is only one weapon in the server, that its owner can be either LocallyControlled or !LocallyControlled - but not both :D :D (other chars in the server dont even hold the weapon and they're IRRELEVANT!, yeah!)
//		//this explain lain we do the TIRE3 "else, if" like that is correct and consider all worst cases!
//
//		//this is rather important that you set some important value to set Projectile::bUseRewind_TIRE2 for PART2 as Projectile::OnHit() trigger and so that you can decide you should apply damage or not!
//	AProjectile* SpawnedProjectile = nullptr;
//
//	//default back to OLD code, replicated Projectile dont need to use rewind
//	if (!bUseServerSideRewind && HasAuthority())
//	{
//
//		SpawnedProjectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
//		SpawnedProjectile->bUseServerSideRewind_TIRE2 = false;
//		//Stephen does this, but this doesn't make sense, you shouldn't use Weapon::Damage that is meant for HitScaneWeapon only for ProjectileWeapon, the Projectile itself will decide what damage is
//		//also AProjectile has its own initial value of damage already what bother you to set it like this?
//
//		 //I dont think there is any relevance to 'passing non-replicated pointer' rule? - see explain in the bottom
//		SpawnedProjectile->Damage = Damage;
//	}
//
//	if (bUseServerSideRewind)
//	{
//		//there is only one weapon in the server, its owner could be either CD or non-CD but can't be both
//		//meaning either of the if,else inside this is executed but not both (I LITERALLY mean untilmiately in total, if you know what I mean, there is only one weapon in the server so there is no next turn LOL) 
//		if (HasAuthority())
//		{
//			//second condition must move down here so that in case it is 'NOT locally controlled' , it has a chance to spawn non-replicated projectile for cosmetic effect as well
//			if (OwnerCharacter->IsLocallyControlled()) //(*) , spawn replicated projectile in the server if it does controll the weapon, in this case none of clients can satify (**). but still it will satisy the else of (**) and spawn 'a local non-replicated projectile' in all clients - meaning in this case you will see '2 projectiles: one localled spawned, one replicated back from the server- oh my god! this is a weakness :D :D - we must accept it for now LOL. Shocking news: because when server shoot it takes the same 'ping' time for Fire() function to reach client, and it also take the same time for Fire()::Spawn Replicated Projectile to reach clients, hence clients accidentally spawn 2 projectiles: one non-replicated, one replicated at the very same time and very place, make it feel like, as if there is only one, oh my good! it is accidentially good and we dont need to fix it any more, hell yeah!
//			{
//				//default back to OLD code, replicated Projectile dont need to use rewind
//				SpawnedProjectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
//				SpawnedProjectile->bUseServerSideRewind_TIRE2 = false;
//				//Stephen does this, but this doesn't make sense, you shouldn't use Weapon::Damage that is meant for HitScaneWeapon only for ProjectileWeapon, the Projectile itself will decide what damage is
//				//also AProjectile has its own initial value of damage already what bother you to set it like this?
//				// I dont think there is any relevance to 'passing non-replicated pointer' rule? - see explain in the bottom
//				SpawnedProjectile->Damage = Damage;
//			}
//			else //spawn non-replicated projectile in the server if it doesn't controll the weapon, in this case one of the client will satify (**), the rest will satify the else of (**) for comstic effect (we dont let them ApplyDamage in stage2)
//			{
//				SpawnedProjectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass_Rewind, SpawnLocation, SpawnRotation, SpawnParams);
//				SpawnedProjectile->bUseServerSideRewind_TIRE2 = false;
//			}
//		}
//
//		//the same logic here, only one weapon per device, and its owner could be either CD or non-CD. However in total there will be more than one turn because there is many clients right (unlike the server, only have one server LOL)
//		//there is only one LocallyControlled owner of the weapon, and it could be either HasAuthority() or !NotHasAuthorty() - but not both in total, i.e (*) and (**) are mutually-exclusive - ONLY either of them can be run - but never both!  
//		if (!HasAuthority())
//		{
//			if (OwnerCharacter->IsLocallyControlled()) //(**)
//			{
//				//this is the only place we set bUseServerSideRewind_TIRE2 = true, and this is the only place we need to set Projectile::InitialVelocity_Path , TraceStart_Path
//				//but why would we need that? didn't we set them all in Projectile itself locally already?
//				//no we didn't LOL, we only set InitilSpeed nothing more, However we can in fact do it locally from Projectile::BeginPlay if we want (or even in its constructor) so that you dont have to do it here any more
//				//simply because we did setup 'SpawnParams' above and pass it in as we spawn actor right??
//				//but anyway let's follow stephen.
//				SpawnedProjectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass_Rewind, SpawnLocation, SpawnRotation, SpawnParams);
//				SpawnedProjectile->bUseServerSideRewind_TIRE2 = true;
//				//this can be done in Projectile implementation (either in BeginPlay or Constructor I guess):
//				SpawnedProjectile->TraceStart = SpawnedProjectile->GetActorLocation();
//				SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitalSpeed_ProjectilePath;
//
//				//I dont think there is any relevance to 'passing non-replicated pointer' rule? 
//				//it is meant to replace any possible hack value passed-in as a client send ServerScoreRequest? - see explain in the bottom
//				SpawnedProjectile->Damage = Damage;
//			}
//			else
//			{
//				//this will be in fact 'duplicate, redundant non-replicated projectile' in case owner of the weapon is the server :D :D 
//				//to avoid this I think we could create extra bool 'bShouldSpawnNonrepliactedProjectile' when we construct the projectile based on its owner is controlled by a client or the server, if it is controlled by the server, we set it to false and dont even do this code:
//				//however I accept the imperfection for now lol:
//				SpawnedProjectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass_Rewind, SpawnLocation, SpawnRotation, SpawnParams);
//				SpawnedProjectile->bUseServerSideRewind_TIRE2 = false;
//			}
//		}
//	}
//	//this is what we usually do, but no more applicable, we need to adapt it a bit, unlike 'JUST' apply damage, we can decide to apply damage or not, where for SPAWNING projectile in case you use server side rewind spawn all non-replicated projectiles locally, not only you need to spawn it in the DC but also in the rest devices for cosmetic effects, you can't just spawn in DC alone right? :) :)
//		//if (!bUseServerSideRewind)
//		//{
//		//	GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
//		//}
//		//if (bUseServerSideRewind)
//		//{
//		//	if (HasAuthority() && OwnerCharacter->IsLocallyControlled())
//		//	{
//		//		GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
//		//	}
//		//	if (!HasAuthority() && OwnerCharacter->IsLocallyControlled())
//		//	{
//		//		GetWorld()->SpawnActor<AProjectile>(ProjectileClass_Rewind, SpawnLocation, SpawnRotation, SpawnParams);
//		//	}
//		//}
//}


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


/*
PUZZLE: why stephen need to do SpawnedProjectile->Damage = ShootingWeapon::Damage?

*He said1:
- Now these replicated projectiles handle damage already.
- And we decided to change the projectile to use the weapons damage variable as we can't just pass a non-replicated projectile up to the server for server side rewind.
- So the server is going to check the equipped weapon now for the damage for projectile weapons.
*Explain: [needVERFIEID, 99% true] well he try to a client cheating by passing any amount of damage
, but we can still pass in 'SpawnedProjectile->Damage' directly right?
, well as long the function require 'a float' (rather than a replicated pointer of actor type) client can cheat and pass in any number they want
, they can in fact replace 'the value of 'SpawnedProjectile->Damage' by their own number LOL

*Meaning whenever you use BP_ProjectileWeapon_X
, you must go and see how much the Projectile_X::Damage is and go back and set the weapon to the same value
, hell yeah!

*He said2:
Now we need to think about damage here for a second.
Previously, our projectiles had damage on them, but now we need to use the weapons damage value because
if we're spawning a non replicated damage causer, we can't simply pass that damage causer up to the
server.
Passing that pointer up will not work because that variable is not replicated.
You can only pass a replicated pointer to the server via an r.p c as the server will make sure it understands what you mean when you pass it that pointer.

So now we have to check the weapons damage variable on the server when confirming hits.
Now projectiles that are replicated will still use their damage variable.
So this means once we spawn a projectile that's using server side rewind, we need to set its damage
equal to the damage on the weapon.

So this means that once we spawn a projectile we should set its damage variable.
Now on projectile.
If we search for damage, we see that it's a protected variable.
I'm going to move this up to the public section so we can easily set it without creating a public setter.
And that's going to go up here with our server side rewind variables, and we're now going to set this
directly once we spawn a projectile from our projectile weapon class.

So I'm not going to have it exposed anymore this way.

Anyone who goes into the projectile blueprint will not see that they can change it as it's now just
going to be set directly when we spawn it.

So we need to set that when we spawn a projectile, and we'll do that here where it matters.
It matters when we're spawning a replicated projectile as replicated projectiles are going to cause
damage in their hit events.
So here on the server host using replicated projectiles we're going to take spawn projectile.
Said its damage equal to the damage on the projectile weapon.

And if we go to a weapon that age and search for damage.

We'll see that it's here in the protected section, so we have access to it.
So we need to do this when spawning a replicated projectile, but we also need to do this when spawning
a projectile that's not replicated using server side rewind.
So we're going to set it here as well.
*/

