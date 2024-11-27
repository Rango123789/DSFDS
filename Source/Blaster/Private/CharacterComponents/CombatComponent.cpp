// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterComponents/CombatComponent.h"
#include "Weapons/Weapon.h"
#include "Characters/BlasterCharacter.h"
#include "PlayerController/BlasterPlayerController.h"
#include "HUD/BlasterHUD.h"
#include "Weapons/Projectile.h"

#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h" //for DoLineTrace
#include "Kismet/GameplayStatics.h" //for DeprojectScreenToWorld
//#include "TimerManager.h" //NEWs

UCombatComponent::UCombatComponent()
	//: TimerDelegate( FTimerDelegate::CreateUObject(this, &ThisClass::FireTimer_Callback) )
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true; //if you tick later, then turn it on

	SetIsReplicated(true);

	//TimerDelegate.BindUFunction(this, FName("FireTimer_Callback")); 

	/*
	#Puzzle: 
+I didn't set UCombatComponent::SetIsReplicated(true)/LIKE
, how it can still work so well? did I check it from BP_Char?

+Well i set this from the HOSTING actor perspective already
, meaning for UActorComponent created by CreateDefaultSubobject we register the component in the hosting actor's constructor

+There is a thing you must be aware of
(1) Char::Char(){ Combat::SetIsReplicated(true)} and Combat::Combat(){  SetIsReplicated(true)  } in fact change the same underlying data
=you need to only change EITHER of them, if you do both, then the setting in the HOSTING actor will take precendence naturally (Actor's contructor run after its comp's constructor, who ever run after will be the final value)
(2)_[UPDATE] if you did do SetIsReplicated(true) LOCALLY in UCombatComponent
, then it will be the default value of it in the HOSTING actor too
, hell yeah!
(3) However for the built-in UActorComponent, UE5 dont typically set SetIsReplicated(true)
, perhaps because it suit better for the default case of single player
, hence if you use any buil-in Component, you gotta remember call HostingActor::UActorComponent::SetIsReplicated(true) within the Hosting Actor class!

+However a common practice is that you DO NOT do it in LOCAL UActorComponent class (even if it work) so that the default behaviour is NOT replicated
, you follow UE5's practice and only do it in hosting Actgor
	*/
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
	//DOREPLIFETIME(UCombatComponent, bIsFiring); //just added, BUT didn't affect anyway
	//DOREPLIFETIME(UCombatComponent, CarriedAmmo); //will still work but redudant for non-owning clients to receive it - they dont need it in this game
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly); //only owning proxy need to receive it
	DOREPLIFETIME_CONDITION(UCombatComponent, ThrowGrenade, COND_OwnerOnly); //only owning proxy need to receive it

	DOREPLIFETIME(UCombatComponent, CharacterState);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	//assign its value here (or BeginPlay()), so surely all character instances has it
	MaxWalkSpeed_Backup = Character->GetCharacterMovement()->MaxWalkSpeed;
	AimWalkSpeed = 300.f;

	if (Character)
	{
		DefaultPOV = Character->GetCamera()->FieldOfView;
		CurrentPOV = DefaultPOV;

		//I have no idea why he add this HasAuthority(), i thought it is like a table of reference so it is for IsLocallyControlled()/Owner at least? what the heck is going on LOL?
		//Well because CarriedAmmo will be set in Equip which in GOLDEN1 that will always run from the server, and also CarriedAmmo is replicated to COND_Owner only, so the owner get the needed value lol = OKAY!
		if (Character->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    //UPDATE: because other non-controlling device dont care about Crosshairs and POV of this camera so add IsLocallyContrlled() will save performance for other device
	if (Character && Character->IsLocallyControlled()) //ADDED!
	{
		SetHUDPackageForHUD(DeltaTime);
		SetPOVForCamera(DeltaTime);
	}
}

void UCombatComponent::Input_Reload()
{
//impliment things, RPCs(if needed) from here:
	//the GOLDEN1 is reduced to this, also no point to reload if you dont have any CarriedAmmo left:
	//we only reload if it is Unoccupied and currently not already Readling (avoiding Spam) -- && CharacterState != ECS_Reloading  is redudant
	if (CarriedAmmo > 0 &&
		EquippedWeapon &&
	    !EquippedWeapon->IsFull() &&
		CharacterState == ECharacterState::ECS_Unoccupied)
	{
		ServerInput_Reload();
	}
}

void UCombatComponent::Input_Throw()
{
	if (Character == nullptr) return;
	//so that it wont spam itself and any other montages:
	if (CharacterState != ECharacterState::ECS_Unoccupied) return;
	if (ThrowGrenade <= 0) return;

//i start to feel this STUPID lol, if you bother to make it happen in server first, why still bother to restrict it to be called in the server and then give up and make it work in the controlling device first?
	//Stephen said, that he want to play Montage in controlling device immediately to see it immediatly, and then send RPC to the server and other clients later:  FIRST TIME!
	// However in OnRep_CharState we will include the IsLocallycontroll case to avoid playing twice LOL: FIRST TIME!
	//Consequencely you must change the state for it to (even before it receive LEGAL value from server later)
	CharacterState = ECharacterState::ECS_Throwing; // I forget this line LOL

	Character->PlayThrowMontage(); //too see immediate effect in controlling device
	AttachEquippedWeaponToLeftHandSocket();

	UpdateHUD_ThrowGrenade(); //in case it is already the server and can't pass next check

	//the second consequence, because you do the 2 line above, so you can in fact OPTIONALLY put the 'ServerInput_Throw()' inside if(!HasAuthority())), because in worst case it is called in the server it skip this RPC it still has the 2 line above back it up! :D :D
	    //usual option:
	//ServerInput_Throw(); //this solve server part, CharacterState_will solve in OnRep_CharState
		//better option: as we already have 2 line above back up in worst case
	if (Character->HasAuthority() == false)
	{
		ServerInput_Throw();
	}
}

void UCombatComponent::ServerInput_Throw_Implementation()
{
	if (Character == nullptr) return;

	CharacterState = ECharacterState::ECS_Throwing; //trigger OnRep_ -->paste code for clients

	Character->PlayThrowMontage();
	AttachEquippedWeaponToLeftHandSocket(); //this is self-replicated, no need to call it in OnRep_CharacterState::Throwing at all _VERFIED - I test it!

	UpdateHUD_ThrowGrenade();
}

void UCombatComponent::ServerInput_Reload_Implementation()
{
	if (Character == nullptr) return;
	//this help trigger OnRep_, if it changes WeaponState from Unoccupied
	CharacterState = ECharacterState::ECS_Reloading; //self-replicated, and trigger its OnRep_X

	//Call this in ReloadEnd() with HasAuthority() if you want it to update HUD that time = everyone like this :)
	//UpdateHUD_CarriedAmmo(); //self-replicated, we have OnRep_CarriedAmmo work already

//all code bellow will be copied and pasted to OnRep_, STEPHEN factorize all of them into ReloadHandle() so that when we add more stuff to ReloadHandle() we dont need to go and paste to OnRep_ again LOL, BUT I say we way LOL
	//you must go and use AimNotify to reset it back to UnOccupied, otherwise you get stuck in state :D :D
	Character->PlayReloadMontage(); //call it in  OnRep_CharacterState too for clients
}

//I haven't call this anywhere yet, as this i meant for when Reloading
//this will be called in RealoadEnd Notify, giving the sense of rewarding after wating for the anim animation to finish, except Shotgun will use the specialized version below this one:
//Change Ammo + CarriedAmmo should be done in the server first, this function will be  wrapped inside ReloadEnd() and checked there, dont worry:
void UCombatComponent::UpdateHUD_CarriedAmmo()
{
	if (EquippedWeapon == nullptr) return;

	//change CarriedAmmo's value here when Reload, say when Reload, so it will be different a bit LOL, load X_max - x to get max or load y if y < X_max - x:
	int32 LoadAmount = FMath::Min(EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo(), CarriedAmmo);

	//Change Ammo here will trigger OnRep_Ammo ->SetHUDAmmo work automatically in clients!
	EquippedWeapon->SetAmmo(EquippedWeapon->GetAmmo() + LoadAmount);

	//Stephen swap order of them for sematic reason, but I prefer this, changing CarriedAmmo here ill trigger OnRep_CarriedAmmo  ->SetHUDCarriedAmmo work automatically in clients!
	CarriedAmmo = CarriedAmmo - LoadAmount;
	   //this line not affect this lesson but update the author_Char::Combat::CarriedAmmoMap
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] = CarriedAmmo;
	}
