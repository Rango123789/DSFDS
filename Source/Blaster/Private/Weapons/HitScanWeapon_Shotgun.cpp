// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HitScanWeapon_Shotgun.h"
#include <Kismet/GameplayStatics.h>
#include "Characters/BlasterCharacter.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/KismetMathLibrary.h" //RandomUnitVector
#include "PlayerController/BlasterPlayerController.h"
#include "CharacterComponents/LagCompensationComponent.h"

//Shotgun dont use this version anymore, just let it be as reference LOL:
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
	ABlasterPlayerController* AttackController =Cast<ABlasterPlayerController>( AttackBlasterCharacter->GetController());

	FVector End{};
	
				//GROUP2: specific to each time call LineTrace, put them all into for loop, except ApplyDamage will applied ONCE per Hit char:
	//this map contains all hit chars and its number of being hit after NumOfPellets:

	TMap<ABlasterCharacter*, uint32> HitCharsMap; //hopefully their starting value is 0

	for (int32 i = 0; i < NumOfPellets; i++)
	{
		//each Trace has different HitResult:
		FHitResult HitResult;

	//We will replace End by HitTarget and perform RandomEndWithScatter in TIRE1 instead in next lesson , don worry:
		//this line is the KEY of the lesson: 
		End = RandomEndWithScatter(HitTarget);

		GetWorld()->LineTraceSingleByChannel(HitResult,Start,End,ECollisionChannel::ECC_Visibility);
		//GetWorld()->LineTraceSingleByChannel(HitResult, Start, HitTarget, ECollisionChannel::ECC_Visibility);

//STAGE2: check if it hit any ABasterCharacter_instance and apply damage on him
//they will run in all devices, fine!
		FVector BeamEnd = End; //if hit something we change it inside the next if, otherwise it will stay this 'End'
		//FVector BeamEnd = HitTarget;

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
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),FireParticle,MuzzleFlashSocketTransform,true);

	//now apply all hits ONCE per hit char:


	for (auto& Pair : HitCharsMap)
	{
		//Pair.Key =HitCharacter_i
		//Pair.value = num of hit of that  HitCharacter_i
		if (Pair.Key == nullptr) continue;

		UGameplayStatics::ApplyDamage(
			Pair.Key, //this will trigger HitBlasterCharacter::ReceiveDamage()
			Damage * Pair.Value,
			AttackController,    //this will be important at where you receive it for purpose
			this,
			UDamageType::StaticClass()
		);
	}
}

