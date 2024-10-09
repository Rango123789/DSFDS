// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterComponents/CombatComponent.h"
#include "Weapons/Weapon.h"
#include "Characters/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h" //for DoLineTrace
#include "Kismet/GameplayStatics.h" //for DeprojectScreenToWorld

UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true; //if you tick later, then turn it on
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FHitResult HitResult;
	DoLineTrace_UnderCrosshairs(HitResult);
}
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	//assign its value here (or BeginPlay()), so surely all character instances has it
	MaxWalkSpeed_Backup = Character->GetCharacterMovement()->MaxWalkSpeed;
	AimWalkSpeed = 300.f;
}

void UCombatComponent::Fire(bool InIsFiring)
{
	ServerFire(InIsFiring);
}


void UCombatComponent::ServerFire_Implementation(bool InIsFiring)
{
	MulticastFire(InIsFiring);
}

void UCombatComponent::MulticastFire_Implementation(bool InIsFiring)
{
	//note that because the machine to be called is different, so put this line here or in the HOSTING function 'could' make a difference generally lol:
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	bIsFiring = InIsFiring;

	if (bIsFiring)
	{
		Character->PlayFireMontage();
		EquippedWeapon->PlayFireAnimation();
	}
	//else Character->StopFireMontage();
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

void UCombatComponent::DoLineTrace_UnderCrosshairs(FHitResult& LineHitResult)
{
	//out goal is to trace from Center of Screen -> GetBaseAimRotation()/ControlRotation/Camera's direction? well no need we have other better way!
	FVector2D ViewportSize;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	FVector2D ViewportCenterLocation = FVector2D(ViewportSize.X / 2.f, ViewportSize.Y / 2.f); //Stephen call it "CrosshairsLocation", this is relative to Screen 2D coordinates 

    //WAY1: me, not sure it is exactly like way2
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	//WAY2: stephen, he said player index 0 here will be always the controller controlling the controlled pawn in the local device, even if it is multiplayer game! so index is also relative huh? :D :D
	APlayerController* PlayerController1 = UGameplayStatics::GetPlayerController(this, 0);
	
	FVector WorldLocation;
	FVector WorldDirection; //it will be modified and normalized 

	bool bIsSuccessful = 
		UGameplayStatics::DeprojectScreenToWorld(
			PlayerController1, 
			ViewportCenterLocation,
			WorldLocation,
			WorldDirection
	    );
	
	if (!bIsSuccessful) return;

	FVector Start = WorldLocation;
	FVector End = Start + WorldDirection * 80000; //Direction is current a vector unit with length=1 only, so yeah!
	GetWorld()->LineTraceSingleByChannel(
		LineHitResult,
		Start,
		End,
		ECollisionChannel::ECC_Visibility //almost all things block visibility by default
	);

	if (LineHitResult.bBlockingHit == false) LineHitResult.ImpactPoint = End;
	
	DrawDebugSphere(GetWorld(), LineHitResult.ImpactPoint, 5.f, 12.f, FColor::Red, false);
}