//you can move these 2 lines into Char::ReloadEnd() so that it wont update before that point, however the SetHUDAmmo trigger in Client already, so you have to move them all together :D :D. 
//So the final idea is to Call this HOSTING function in ReloadEnd() instead of in ServerInput_Reload()
//So because it is called in the server, so to compensate we add 'HasAuthority()' if call from there! 
// So you must understand that once we add it for Do_Action, the clients wont pass it and wont excecute it, so you must rely on OnRep (if any)
	
	CheckAndSetHUD_CarriedAmmo();

	//still need to help HUDAmmo in the server update to, I choose to do it here too, rather than outside:
	EquippedWeapon->CheckAndSetHUD_Ammo();
}

//For PlayReloadAnimation ~>Shotgun section, we call this instead:
//if it reach this function for first time, it must pass the condition < MagCapacity I guess
//so for now I dont check it full when I start to Load :D :D
//TO MAKE it consistent, I will create 'ReloadOneShell()' and wrap around this function and then check HasAuthorioty() from there too - simply make a copy of 'ReloadEnd()' and adjust it!
void UCombatComponent::UpdateHUD_CarriedAmmo_SpecializedForShotgun()
{
//PART1: almost the same UpdateHUD_CarriedAmmo()
	if (EquippedWeapon == nullptr) return;

	//this time it will much more easier LOL:
	int32 LoadAmount = 1;

	//Change Ammo here will trigger OnRep_Ammo ->SetHUDAmmo work automatically in clients!
	EquippedWeapon->SetAmmo(EquippedWeapon->GetAmmo() + LoadAmount);

	//Stephen swap order of them for sematic reason, but I prefer this, changing CarriedAmmo here ill trigger OnRep_CarriedAmmo  ->SetHUDCarriedAmmo work automatically in clients!
	CarriedAmmo = CarriedAmmo - LoadAmount;
	//this line not affect this lesson but update the author_Char::Combat::CarriedAmmoMap
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] = CarriedAmmo;
	}
	//you can move these 2 lines into Char::ReloadEnd() so that it wont update before that point, however the SetHUDAmmo trigger in Client already, so you have to move them all together :D :D. 
	//So the final idea is to Call this HOSTING function in ReloadEnd() instead of in ServerInput_Reload()
	//So because it is called in the server, so to compensate we add 'HasAuthority()' if call from there! 
	// So you must understand that once we add it for Do_Action, the clients wont pass it and wont excecute it, so you must rely on OnRep (if any)

	CheckAndSetHUD_CarriedAmmo();

	//still need to help HUDAmmo in the server update to, I choose to do it here too, rather than outside:
	EquippedWeapon->CheckAndSetHUD_Ammo();