void AHitScanWeapon_Shotgun::ShotgunFire(const TArray<FVector_NetQuantize>& HitTargets)
{
	if (WeaponMesh == nullptr || GetWorld() == nullptr || HitTargets.Num() == 0) return;

	//well the fact is that the parent AWeapon::Fire only play FireAnimation and Spawn Casing at MuzzleFlash so it does need the exact HitTarget location, hence pass it Vector Zero will do in this case LOL, since we dont have HitTarget parameter in the hosting funciton any more:

	AWeapon::Fire( FVector() ); //we want fire anim and BP_Casing_X to be spawned

	//AHitScanWeapon::Fire(HitTarget); //NAH! we dont want this (by the way this also include AWeapon::Fire() LOL)

/***START specialize the Shotgun from here :***/

//STAGE1: Do Line Trace
			//GROUP1: outside of all

	FTransform MuzzleFlashSocketTransform = WeaponMesh->GetSocketTransform(FName("MuzzleFlash"));

	FVector Start = MuzzleFlashSocketTransform.GetLocation();

	//belong to this weapon && its owner=attacker=who shoot this weapon - hence outside of all:
	ABlasterCharacter* AttackBlasterCharacter = Cast<ABlasterCharacter>(GetOwner());
	if ( AttackBlasterCharacter == nullptr) return;
	ABlasterPlayerController* AttackController = Cast<ABlasterPlayerController>(AttackBlasterCharacter->GetController());
		//Do this mean if the attacking player is elimmed at the time it shot other chars, other chars wont get hurt
		//i dont typically do this, but just for ease of use in next step:
	if (AttackController == nullptr) return;

//replace the left of 'End' = ___  with  = 'HitTargets[i]'
	FVector End{};

			//GROUP2: specific to each tim call LineTrace, put them all into for loop, except ApplyDamage will applied ONCE per Hit char:
//this map contains all hit chars and its number of being hit after NumOfPellets:

	TMap<ABlasterCharacter*, uint32> HitCharsMap; //hopefully their starting value is 0

	for (int32 i = 0; i < HitTargets.Num(); i++)  // or < NumOfPellets also OKAY
	{
		//each Trace has different HitResult:
		FHitResult HitResult;

		//We will replace End by HitTarget and perform RandomEndWithScatter in TIRE1 instead in next lesson , don worry:
		//End = RandomEndWithScatter(HitTarget); 
		End = Start + (HitTargets[i] - Start) * 1.25f;

		GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility);
		//GetWorld()->LineTraceSingleByChannel(HitResult, Start, HitTarget, ECollisionChannel::ECC_Visibility);

//STAGE2: check if it hit any ABasterCharacter_instance and apply damage on him

		FVector BeamEnd = End; //if hit something we change it inside the next if, otherwise it will stay this 'End'
		//FVector BeamEnd = HitTarget;

		if (HitResult.bBlockingHit)
		{
			if (HitResult.GetActor()) BeamEnd = HitResult.ImpactPoint;

			DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 3.f, 12, FColor::Cyan, true);
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
				FMath::RandRange(0.f, 0.7f), //random volume, I dont want it to loud lol
				FMath::RandRange(-1.f, 1.f) //random pitch
			);
			//our particle isn't symetric, hence this ImpactNormal.Rotation() rather than ZeroRotator is much better LOL:
			UGameplayStatics::SpawnEmitterAtLocation(this, HitParticle, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation(), true);
		}

		//this need to be played per pellet, no matter hit something or not:
		UParticleSystemComponent* Component = UGameplayStatics::SpawnEmitterAtLocation(this, BeamParticle, Start, FRotator(), true);
		//this "Target" match the name in the BeamParticle asset to be selected from UE5
		if (Component) Component->SetVectorParameter(FName("Target"), BeamEnd);

	}

	//this need to play per FIRE only, however Shotgun has animation already, so these 2 lines are not needed:
	UGameplayStatics::PlaySoundAtLocation(this, FireSound, MuzzleFlashSocketTransform.GetLocation());
	//Just try to spawn where the Socket is, i.e use MuzzleFlash Transform,if it is wrong in direction then simply go the SMG mesh and rotate it to match, but I'm sure it matches already, I got that feeling LOL:
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FireParticle, MuzzleFlashSocketTransform, true);
	
	//we need to make this because ServerSideRewind_Shotgun need this parameter:
	//you can directly get its element with the tracing loop above, but anyway we know that the map only contain UNIQUE elements so why dont just extra it back LOL:
	TArray<ABlasterCharacter*> HitCharacters;
	for (auto& Pair : HitCharsMap)
	{
		HitCharacters.Add(Pair.Key);
	}

	uint32 temp = HitCharacters.Num();
	//now apply all hits ONCE per hit char:
	if (!bUseServerSideRewind && HasAuthority())
	{
		for (auto& Pair : HitCharsMap)
		{
			//Pair.Key =HitCharacter_i
			//Pair.value = num of hit of that  HitCharacter_i
			if (Pair.Key == nullptr) continue;

			UGameplayStatics::ApplyDamage(
				Pair.Key, //this will trigger HitBlasterCharacter::ReceiveDamage()
				Damage * Pair.Value,
				AttackController,    //this will be important at where you receive it for purpose
				this,
				UDamageType::StaticClass()
			);
		}
	}

	if (bUseServerSideRewind)
	{
		//if it is the sever controlling the attacking char, we dont need serverside rewind:
		if (HasAuthority() && AttackBlasterCharacter->IsLocallyControlled())
		{
			for (auto& Pair : HitCharsMap)
			{
				//Pair.Key =HitCharacter_i
				//Pair.value = num of hit of that  HitCharacter_i
				if (Pair.Key == nullptr) continue;

				UGameplayStatics::ApplyDamage(
					Pair.Key, //this will trigger HitBlasterCharacter::ReceiveDamage()
					Damage * Pair.Value,
					AttackController,    //this will be important at where you receive it for purpose
					this,
					UDamageType::StaticClass()
				);
			}
		}

		//this is the case we use serverside rewind:
		if (!HasAuthority() && AttackBlasterCharacter->IsLocallyControlled())
		{
			float HitTime = AttackController->GetServerTime_Synched() - ((AttackController->RTT) * RTTFactor);

			if(AttackBlasterCharacter->GetLagComponent())

			AttackBlasterCharacter->GetLagComponent()->ServerScoreRequest_Shotgun(
				Start,
				HitTargets,
				HitCharacters,
				HitTime,
				this
			);
		}

	}
}

void AHitScanWeapon_Shotgun::RandomEndsWithScatter_Shotgun(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
//OPTION1:
	//can find 'Start' itself, so now you can remove the parameter:
	FTransform MuzzleFlashSocketTransform = WeaponMesh->GetSocketTransform(FName("MuzzleFlash"));
	FVector Start = MuzzleFlashSocketTransform.GetLocation();

	//this is where sphere center should be, from which we + RandomUnitVector * (0->R)
	FVector SphereCenterEnd = Start + (HitTarget - Start).GetSafeNormal() * DistanceToSphere;

	for (int32 i = 0; i < NumOfPellets; i++)  // HitTargets.Num() - incorrect
	{
		float RandomRadius = FMath::RandRange(0.f, SphereRadius);
		FVector RandomPointWithinSphere = SphereCenterEnd + UKismetMathLibrary::RandomUnitVector() * RandomRadius;

		//we dont stop at RandomPointWithinSphere that is 800.f = 8m only, we extend it so that it makes more sense
		//Because this will be used for SMG (not just Shotgun, So stephen and I temporary trace it as far as End point in DoLineTrace_Crosshairs LOL, I JUST define TRACE_LENGTH 80000 in WeaponTypes.h:
		FVector ActualEnd = Start + (RandomPointWithinSphere - Start).GetSafeNormal() * TRACE_LENGTH;
		HitTargets.Add(ActualEnd);

		DrawDebugSphere(GetWorld(), RandomPointWithinSphere, 4.f, 12, FColor::Blue, true); //you can draw point instead if you want, but I think snall sphere look more cool, and we will remove it anyway so It will save me sometime not drawing point lol!
		DrawDebugLine(GetWorld(), Start, ActualEnd, FColor::Cyan, true);
	}
	//need only to draw the SPHERE once
	DrawDebugSphere(GetWorld(), SphereCenterEnd, SphereRadius, 12, FColor::Red, true);

////OPTION2: not used, cost more performance:
//	for (int32 i = 0; i < HitTargets.Num(); i++)
//	{
//		HitTargets.Add(RandomEndWithScatter(HitTarget));
//	}
}

//FVector AWeapon::RandomEndWithScatter(const FVector& HitTarget)
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