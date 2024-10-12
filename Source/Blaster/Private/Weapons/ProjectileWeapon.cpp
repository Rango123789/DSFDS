// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ProjectileWeapon.h"
#include "Weapons/Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	//this make ONLY the server copy can fire out a projectile (a proectile could only be spawned from the server no matter which player firing it), this is where the story begin LOL:
	//if (!HasAuthority()) return;

	FTransform MuzzleFlashSocket_Transform_InWeapon = WeaponMesh->GetSocketTransform(FName("MuzzleFlash"));

	FVector SpawnLocation = MuzzleFlashSocket_Transform_InWeapon.GetLocation();
	
	FVector FacingDirection = (HitTarget - SpawnLocation);

	FRotator SpawnRotation = FacingDirection.Rotation(); //accept ROLL = 0 -> YAW & PTICH

	FActorSpawnParameters SpawnParams;

	//stephen: SpawnParams.Owner = GetOwner() too, what the heck? is that sensible? :D :D
	SpawnParams.Owner = this; //the Owner of the to-be-spawn projectile is the ProectileWeapon - make sense!

	SpawnParams.Instigator = Cast<APawn>(GetOwner()); 

	GetWorld()->SpawnActor<AProjectile>(
		ProjectileClass,
		SpawnLocation,SpawnRotation,SpawnParams
	);
}