//PART2: check if it full or out of CarriedAmmo and jump directly to sub section 'Shotgun End':
	//this only jump in the server within ReloadOneAmmo(), hence also check and call it in Weapon::OnRep_Ammo() is perfect! but from there we could only check 'EquippedWeapon->IsFull()' ? Well it has 'Ownercharacter' you can use :D 
	if (EquippedWeapon->IsFull() || CarriedAmmo == 0)
	{
		CharacterState = ECharacterState::ECS_Unoccupied; //moved from LoadOneAmmo(); so not any other React can intterupt it after first reload
		if (Character) Character->JumpToShotgunEndSection();//helper from Character
	}
}

void UCombatComponent::UpdateHUD_ThrowGrenade()
{
	if (ThrowGrenade <= 0) return;
	
	ThrowGrenade--;

	CheckAndSetHUD_ThrowGrenade();
}


//It first change when Equipped, this is auto-triggered
void UCombatComponent::OnRep_CarriedAmmo()
{
	CheckAndSetHUD_CarriedAmmo();
}

void UCombatComponent::CheckAndSetHUD_CarriedAmmo()
{
	if (Character == nullptr) return;
	BlasterPlayerController = BlasterPlayerController == nullptr ? Character->GetController<ABlasterPlayerController>() : BlasterPlayerController;

	if (BlasterPlayerController) BlasterPlayerController->SetHUDCarriedAmmo(CarriedAmmo);
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartCarriedAmmo_AR);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartCarriedAmmo_Rocket);

	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartCarriedAmmo_Pistol);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SMG, StartCarriedAmmo_SMG);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartCarriedAmmo_Shotgun);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartCarriedAmmo_SniperRifle);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartCarriedAmmo_GrenadeLauncher);
}

void UCombatComponent::OnRep_ThrowGrenade()
{
	CheckAndSetHUD_ThrowGrenade();
}

void UCombatComponent::CheckAndSetHUD_ThrowGrenade()
{
	if (Character == nullptr) return;
	BlasterPlayerController = BlasterPlayerController == nullptr ? Character->GetController<ABlasterPlayerController>() : BlasterPlayerController;
	if (BlasterPlayerController) BlasterPlayerController->SetHUDThrowGrenade(ThrowGrenade);
}

void UCombatComponent::OnRep_CharacterState()
{
	if (Character == nullptr) return;
	switch (CharacterState)
	{
	case ECharacterState::ECS_Reloading :
		Character->PlayReloadMontage();
		break;
	case ECharacterState::ECS_Throwing :
		//the controlling device already played from Combat::Input_Throw for seeing immediate effect, so we dont want to double-play it (may cause side effect if any), so we exclude it: FIRST TIME
		if( !Character->IsLocallyControlled() ) Character->PlayThrowMontage();

		break;
	case ECharacterState::ECS_Unoccupied : 
		//if (bIsFiring)
		//{
		//	Input_Fire(bIsFiring); 
		//}
		break;
	}
}

void UCombatComponent::SetPOVForCamera(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;
	if (Character == nullptr || Character->GetCamera() == nullptr) return;

	if (bIsAiming)
	{
		CurrentPOV = FMath::FInterpTo(CurrentPOV, EquippedWeapon->GetPOV(), DeltaTime, EquippedWeapon->GetPOVInterpSpeed());
	}
	else
	{
		CurrentPOV = FMath::FInterpTo(CurrentPOV, DefaultPOV, DeltaTime, EquippedWeapon->GetPOVInterpSpeed());
	}

	Character->GetCamera()->SetFieldOfView(CurrentPOV);
} 

//Stephen call it SetHUDCrosshairs, this is to be put in Tick so we need to optimize it
void UCombatComponent::SetHUDPackageForHUD(float DeltaTime)
{
	if (Character == nullptr || Character->GetController() == nullptr ) return;

//DEMO-ready: can also use if (__ ==nullptr) __ = Cast<();

	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Character->GetController()) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(BlasterPlayerController->GetHUD()) : BlasterHUD;
	}

