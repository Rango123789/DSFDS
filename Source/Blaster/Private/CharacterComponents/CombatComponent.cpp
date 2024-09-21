// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterComponents/CombatComponent.h"
#include "Weapons/Weapon.h"
#include "Characters/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"

UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false; //if you tick later, then turn it on

	// ...
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

//Instead of doing it from AWeapon::Equip() we do it here, hence it should do all the stuff we usually did here
void UCombatComponent::EquipWeapon(AWeapon* InWeapon)
{
	if (InWeapon == nullptr || Character == nullptr) return;

	//this is not replicated unless you mark it 'Replicated' here
	EquippedWeapon = InWeapon;

	//this function change "custom type's value", no way it is replicated
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped) ;

	//I just add it, COLLISION wont be replicated as experience from lass lesson; the only place can have Collision is the server, so you can simply turn it OFF from the server is ENOUGH, no matter via which controlled char you turn it OFF, there is ONLY one copy of weapon in each device, so yeah! hence this code will stay here to be called via INPUT-callback with 'HasAuthority()' condition I guess. We'll see after we make the clients pick the weapon by the other way around. Away I try this for the case without HasAuthority, so it should work after then too.
	// This problem is solved! now focusing on other statements around that may not be replicated	
	
	//EquippedWeapon->GetSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//this is replicated from server to clients via UE5 OnRep_
	const USkeletalMeshSocket* RightHandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if(RightHandSocket) RightHandSocket->AttachActor(EquippedWeapon, Character->GetMesh());

	//I'm checking this, yes if it is set from the server, it can be replicated to clients
	//also you have OnRep_SetOwner() to use when the need arise!
	EquippedWeapon->SetOwner(Character);

	//It is not replicated, however you dont need to turn it OFF for the other devices that ALREADY dont see it (from the last lesson)
	// BUT what if other devices themself overlap with the picked weapon? hell no, it shows the widget LOL, hence its collision should be turn off from the server as well (clients is disabled from the beginning)
	//What if I want it to replicated? UNLESS I check UWidgetComponent::"component replicate" from BP_Character GLOBALLY? and register it from AWeapon locally? - i believe i tried it and it worked! but it is not the way to go here )
	
	//EquippedWeapon->ShowPickupWidget(false);
}

