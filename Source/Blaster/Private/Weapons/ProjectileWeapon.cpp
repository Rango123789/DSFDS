// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ProjectileWeapon.h"
#include "Weapons/Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	//this HOSTING function is called in all machine, but we only let the server spawn the Projectile, this block only run in the server
	if (HasAuthority()) 
	{
		FTransform MuzzleFlashSocket_Transform_InWeapon = WeaponMesh->GetSocketTransform(FName("MuzzleFlash"));

		FVector SpawnLocation = MuzzleFlashSocket_Transform_InWeapon.GetLocation();
	
		FVector FacingDirection = (HitTarget - SpawnLocation);

		FRotator SpawnRotation = FacingDirection.Rotation(); //accept ROLL = 0 -> YAW & PTICH

		FActorSpawnParameters SpawnParams;

		//stephen: SpawnParams.Owner = GetOwner() too, what the heck? is that sensible? :D :D
		SpawnParams.Owner = this; //the Owner of the to-be-spawn projectile is the ProectileWeapon - make sense!

		//GetOwnwer() of this weapon is the Character holding it.
		SpawnParams.Instigator = Cast<APawn>(GetOwner()); //this is downcast, but is needed as GetOwner() return AActor*

		GetWorld()->SpawnActor<AProjectile>(ProjectileClass,SpawnLocation,SpawnRotation,SpawnParams);
	}
}
