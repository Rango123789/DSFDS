// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterComponents/CombatComponent.h"
#include "Weapons/Weapon.h"
#include "Characters/BlasterCharacter.h"
#include "HUD/BlasterHUD.h"

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

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	//assign its value here (or BeginPlay()), so surely all character instances has it
	MaxWalkSpeed_Backup = Character->GetCharacterMovement()->MaxWalkSpeed;
	AimWalkSpeed = 300.f;
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SetHUDPackageForHUD(DeltaTime);
}

//Stephen call it SetHUDCrosshairs, this is to be put in Tick so we need to optimize it
void UCombatComponent::SetHUDPackageForHUD(float DeltaTime)
{
//DEMO-ready:
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

	ABlasterHUD* BlasterHUD = Cast<ABlasterHUD>(PlayerController->GetHUD());

//DEMO-setup
	FHUDPackage HUDPackage;

	HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
	HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
	HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
	HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
	HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;

	BlasterHUD->SetHUDPackage(HUDPackage);

}




void UCombatComponent::Input_Fire(bool InIsFiring)
{
	FHitResult HitResult;
	DoLineTrace_UnderCrosshairs(HitResult);

	ServerInput_Fire(InIsFiring, HitResult.ImpactPoint); //rather than member HitPoint
}

void UCombatComponent::ServerInput_Fire_Implementation(bool InIsFiring, const FVector_NetQuantize& Target)
{
	MulticastInput_Fire(InIsFiring, Target);
}

void UCombatComponent::MulticastInput_Fire_Implementation(bool InIsFiring, const FVector_NetQuantize& Target)
{
	//note that because the machine to be called is different, so put this line here or in the HOSTING function 'could' make a difference generally lol:
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	bIsFiring = InIsFiring;

	if (bIsFiring)
	{
		Character->PlayFireMontage();

		EquippedWeapon->Fire(Target); //instead of member HitTarget, now you can remove it!

	}
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
	if (GetWorld() == nullptr || GEngine == nullptr) return;
	
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
	
	if (!bIsSuccessful) return; //this is not the reason that why a device only see its own DebugSphere

	FVector Start = WorldLocation;
	FVector End = Start + WorldDirection * 80000; //Direction is current a vector unit with length=1 only, so yeah!
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
}











