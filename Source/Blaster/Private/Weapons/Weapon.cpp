// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Characters/BlasterCharacter.h"
#include "HUD/Overhead_UserWidget.h" //for testing
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//We want replicate this Weapon accross all clients from the server:
	bReplicates = true; //NEW1

	//Setup WeaponMesh:
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("WeaponMesh");
	RootComponent = WeaponMesh;

		//collision setup: default beviour: block all except pawn, INACTIVE by default - we enable it later
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block) ;
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); //we make it ACTIVE when appropriate
	
	//Setup Sphere:
	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	Sphere->SetupAttachment(RootComponent);

		//collision setup: default behaviour: it should be overlap at least with Pawn, but for now I follow STEPHEN  and set it ignore all channels, set it INACTIVE  
	Sphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); //we only enable in BeginPlay() for authority version, but it will be replicated it back to update the client versions via bReplicated=true above, so dont worry :D :D

	//Setup WidgetComponent:
	Pickup_WidgetComponent = CreateDefaultSubobject<UWidgetComponent>("Pickup_WidgetComponent");
	Pickup_WidgetComponent->SetupAttachment(RootComponent);
	Pickup_WidgetComponent->SetVisibility(false); //indirectly not allow sub-widget to not show until overlap
	//WeaponMesh->SetVisibility(true);
	//Pickup_WidgetComponent->SetVisibility(true);
	//SetActorHiddenInGame(false);  

		//for testing
	Overhead_WidgetComponent = CreateDefaultSubobject<UWidgetComponent>("Overhead_WidgetComponent");
	Overhead_WidgetComponent->SetupAttachment(RootComponent);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeapon, WeaponState);
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	//A particular instance of this weapon will have n copies in n machines
	//but with the if check, the block inside it will only run for the copy that is in the server machine
	// i.e n-1 copies of it will pass the block without running it.
	//However if this weapon has bReplicates = true, its properties/member values will be replicated and updated back to the rest n-1 copies - sound funny but this is the CLASSICAL way that multiplayer game work :) 

	//if (GetLocalRole() == ENetRole::ROLE_Authority) // <=> if (HasAuthority()) 
	//if (GetLocalRole() == ENetRole::ROLE_SimulatedProxy|| GetRemoteRole() == ENetRole::ROLE_SimulatedProxy)
	//if (GetLocalRole() == ENetRole::ROLE_AutonomousProxy || GetRemoteRole() ==ENetRole::ROLE_AutonomousProxy )

	ENetRole Local = GetLocalRole();
	ENetRole Remote = GetRemoteRole();
	//if (Local == ENetRole::ROLE_AutonomousProxy || Remote == ENetRole::ROLE_AutonomousProxy)
	if (HasAuthority())
	{
		if (Sphere) 
		{
			Sphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
			Sphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); //what the heck? I thought it is Query only?
			//doing it here you only bind callbacks for the weapon copy in the server:
			Sphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
			Sphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);
		}
	}

	//for testing:
	if (Overhead_WidgetComponent)
	{
		UOverhead_UserWidget* Overhead_UserWidget = Cast<UOverhead_UserWidget>(Overhead_WidgetComponent->GetUserWidgetObject());
		if (Overhead_UserWidget) Overhead_UserWidget->ShowPlayerNetRole(this);
	}
}


// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//USceneComponent::SetVisibility() - TIRE1 , the visibility of the TextBlock is still true by default and we dont care:
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter && Pickup_WidgetComponent)
	{
		BlasterCharacter->SetOverlappingWeapon(this);
		
		// //if we do it here we can use replication trick from char having this class as replicated member.
		//if (HasAuthority() && BlasterCharacter->GetRemoteRole() == ENetRole::ROLE_AutonomousProxy)
		//{
		//	//focus on the server copy only! the rest has been taken cared already, do not mess it up LOL:
		//	Pickup_WidgetComponent->SetVisibility(true);
		//}
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//USceneComponent::SetVisibility() - TIRE1 , the visibility of the UUserWidget_TIRE2 and TextBlock_TIRE3 is still true by default and we dont care:
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter) 
	{
		BlasterCharacter->SetOverlappingWeapon(nullptr);
		//Pickup_WidgetComponent->SetVisibility(false);  //if we do it here we can use replication trick from char having this class as replicated member.
	}
}

void AWeapon::ShowPickupWidget(bool bShowWdiget)
{
	if (Pickup_WidgetComponent)	Pickup_WidgetComponent->SetVisibility(bShowWdiget);
}

//to be called in ::Equip(), hence to be called in Input_EKeyPressed(), either in the server or a client (but NOT both, but NOT more than one), if in a client, this client again send RPC to execute in the server gain :D :D
void AWeapon::SetWeaponState(EWeaponState InState)
{
	 WeaponState = InState; //WeaponState is 2-way replicated, so we dont worry about it, no matter it is called on client or server copy
	 
	 //hitchhiking code: the reason why we need it here is for the care it is called on server-controlled-char copy
	 //these code below is not currently replicated, hence mind to do it "in 2 places" 
	 switch (WeaponState)
	 {
	 case EWeaponState::EWS_Equipped : 
		 ShowPickupWidget(false); 
		 if(Sphere) Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		 return;
	 }
}

//Will trigger as WeaponState is changed (say by SetWeaponState() ), [Server->clients]
//I dont think we need this function, because we have 'SetWeaponState' under 'GOLDEN pattern' (always execute on server) ?
// YES IT WORK WITHOUT THIS FUNCTION LOL, stephen didn't realize this and 'OVERKILL'
//this REDUDANT code will be called on all clients
void AWeapon::OnRep_WeaponState()
{
	//hitchhiking code: after this (and above...) you can remove 2 statements from CombatCompoent::Equip() now.
	//switch (WeaponState)
	//{
	//case EWeaponState::EWS_Equipped:
	//	ShowPickupWidget(false);
	//	//if (Sphere) Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); //surely redudant TIRE2 lol, COLLISION is only in the server so far
	//	return;
	//}
}

void AWeapon::PlayFireAnimation()
{
	if(WeaponMesh && AS_FireAnimation)  WeaponMesh->PlayAnimation(AS_FireAnimation , false);
}
