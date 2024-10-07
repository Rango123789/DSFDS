// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterComponents/CombatComponent.h"
#include "Weapons/Weapon.h"
#include "Characters/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false; //if you tick later, then turn it on

}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	//assign its value here (or BeginPlay()), so surely all character instances has it
	MaxWalkSpeed_Backup = Character->GetCharacterMovement()->MaxWalkSpeed;
	AimWalkSpeed = 300.f;
}



void UCombatComponent::SetIsFiring(bool InIsFiring)
{
	bIsFiring = InIsFiring;
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

//Instead of doing it from AWeapon::Equip() we do it here, hence it should do all the stuff we usually did here
void UCombatComponent::Equip(AWeapon* InWeapon)
{
	if (InWeapon == nullptr || Character == nullptr) return;

	EquippedWeapon = InWeapon;

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped) ;

	const USkeletalMeshSocket* RightHandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if(RightHandSocket) RightHandSocket->AttachActor(EquippedWeapon, Character->GetMesh());

	EquippedWeapon->SetOwner(Character);

	//EquippedWeapon->GetSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//EquippedWeapon->ShowPickupWidget(false);
	
	//we want after we have a weapon on hand, we want Actor facing in the same direction as Camera!
	Character->GetCharacterMovement()->bOrientRotationToMovement = false; //at first it is true
	Character->bUseControllerRotationYaw = true; //at first it is false
}

//this is to fix the owning client can't update these on itself (weird case, can't explain :D )
void UCombatComponent::OnRep_EquippedWeapon()
{
	//this OnRep_ is called whenever it changes, so you dont want to DoAction when it is changed to NOT valid value but null :D :D
	//it is easy to forget this LOL, but yeah :D 
	if (EquippedWeapon == nullptr || Character == nullptr) return;

	//the condition is optional, but since I know only that owning client have problem, so I only need to let this code run on that client LOL, hell yeah!

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