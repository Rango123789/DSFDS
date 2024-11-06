// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterComponents/CombatComponent.h"
#include "Weapons/Weapon.h"
#include "Characters/BlasterCharacter.h"
#include "PlayerController/BlasterPlayerController.h"
#include "HUD/BlasterHUD.h"

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

	//TimerDelegate.BindUFunction(this, FName("FireTimer_Callback")); 
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
	//DOREPLIFETIME(UCombatComponent, bIsFiring); //just added, BUT didn't affect anyway
	//DOREPLIFETIME(UCombatComponent, CarriedAmmo); //will still work but redudant for non-owning clients to receive it - they dont need it in this game
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly); //only owning client need to receive it
	DOREPLIFETIME(UCombatComponent, CharacterState);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	//FString OwnerName;
	//if(GetOwner()) GetOwner()->GetName(OwnerName);
	//UE_LOG(LogTemp, Warning, TEXT("Owner of CombatComp: %s"), *OwnerName);

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
	if(CarriedAmmo > 0 && CharacterState == ECharacterState::ECS_Unoccupied ) ServerInput_Reload();
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


}

void UCombatComponent::OnRep_CharacterState()
{
	if (Character == nullptr) return;
	switch (CharacterState)
	{
	case ECharacterState::ECS_Reloading:
		Character->PlayReloadMontage();

		break;
	case ECharacterState::ECS_Unoccupied: 
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
	//News: to fix can't stop firing, as when you realease the key, bIsFiring = false before you trigger the .SetTimer below!

	bIsFiring = InIsFiring;

	//no matter what copy is, the EquippedWeapon::Ammo are always the same! but this is NOT the best place
		//if (EquippedWeapon == nullptr) return;
	   //if (EquippedWeapon->GetAmmo() <= 0 && CarriedAmmo <= 0) return;
		//if (EquippedWeapon->GetAmmo() <= 0 && CarriedAmmo > 0)
		//{
		//	Input_Reload();
		//}

//can factorize these in to Combat::Fire() to be used instead of Input_Fire itself in timer_callback
	//you may wan to factorize this block into CanFire(), but I say no, I let it be here
	{
		//this is a part of stopping Gun can't stop firing:
		if (bCanFire == false) return;
		//extra condition about ammo: || EquippedWeapon->GetAmmo() <= 0 - no need because we check it above also
		if (EquippedWeapon == nullptr || CharacterState !=ECharacterState::ECS_Unoccupied || EquippedWeapon->GetAmmo() <= 0) return;
	}

	//set it back to false as a part of preventing we spam the fire button during WaitTime to reach Timer callback
	bCanFire = false;

	FHitResult HitResult;
	DoLineTrace_UnderCrosshairs(HitResult);

	ServerInput_Fire(InIsFiring, HitResult.ImpactPoint); //rather than member HitPoint

	Start_FireTimer(); //this is the right place to call .SetTimer (which will be recursive very soon)
}

void UCombatComponent::ServerInput_Fire_Implementation(bool InIsFiring, const FVector_NetQuantize& Target)
{
	////move up to serverRPC
	//bIsFiring = InIsFiring; //We can't remove it here as this is for replication perupose :)

	MulticastInput_Fire(InIsFiring, Target);

	//I decide to call it here, safe enough from SetOwner above LOL:
	if (EquippedWeapon) EquippedWeapon->UpdateHUD_Ammo();
}

void UCombatComponent::MulticastInput_Fire_Implementation(bool InIsFiring, const FVector_NetQuantize& Target)
{
	//note that because the machine to be called is different, so put this line here or in the HOSTING function 'could' make a difference generally lol:
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	//move up to serverRPC
	bIsFiring = InIsFiring; //We can't remove it here as this is for replication perupose :)

	//Option1 to fix Fire intterupt Reload back when timer reach:
	if (bIsFiring && CharacterState == ECharacterState::ECS_Unoccupied)
	{
		Character->PlayFireMontage();

		EquippedWeapon->Fire(Target); //instead of member HitTarget, now you can remove it!
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

	//Option2 to fix Fire intterupt Reload back when timer reach:
	if(CharacterState == ECharacterState::ECS_Unoccupied) Input_Fire(bIsFiring);

	//this is the best place to reload, Stephen said, as this reach after DelayTime and eveything has been settle correctly! it is true I didn't have side effect as I release the Fire button and lose one more button without playing FireSound lol
	if (EquippedWeapon == nullptr) return;
	if (EquippedWeapon->GetAmmo() <= 0 && CarriedAmmo <= 0) return;
	if (EquippedWeapon->GetAmmo() <= 0 && CarriedAmmo > 0)
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
    
	//News: drop current weapon (if any) before you pick a new one
	if (EquippedWeapon) EquippedWeapon->Drop();

	EquippedWeapon = InWeapon;  // -->trigger OnRep_EquippedWeapon, why my OnRep_ didn't trigger?

	EquippedWeapon->PlayEquipSound(Character); //just for cosmetic

	//checking if Ammo is 0 and CarriedAmmo to Auto-Reload it:
	if (EquippedWeapon == nullptr) return;
	if (EquippedWeapon->GetAmmo() <= 0 && CarriedAmmo <= 0) return;
	if (EquippedWeapon->GetAmmo() <= 0 && CarriedAmmo > 0)
	{
		Input_Reload(); //self-organized (i.e self replicated in some way)
	}

	//I move this on top with the hope that it is replicated before OnRep_WeaponState
	EquippedWeapon->SetOwner(Character);

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped) ;

	//these 2 lines are optional, so I will remove it anyway
	bIsAutomatic = EquippedWeapon->GetIsAutomatic();
	FireDelay = EquippedWeapon->GetFireDelay();

	//I just add these lines to make sure it is off before pick up, THIS IS FOR THE SERVER:
	EquippedWeapon->GetWeaponMesh()->SetSimulatePhysics(false); //TIRE1
	EquippedWeapon->GetWeaponMesh()->SetEnableGravity(false);   //TIRE3 - no need nor should you do this LOL
	EquippedWeapon->GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	const USkeletalMeshSocket* RightHandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));

	if(RightHandSocket) RightHandSocket->AttachActor(EquippedWeapon, Character->GetMesh());

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()) )
	{
		//map[KEY] = value of that KEY, remeMber? 
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		//CarriedAmmo = *(CarriedAmmoMap.Find(EquippedWeapon->GetWeaponType())) ;
	}
	CheckAndSetHUD_CarriedAmmo();

	//I decide to call it here, safe enough from SetOwner above LOL:
	EquippedWeapon->CheckAndSetHUD_Ammo();
	
	//we want after we have a weapon on hand, we want Actor facing in the same direction as Camera!
	Character->GetCharacterMovement()->bOrientRotationToMovement = false; //at first it is true
	Character->bUseControllerRotationYaw = true; //at first it is false
}