//DEMO-setup
	if (BlasterHUD == nullptr) return;

	FHUDPackage HUDPackage;

	if (EquippedWeapon) //theriocically this is enough
	{
		HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
		HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
		HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
		HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
		HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;

		//expanding and shinking 
		if (Character->GetCharacterMovement()->IsFalling()) AdditionalJumpFactor = FMath::FInterpTo(AdditionalJumpFactor, JumpFactorMax, DeltaTime, 2.25f); 
		else AdditionalJumpFactor = FMath::FInterpTo(AdditionalJumpFactor, 0, DeltaTime, 10.f); 

		//I dont interp [0->max] because I like the sudden effect when I shoot/fire :D :D 
		if (bIsFiring) AdditinalFireFactor = FireFactorMax;
		else AdditinalFireFactor = FMath::FInterpTo(AdditinalFireFactor, 0, DeltaTime, 5.f);

		// 0.65f or equal to the inital 0.5f below are the matter of preference
		if (bIsAiming)	SubtractiveAimFactor = FMath::FInterpTo(SubtractiveAimFactor, AimFactorMax , DeltaTime, 5.f);
		else SubtractiveAimFactor = FMath::FInterpTo(SubtractiveAimFactor, 0 , DeltaTime, 5.f);


		//Velocity is interloated behind the scene so no need FMath::FInterpTo
		HUDPackage.ExpandFactor = Character->GetVelocity().Size2D() / 600.f 
			+ 0.5f  //give it an inital expan of 0.5f (rather than let 0) , so that you aim, it can shink.
			+ AdditionalJumpFactor 
			+ AdditinalFireFactor
			- SubtractiveAimFactor; 
		HUDPackage.Color = CrosshairsColor;
	}
	else //but in case you lose your weapon even if you did have one (out of bullets+) 
	{
		HUDPackage.CrosshairsCenter = nullptr;
		HUDPackage.CrosshairsRight = nullptr;
		HUDPackage.CrosshairsLeft = nullptr;
		HUDPackage.CrosshairsTop = nullptr;
		HUDPackage.CrosshairsBottom = nullptr;
	}

	BlasterHUD->SetHUDPackage(HUDPackage);
}

void UCombatComponent::Input_Fire(bool InIsFiring)
{

	bIsFiring = InIsFiring;

	if (CanFire())
	{
		//set it back to false as a part of preventing we spam the fire button during WaitTime to reach Timer callback
		bCanFire = false;

		FHitResult HitResult;
		DoLineTrace_UnderCrosshairs(HitResult);

		ServerInput_Fire(InIsFiring, HitResult.ImpactPoint); //rather than member HitPoint

		Start_FireTimer(); //this is the right place to call .SetTimer (which will be recursive very soon)
	}

}
bool UCombatComponent::CanFire()
{
	//{
	//	//this is a part of stopping Gun can't stop firing:
	//	if (bCanFire == false) return;
	//	//extra condition about ammo: || EquippedWeapon->GetAmmo() <= 0 - no need because we check it above also
	//	if (
	//		(EquippedWeapon == nullptr || CharacterState != ECharacterState::ECS_Unoccupied || EquippedWeapon->GetAmmo() <= 0))
	//	{
	//		return;
	//	}
	//}

	return
	  ( bCanFire &&
		EquippedWeapon && EquippedWeapon->GetAmmo() > 0 &&
		//this make Fire can't interrupt any montage generally:
		CharacterState == ECharacterState::ECS_Unoccupied )
		||
	  ( bCanFire &&
		EquippedWeapon && EquippedWeapon && EquippedWeapon->GetAmmo() > 0 &&
		//this is an exception for Shotgun, even if it is Reloading:
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun  &&
		CharacterState == ECharacterState::ECS_Reloading
			);
}

void UCombatComponent::ServerInput_Fire_Implementation(bool InIsFiring, const FVector_NetQuantize& Target)
{
	////move up to serverRPC
	//bIsFiring = InIsFiring; //We can't remove it here as this is for replication perupose :)

	MulticastInput_Fire(InIsFiring, Target);

	//I decide to call it here, Hell no! if you call it here, Ammo will be subtracted even before the PlayFireMontage happen - which we're not even sure it will pass it next condition to be up there :D :D :
	//if (EquippedWeapon) EquippedWeapon->UpdateHUD_Ammo();
}

//when it reaches this TIRE it must pass CanFire() and have Ammo > 0, So we dont need to check it again I suppose:
void UCombatComponent::MulticastInput_Fire_Implementation(bool InIsFiring, const FVector_NetQuantize& Target)
{
	//note that because the machine to be called is different, so put this line here or in the HOSTING function 'could' make a difference generally lol:
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	//move up to serverRPC
	bIsFiring = InIsFiring; //We can't remove it here as this is for replication perupose :)

	//Option1 to fix Fire intterupt Reload back when timer reach:
	//when it reaches this TIRE it must pass TIRE1::CanFire() and have Ammo > 0, So we dont need to check it again I suppose:
	if (
		( bIsFiring && 
		//this make Fire can't interrupt any montage generally:
		  CharacterState == ECharacterState::ECS_Unoccupied)
		||
		( bIsFiring && 
		  //this is an exception for Shotgun, even if it is Reloading:
		  EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun &&
		  CharacterState == ECharacterState::ECS_Reloading) )
	{
		Character->PlayFireMontage();

		EquippedWeapon->Fire(Target); //instead of member HitTarget, now you can remove it!

		//it should be here:
		if (Character->HasAuthority()) EquippedWeapon->UpdateHUD_Ammo();

		/*
		Stepehen directly set it back to Unoccupied right in "Combat::MulticastInput_Fire"
		, this is the reason:
		(1) the fire montage is "TOO short"
		(2) "Combat::MulticastInput_Fire" is the ONLY play trigger the PlayFireMontage()
		=hell yeah!
		*/
		CharacterState = ECharacterState::ECS_Unoccupied;
	}
}

