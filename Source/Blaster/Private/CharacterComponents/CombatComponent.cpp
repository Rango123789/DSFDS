// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterComponents/CombatComponent.h"
#include "Weapons/Weapon.h"
#include "Characters/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"

UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

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

	//state and assignment:
	EquippedWeapon = InWeapon;

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped) ;

	//attachment: you can always use "AActor::AttachToComponent( Parent , AttachmentRules , SocketName = NONE)
	//you must go the SKM and its skeleton and add the same socket name for it to work LOL.
	const USkeletalMeshSocket* RightHandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if(RightHandSocket) RightHandSocket->AttachActor(EquippedWeapon, Character->GetMesh());

	//set owner: I like to set owner after the attachment
	EquippedWeapon->SetOwner(Character);

	//cosmetic:
	EquippedWeapon->ShowPickupWidget(false);
}

