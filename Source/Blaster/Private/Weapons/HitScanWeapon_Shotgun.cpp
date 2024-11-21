// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HitScanWeapon_Shotgun.h"
#include <Kismet/GameplayStatics.h>
#include "Characters/BlasterCharacter.h"
#include "Particles/ParticleSystemComponent.h"

void AHitScanWeapon_Shotgun::Fire(const FVector& HitTarget)
{
	if (WeaponMesh == nullptr || GetWorld() == nullptr) return;

	AWeapon::Fire(HitTarget); //we want fire anim and BP_Casing_X to be spawned

	//AHitScanWeapon::Fire(HitTarget); //NAH! we dont want this (by the way this also include AWeapon::Fire() LOL)
	
/***START specialize the Shotgun from here :***/

//STAGE1: Do Line Trace
				//GROUP1: outside of all

	FTransform MuzzleFlashSocketTransform = WeaponMesh->GetSocketTransform(FName("MuzzleFlash")); 

	FVector Start = MuzzleFlashSocketTransform.GetLocation();

	//belong to this weapon && its owner=attacker=who shoot this weapon - hence outside of all:
	ABlasterCharacter* AttackBlasterCharacter = Cast<ABlasterCharacter>(GetOwner());
	AController* AttackController = AttackBlasterCharacter->GetController();

	FVector End{};
	
				//GROUP2: specific to each tim call LineTrace, put them all into for loop, except ApplyDamage will applied ONCE per Hit char:
	//this map contains all hit chars and its number of being hit after NumOfPellets:

	TMap<ABlasterCharacter*, uint32> HitCharsMap; //hopefully their starting value is 0

	for (int32 i = 0; i < NumOfPellets; i++)
	{
		//each Trace has different HitResult:
		FHitResult HitResult;

		//this line is the KEY of the lesson:
		End = RandomEndWithScatter(Start, HitTarget);

		GetWorld()->LineTraceSingleByChannel(HitResult,Start,End,ECollisionChannel::ECC_Visibility);

//STAGE2: check if it hit any ABasterCharacter_instance and apply damage on him

		FVector BeamEnd = End; //if hit something we change it inside the next if, otherwise it will stay this 'End'
		if (HitResult.bBlockingHit)
		{
			if (HitResult.GetActor()) BeamEnd = HitResult.ImpactPoint;

			//each hit can hit nothing, hit non-character or a character:
			ABlasterCharacter* HitBlasterCharacter = Cast<ABlasterCharacter>(HitResult.GetActor());

			//if it hit the current BlasterCharacter, add him to the Map /& +1 hit:
			if (HitBlasterCharacter)
			{
				//this code is moved outside for new approach: "apply all hits ONCE per hit char" after the for loop.
				//Review, HitScanWeapon::Fire is currently run in all devices -> hence we need to add 'HasAuthority()' for ApplyDamge line, the rest code is cosmetic we leave it as 'FREE'!
				//Review, Projectile::OnHit is only set to run the server (for bullet) ->ApplyDamage is only in the server naturally - just to compare, not relevant here:
					//if (HasAuthority())
					//{
					//	UGameplayStatics::ApplyDamage(
					//		HitBlasterCharacter, //this will trigger HitBlasterCharacter::ReceiveDamage()
					//		Damage,
					//		AttackController,    //this will be important at where you receive it for purpose
					//		this,
					//		UDamageType::StaticClass()
					//	);
					//}

				if (HitCharsMap.Contains(HitBlasterCharacter))
				{
					HitCharsMap[HitBlasterCharacter] += 1; //or use ++
				}
				else
				{
					//there is overload version that you can add them in one line LOL 
					//can also use .Emplace (HitCharsMap.Add( , 1) || HitCharsMap.Emplace( , 1) 
					HitCharsMap.Add(HitBlasterCharacter);
					HitCharsMap[HitBlasterCharacter] = 1;
				}
			}

			//Apply HitSound + HitParticle on Whatever it hit (henc no need if(Char) )
			UGameplayStatics::PlaySoundAtLocation(this, HitSound, HitResult.ImpactPoint,
				FMath::RandRange(0.f,0.7f), //random volume, I dont want it to loud lol
				FMath::RandRange(-1.f, 1.f) //random pitch
			);
			//our particl isn't symetric, hence this ImpactNormal.Rotation() rather than ZeroRotator is much better LOL:
			UGameplayStatics::SpawnEmitterAtLocation(this,HitParticle,HitResult.ImpactPoint,HitResult.ImpactNormal.Rotation(), true);
		}

		//this need to be played per pellet, no matter hit something or not:
		UParticleSystemComponent* Component = UGameplayStatics::SpawnEmitterAtLocation(this, BeamParticle, Start, FRotator(), true);
		//this "Target" match the name in the BeamParticle asset to be selected from UE5
		if (Component) Component->SetVectorParameter(FName("Target"), BeamEnd);

	}
	

	//this need to play per FIRE only, however Shotgun has animation already, so these 2 lines are not needed:
	UGameplayStatics::PlaySoundAtLocation(this, FireSound, MuzzleFlashSocketTransform.GetLocation());
	//Just try to spawn where the Socket is, i.e use MuzzleFlash Transform,if it is wrong in direction then simply go the SMG mesh and rotate it to match, but I'm sure it matches already, I got that feeling LOL:
	UGameplayStatics::SpawnEmitterAtLocation(
		GetWorld(),
		FireParticle,
		MuzzleFlashSocketTransform,
		true
	);

	//now apply all hits ONCE per hit char:
	for (auto& Pair : HitCharsMap)
	{
		//Pair.Key =HitCharacter_i
		//Pair.vlue = num of hit of that  HitCharacter_i
		if (HasAuthority() && Pair.Key) //you can move this out so that the for loop wont even start in clients
		{
			UGameplayStatics::ApplyDamage(
				Pair.Key, //this will trigger HitBlasterCharacter::ReceiveDamage()
				Damage * Pair.Value,
				AttackController,    //this will be important at where you receive it for purpose
				this,
				UDamageType::StaticClass()
			);
		}
	}
}