void UCombatComponent::Start_FireTimer()
{
	GetWorld()->GetTimerManager().SetTimer(TimeHandle, this, &ThisClass::FireTimer_Callback, FireDelay);
}

void UCombatComponent::FireTimer_Callback()
{
	//We must do this before (*), as we need it to be true in case we actually release the fire button
	bCanFire = true;

	if (!bIsFiring || !bIsAutomatic) return; //(*)

	//I move it up here, shouldn't put any code below a RECURSIVE code:
	//this is the best place to reload, Stephen said, as this reach after DelayTime and eveything has been settle correctly! it is true I didn't have side effect as I release the Fire button and lose one more button without playing FireSound lol

	//Option2 to fix Fire intterupt Reload back when timer reach:
	if(CharacterState == ECharacterState::ECS_Unoccupied) Input_Fire(bIsFiring);

	if (EquippedWeapon && EquippedWeapon->IsEmpty() && CarriedAmmo > 0)
	{
		Input_Reload();
	}
}

void UCombatComponent::Input_Fire_WithoutAssingmentLine()
{
	//if (bIsFiring == false) GetWorld()->GetTimerManager().ClearTimer(TimeHandle);

	//can factorize these in to Combat::Fire() to be used instead of Input_Fire itself in timer_callback
	//you may wan to factorize this block into CanFire(), but I say no, I let it be here
	{
		//news:
		//if (InIsFiring == false) return;
		//this is a part of stopping Gun can't stop firing:
		if (bCanFire == false) return;
		//extra condition about ammo:
		if (EquippedWeapon == nullptr || EquippedWeapon->GetAmmo() <= 0 || CharacterState != ECharacterState::ECS_Unoccupied) return;
	}

	//set it back to false as a part of preventing we spam the fire button during WaitTime to reach Timer callback
	bCanFire = false;

	FHitResult HitResult;
	DoLineTrace_UnderCrosshairs(HitResult);

	ServerInput_Fire(bIsFiring, HitResult.ImpactPoint); //rather than member HitPoint

	Start_FireTimer(); //this is the right place to call .SetTimer (which will be recursive very soon)
}

//Instead of doing it from AWeapon::Equip() we do it here, hence it should do all the stuff we usually did here
void UCombatComponent::Equip(AWeapon* InWeapon)
{
	if (InWeapon == nullptr || Character == nullptr) return;
	//I decide to not let it equip if we're currenting do whatever (say Reloading/Throwing)
	if (CharacterState != ECharacterState::ECS_Unoccupied) return;
    
	DropCurrentWeaponIfAny();

	//this is new weapon:
	EquippedWeapon = InWeapon;  // -->trigger OnRep_EquippedWeapon, why my OnRep_ didn't trigger?

	if (EquippedWeapon == nullptr) return;

	EquippedWeapon->PlayEquipSound(Character); //just for cosmetic

	//I move this on top with the hope that it is replicated before OnRep_WeaponState
	EquippedWeapon->SetOwner(Character);

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped) ;

	//this better after SetOwner:
	ExtractCarriedAmmoFromMap_UpdateHUDAmmos_ReloadIfEmpty();

	//these 2 lines are optional, so I will remove it anyway
	bIsAutomatic = EquippedWeapon->GetIsAutomatic();
	FireDelay = EquippedWeapon->GetFireDelay();

