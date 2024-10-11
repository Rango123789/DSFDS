// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ProjectileWeapon.h"
#include "Weapons/Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	//the AWEwapon::bReplicates = true and it is 'non-controlled' actors
	//when the game starts, it is always spawned from the server first and then propogate to clients
	// hence only the server has authority over it (in case AWEwapon::bReplicates = false, each device has each own local copy and no replicated, ie each device has authority over its own local copy!)

	//this make ONLY the server copy can fire out a projectile (a proectile could only be spawned from the server no matter which player firing it), this is where the story begin LOL:
	if (!HasAuthority()) return;

	//Ready arguments: stephen GetSocketByName first, and so on.
	FTransform MuzzleFlashSocket_Transform_InWeapon = WeaponMesh->GetSocketTransform(FName("MuzzleFlash"));
	FVector SpawnLocation = MuzzleFlashSocket_Transform_InWeapon.GetLocation();
	
	FVector FacingDirection = (HitTarget - SpawnLocation);

	FRotator SpawnRotation = FacingDirection.Rotation(); //accept ROLL = 0 -> YAW & PTICH

	FActorSpawnParameters SpawnParams;

	//stephen: SpawnParams.Owner = GetOwner() too, what the heck? is that sensible? :D :D
	SpawnParams.Owner = this; //the Owner of the to-be-spawn projectile is the ProectileWeapon - make sense!

	SpawnParams.Instigator = Cast<APawn>(GetOwner()); //Owner of the Weapon will in turn be the instigator of his to-be-spawn projectile , We did call EquippedWeapon->SetOwner(Character) in CombatComponent::Equip() remember? hell yeah!

	//You may be tempting to pass in FTransform 'MuzzleFlashSocket_Transform_InWeapon' directly
	//However the even better directions for the Projectile is [CenterScreen -> End/HitTarget]
	//So go back to DoLineTrace and store it to be used here LOL
	//that explain why we need 'const FVector& HitTarget' LOL!
	GetWorld()->SpawnActor<AProjectile>(
		ProjectileClass,
		SpawnLocation,SpawnRotation,SpawnParams
	);
}