//this is to fix the owning client can't update these on itself (weird case, can't explain :D )
void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon == nullptr || EquippedWeapon->GetWeaponMesh() ==nullptr || Character == nullptr) return;

	UE_LOG(LogTemp, Warning, TEXT("OnRep_EquippedWeapon trigger"))

	EquippedWeapon->PlayEquipSound(Character);
	//the condition is optional, but since I know only that owning client have problem, so I only need to let this code run on that client LOL, hell yeah!

	//Note that I've been thinking about extra the code, setting WeaponState, Setting physics, but unfortunately WeaponState is not public member nor did I want to move it to public sesson to break my UNIVERSAL pattern :)
	//Hence the only choice1: is to follow stephen, focus on case=Equipped only
	//Choice2: create another exclusive setter SetWeaponStateOnly()

	////choice1:
	//EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped); //go and restrict SetWeaponState::case_Equipped that the local can't touch Sphere collision -->Go and add If(HasAuthority()), but you dont have to setup physics here.
	
	//choice2: 
	EquippedWeapon->SetWeaponState_Only(EWeaponState::EWS_Equipped); //without needing to break the GLOBAL pattern, but you will have to setup physics here

	EquippedWeapon->GetWeaponMesh()->SetSimulatePhysics(false); //TIRE1
	EquippedWeapon->GetWeaponMesh()->SetEnableGravity(false);   //TIRE3 - no need nor should you do this LOL
	EquippedWeapon->GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//even if Attachment action is replicated we call it here to make sure it works
	const USkeletalMeshSocket* RightHandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (RightHandSocket) RightHandSocket->AttachActor(EquippedWeapon, Character->GetMesh());


	if (Character->IsLocallyControlled())
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false; //at first it is true
		Character->bUseControllerRotationYaw = true; //at first it is false

	}

}

void UCombatComponent::SetIsAiming(bool InIsAiming)
{
	if (Character->HasAuthority())
	{
		bIsAiming = InIsAiming;
		if (Character->GetCharacterMovement()) Character->GetCharacterMovement()->MaxWalkSpeed = InIsAiming ? AimWalkSpeed : MaxWalkSpeed_Backup;
	}
	else
		ServerSetIsAiming(InIsAiming);
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

void UCombatComponent::EndReload()
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

//I separate it here to avoid, replication late:
void UCombatComponent::EndReload_ContinueFiringIf()
{
	//this is INCCORECT, the ORIGIN of Input_Callback is from the OWNING client, not HasAuthority()
	//if (Character && Character->HasAuthority()) 
	
	//this is perfect, that stop the issue can't stop firing after reloading!
	if (Character  && Character->IsLocallyControlled() )
	{
		if (bIsFiring)
		{
			//you may be tempted to pass in "true", but it can still be changed right?
			Start_FireTimer();
		}
	}
}