//factorize this into 'Ready Collision and physics setup before Attachment or Simulation' function:
	//Our SMG dont need these to locally similated, move it out fix the bug:
	EquippedWeapon->GetWeaponMesh()->SetSimulatePhysics(false); //TIRE1
	EquippedWeapon->GetWeaponMesh()->SetEnableGravity(false);   //TIRE3 - no need nor should you do this LOL

	if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SMG)
	{
		EquippedWeapon->GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	else
	{
		EquippedWeapon->GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	AttachEquippedWeaponToRightHandSocket();

//we want after we have a weapon on hand, we want Actor facing in the same direction as Camera!
	Character->GetCharacterMovement()->bOrientRotationToMovement = false; //at first it is true
	Character->bUseControllerRotationYaw = true; //at first it is false
}

//Pickup::Overlap trigger in server -->this Combat::PickupAmmo also trigger in the server first
void UCombatComponent::PickupAmmo(EWeaponType InWeaponType, uint32 InAmmoAmmount)
{
	//Increase Ammo: both of them are self-replicated if called in the server, yes currently is
	if (CarriedAmmoMap.Contains(InWeaponType))
	{
		//this only update the map (to be retrieved later) but doesn't in fact update the CarriedAmmo in case it match the current EquippedWeapon's Type at all:
		CarriedAmmoMap[InWeaponType] = FMath::Clamp(CarriedAmmoMap[InWeaponType] + InAmmoAmmount, 0, MaxCarriedAmmo);
		//This will actually change the Carried Ammo in worst case:
		if (EquippedWeapon && InWeaponType == EquippedWeapon->GetWeaponType())
		{
			CarriedAmmo = CarriedAmmoMap[InWeaponType]; //OnRep_CarriedAmmo will trigger for client part
		}
		//it check on current CarriedAmmo and update HUD, if the WeaponType's Pickup doesn't match WeaponType's EquippedWeapon then this wont cause any visual effect :D :D
		CheckAndSetHUD_CarriedAmmo(); //this for the server part ONLY
	}

	//If it matches the type of EquippedWeapon and EquippedWeapon::Ammo = 0 then auto-reload it;
	if (EquippedWeapon  && EquippedWeapon->IsEmpty() && InWeaponType == EquippedWeapon->GetWeaponType())
	{
		Input_Reload(); //this is self-handled, no matter where you call it
	}
}

//call this specialized function with ThrowEnd():
void UCombatComponent::AttachEquippedWeaponToRightHandSocket()
{
	if (EquippedWeapon == nullptr || Character == nullptr || Character->GetMesh() == nullptr) return;
	const USkeletalMeshSocket* RightHandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (RightHandSocket) RightHandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
}

//can call this function with PlayThrowMontage/Input_Throw, make sure it is called in server for replication:
void UCombatComponent::AttachEquippedWeaponToLeftHandSocket()
{
	if (EquippedWeapon == nullptr || Character == nullptr || Character->GetMesh() == nullptr) return;

	//Add "LeftHandSocket" for the rest of weapon:
	FName SocketName("LeftHandSocket");
	
	//Add a socket 'LeftHandSocket_Pistol' for Pistol and Pistol, SMG 
	if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol ||
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SMG)
	{
		SocketName = FName("LeftHandSocket_Pistol");
	}

	const USkeletalMeshSocket* LeftHandSocket = Character->GetMesh()->GetSocketByName(SocketName);
	if (LeftHandSocket) LeftHandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
}

void UCombatComponent::ExtractCarriedAmmoFromMap_UpdateHUDAmmos_ReloadIfEmpty()
{
	//you must update CarriedAmmo from the map first before you check on it LOL
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		//map[KEY] = value of that KEY, remeMber? 
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		//CarriedAmmo = *(CarriedAmmoMap.Find(EquippedWeapon->GetWeaponType())) ;
	}
	CheckAndSetHUD_CarriedAmmo();

	//checking if Ammo is 0 and CarriedAmmo to Auto-Reload it:

	//if (EquippedWeapon->GetAmmo() <= 0 && CarriedAmmo <= 0) return; //this line is stupid
	if (EquippedWeapon->GetAmmo() <= 0 && CarriedAmmo > 0)
	{
		Input_Reload(); //self-organized (i.e self replicated in some way)
	}
	//I decide to call it here, safe enough from SetOwner above LOL:
	EquippedWeapon->CheckAndSetHUD_Ammo(); //no need, Input_Reload is self-handled already? no in case it can't pass the if check, then this will be needed LOL
}

void UCombatComponent::DropCurrentWeaponIfAny()
{
	//News: drop current weapon (if any) before you pick a new one
	if (EquippedWeapon)
	{
		EquippedWeapon->Drop();
		//turn on CustomDepth back here or in AWeapon::Drop() = recommended!
	}
}

//this is to fix the owning client can't update these on itself (weird case, can't explain :D )
void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon == nullptr || EquippedWeapon->GetWeaponMesh() ==nullptr || Character == nullptr) return;

	EquippedWeapon->PlayEquipSound(Character);
	//the condition is optional, but since I know only that owning client have problem, so I only need to let this code run on that client LOL, hell yeah!

	//Note that I've been thinking about extra the code, setting WeaponState, Setting physics, but unfortunately WeaponState is not public member nor did I want to move it to public sesson to break my UNIVERSAL pattern :)
	//Hence the only choice1: is to follow stephen, focus on case=Equipped only
	//Choice2: create another exclusive setter SetWeaponStateOnly()

	////choice1:
	//EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped); //go and restrict SetWeaponState::case_Equipped that the local can't touch Sphere collision -->Go and add If(HasAuthority()), but you dont have to setup physics here.
	
	//choice2: 
	EquippedWeapon->SetWeaponState_Only(EWeaponState::EWS_Equipped); //without needing to break the GLOBAL pattern, but you will have to setup physics here

	//Our SMG dont need these to locally similated, move it out fix the bug:
	EquippedWeapon->GetWeaponMesh()->SetSimulatePhysics(false); //TIRE1
	EquippedWeapon->GetWeaponMesh()->SetEnableGravity(false);   //TIRE3 - no need nor should you do this LOL
	if (EquippedWeapon->GetWeaponType() != EWeaponType::EWT_SMG)
	{
		EquippedWeapon->GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SMG)
	{
		EquippedWeapon->GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	const USkeletalMeshSocket* RightHandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (RightHandSocket) RightHandSocket->AttachActor(EquippedWeapon, Character->GetMesh());

	//even if Attachment action is replicated we call it here to make sure it works


	if (Character->IsLocallyControlled())
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false; //at first it is true
		Character->bUseControllerRotationYaw = true; //at first it is false

	}

}


//currently this could only call in IsLocallyControlled() /autonompus proxy - hence 'if(IsLocallyControlled()' become redudant!
void UCombatComponent::SetIsAiming(bool InIsAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	if (Character->HasAuthority())
	{
		bIsAiming = InIsAiming;
		if (Character->GetCharacterMovement()) Character->GetCharacterMovement()->MaxWalkSpeed = InIsAiming ? AimWalkSpeed : MaxWalkSpeed_Backup;
	}
	else ServerSetIsAiming(InIsAiming);
	//option2: call BP_function(InIsAiming) here
	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		Character->ShowScopeWidget(InIsAiming);
	}
}

void UCombatComponent::ServerSetIsAiming_Implementation(bool InIsAiming) //REPLACE
{
	if (Character)
	{
		bIsAiming = InIsAiming;
		if (Character->GetCharacterMovement()) Character->GetCharacterMovement()->MaxWalkSpeed = InIsAiming ? AimWalkSpeed : MaxWalkSpeed_Backup;
	}
}

FVector UCombatComponent::DoLineTrace_UnderCrosshairs(FHitResult& LineHitResult)
{
	if (GetWorld() == nullptr || GEngine == nullptr) return FVector{};
	
	FVector2D ViewportSize;
	if(GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	FVector2D ViewportCenterLocation = FVector2D(ViewportSize.X / 2.f, ViewportSize.Y / 2.f); //Stephen call it "CrosshairsLocation", this is relative to Screen 2D coordinates 

    //WAY1: me, not sure it is exactly like way2 I test them both, either of them are working the same, in multiplayer test, each device has its own trace line and draw its own sphere, so one see the other no matter WAY1 or WAY2 (which is good)
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	//WAY2: stephen, he said player index 0 here will be always the controller controlling the controlled pawn in the local device, even if it is multiplayer game! so index is also relative huh? :D :D
	APlayerController* PlayerController1 = UGameplayStatics::GetPlayerController(this, 0);
	
	FVector WorldLocation;
	FVector WorldDirection; //it will be modified and normalized 

	bool bIsSuccessful = 
		UGameplayStatics::DeprojectScreenToWorld(
			PlayerController1, //test both
			ViewportCenterLocation,
			WorldLocation,
			WorldDirection
	    );
	
	if (!bIsSuccessful) return FVector{}; //this is not the reason that why a device only see its own DebugSphere

	FVector Start = WorldLocation; //Start is Exactly Camera location, I didn't realize it LOL

	//Stephen: (Character->GetActorLocation() - Start).Size() + StartTraceOffset
	float StartOffset = (Character->GetActorLocation() - Start).Size2D() / abs(FMath::Cos(Character->GetAO_Pitch() * 3.14f / 180.f ) ) + ExtraStartOffset;

	Start += WorldDirection * StartOffset;

	FVector End = Start + WorldDirection * 80000; //Direction is current a vector unit with length=1 only, so yeah!

	//DrawDebugSphere(GetWorld(), Start, SphereRadius, 12.f, FColor::Green, bDrawConsistentLine);

	//GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Blue, FString::Printf(TEXT("TraceOffset: %f , Pitch: %f , Cos: %f "), StartOffset, Character->GetAO_Pitch(), FMath::Cos(Character->GetAO_Pitch() * 3.14f / 180.f)));	
	
	GetWorld()->LineTraceSingleByChannel(
		LineHitResult,
		Start,
		End,
		ECollisionChannel::ECC_Visibility //almost all things block visibility by default
	);

	if (LineHitResult.bBlockingHit == false)
	{
		LineHitResult.ImpactPoint = End;
	}

	HitTarget = LineHitResult.ImpactPoint;

	//HitTarget = LineHitResult.ImpactPoint; //ImpactPoint now can be relied on in either case after the if!

	DrawDebugSphere(GetWorld(), LineHitResult.ImpactPoint, SphereRadius, 12.f, FColor::Red, bDrawConsistentLine);

	return End;
}

void UCombatComponent::ReloadEnd()
{
	if (Character == nullptr) return;
	//this may cause "LEGENDARYcase", if so, simpl remove 'HasAuthority()' will overcome it LOL = no it's not!
		//because the inner code is self-replicated, so add HasAuthority is optional. if you wan the server first, then add HasAuthority, if you want all copies run over, then remove it (for cosmetic action) 
	if (Character->HasAuthority())
	{
		CharacterState = ECharacterState::ECS_Unoccupied;
		//this make my clients can't stop firing, it is because of 'late replication', currently bIsFiring isn't setup very clean in GOLDEN2, it is like a shit LOL, nor should we replicateit LOL
		//continue to fire if bFiring is still true (still holding the Fire key):
		// FOR NOW, I comment it out:
		// the code inside the is meant to be run in the server only, you can't let it run in the client nor should you also call this in OnRep!
	}

	//it should originate from the Owning client , NOT the server, this fix!!!:
	if(Character->IsLocallyControlled())
	{
		if (bIsFiring)
		{
			//you may be tempted to pass in "true", but it can still be changed right?
			Input_Fire(bIsFiring);
		}
	}
	//If you want..., you call this either here or in Combat::ServerInput_Reload, not both:
	if (Character->HasAuthority()) UpdateHUD_CarriedAmmo();
}

void UCombatComponent::ReloadOneAmmo()
{
	if (Character == nullptr) return;

	//Not sure if this is still appropriate? this can be called 4 times lol, it will even set to Unoccupied after first load, and any other Montage can intterupt it (including Fire(wanted), React(depdens): 
	//if you dont like this behaviour simply move it into if(CarriedAmmo =0 || IsFull()) together with JumpToShotgunEnd() in the UpdateHUD_CarriedAmmo_SpecializedForShotgun() bellow! and its hosting function is currently having HasAuthority() here anyway LOL!
		//if (Character->HasAuthority())
		//{
		//	CharacterState = ECharacterState::ECS_Unoccupied;
		//}

	//Optional for shotgun if you want it to continue to fire if Fire button is still press after reload at least one ammo!
	if (Character->IsLocallyControlled())
	{
		if (bIsFiring) Input_Fire(bIsFiring);
	}

	//If you want..., you call this either here or in Combat::ServerInput_Reload, not both:
	if (Character->HasAuthority())
	{
		UpdateHUD_CarriedAmmo_SpecializedForShotgun();
	}
}

//to be called when in ThrowEnd AnimNotify , no condition mean call in all devices (if previous end up PlayThrowAnimation in all devices)
void UCombatComponent::ThrowEnd()
{
	CharacterState = ECharacterState::ECS_Unoccupied;
	//Snap the weapon back to where it begin:
	AttachEquippedWeaponToRightHandSocket();
}

void UCombatComponent::ShowGrenadeMesh()
{
	if (Character) Character->ShowGrenadeMesh();
}

void UCombatComponent::HideGrenadeMesh_SpawnActualProjectileGrenade()
{
	if (Character == nullptr) return;
//cosmetic part: do it for all devices
	Character->HideGrenadeMesh();

//main part: Spawn Projectile_ThrowGrenade: GrenadeSocket_inChar -> DoLineTracce_cross::HitTarget
	//step1: need HitTarget from Controlling device:
	if (Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		DoLineTrace_UnderCrosshairs(HitResult); //if hit, HitResult.Impoint = whatever Hit, if not we did set HitResult.Impoint = End; remember , hell yeah!

		//step2: propogate this value to the server -> clients:
		ServerSpawnGrenade(HitResult.ImpactPoint);
	}

}

//Spawn the AProjectile_Grenade here to be replicated across all devices:
//go and copy the code in AProjectilWeapon::Fire code and adapt: GrenadeSocket_InChar -> HitTarget:
void UCombatComponent::ServerSpawnGrenade_Implementation(const FVector& Target)
{
	if (Character == nullptr || Character->GetMesh() ==nullptr ||ProjectileClass == nullptr)  return;
	//now starting location is socket in Char:
	//you can in fact Character->GetGrenadeMesh()->GetComponentLocation() as well LOL
	FTransform GrenadeSocketTransform_InChar = Character->GetMesh()->GetSocketTransform(FName("GrenadeSocket"));

	FVector SpawnLocation = GrenadeSocketTransform_InChar.GetLocation();

	FVector FacingDirection = (Target - SpawnLocation);

	FRotator SpawnRotation = FacingDirection.Rotation(); //accept ROLL = 0 -> YAW & PTICH

	FActorSpawnParameters SpawnParams;

	//now we dont have weapon, so leave it be nullptr (rather then set it be Character causing inconsistency with other AProjectile_X spawned by Weapon)
	SpawnParams.Owner = nullptr; 

	//UActorComponent::GetOwner() will be naturally its hosting actor, according to the last test remember, however I will simply enter 'Character':
	SpawnParams.Instigator = Cast<APawn>(Character); //GetOwner() will also work

	GetWorld()->SpawnActor<AProjectile>(
		ProjectileClass,
		SpawnLocation, SpawnRotation, SpawnParams
	);
}

////I separate it here to avoid, replication late:
//void UCombatComponent::EndReload_ContinueFiringIf()
//{
//	//this is INCCORECT, the ORIGIN of Input_Callback is from the OWNING client, not HasAuthority()
//	//if (Character && Character->HasAuthority()) 
//	
//	//this is perfect, that stop the issue can't stop firing after reloading!
//	if (Character  && Character->IsLocallyControlled() )
//	{
//		if (bIsFiring)
//		{
//			//you may be tempted to pass in "true", but it can still be changed right?
//			Start_FireTimer();
//		}
//	}
//}
//
//







