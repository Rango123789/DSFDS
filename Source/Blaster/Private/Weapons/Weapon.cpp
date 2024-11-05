// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/Weapon.h"
#include "Weapons/Casing.h" //to be spawned here 
#include "Characters/BlasterCharacter.h"
#include "PlayerController/BlasterPlayerController.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Characters/BlasterCharacter.h"
#include "HUD/Overhead_UserWidget.h" //for testing
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Engine/SkeletalMeshSocket.h"


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
	DOREPLIFETIME(AWeapon, Ammo);
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

void AWeapon::Drop()
{
	SetWeaponState(EWeaponState::EWS_Droppped);

	//these are self-replicated actions, so dont need to put it inside OnRep_
	FDetachmentTransformRules Rules(EDetachmentRule::KeepWorld, true);
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	//FDetachmentTransformRules Rules(EDetachmentRule::KeepWorld, true);
	SetOwner(nullptr);

	//set Char, PC to null too, avoiding stolen weapon affect its old owner's HUD and so on
	//these are not self-replicated so we will call it again, in say , OnRep_Owner too!
    //but this time you MUST check if (Owner == nullptr) before you do these, because it replicated when it is set to nullptr or to valid pointer (dont want this)
	OwnerCharacter = nullptr; //not self-replicated
	BlasterPlayerController = nullptr;
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

void AWeapon::UpdateHUD_Ammo()
{
	if (Ammo <= 0) return;

	Ammo--;

	CheckAndSetHUD_Ammo();
}

void AWeapon::OnRep_Ammo()
{
	CheckAndSetHUD_Ammo();
}

void AWeapon::OnRep_Owner()
{
	//after the change, if Owner is changed to null, then we set Char and PC to null as well
	if (Owner == nullptr)
	{
		OwnerCharacter = nullptr;
		BlasterPlayerController = nullptr;

		return; //you dont need to do code outside if it change to nullptr! or you can use "else" after you factorize these shit below into 'CheckAndSetHUD_Ammo()'
	}

	CheckAndSetHUD_Ammo();

	////OPTION2: remember to add UPROPERTY() will help 'invalid address' into null to avoid undefined behavour when its owner elimned:
	//OwnerCharacter = OwnerCharacter == nullptr ? GetOwner<ABlasterCharacter>() : OwnerCharacter;
	//if (OwnerCharacter == nullptr) return;

	//BlasterPlayerController = BlasterPlayerController == nullptr ? OwnerCharacter->GetController<ABlasterPlayerController>() : BlasterPlayerController;

	//if (BlasterPlayerController) BlasterPlayerController->SetHUDAmmo(Ammo);
}

void AWeapon::CheckAndSetHUD_Ammo()
{
	//OPTION2: remember to add UPROPERTY() will help 'invalid address' into null to avoid undefined behavour when its owner elimned:
	OwnerCharacter = OwnerCharacter == nullptr ? GetOwner<ABlasterCharacter>() : OwnerCharacter;
	if (OwnerCharacter == nullptr) return;

	BlasterPlayerController = BlasterPlayerController == nullptr ? OwnerCharacter->GetController<ABlasterPlayerController>() : BlasterPlayerController;

	if (BlasterPlayerController) BlasterPlayerController->SetHUDAmmo(Ammo);
}

void AWeapon::ShowPickupWidget(bool bShowWdiget)
{
	if (Pickup_WidgetComponent)	Pickup_WidgetComponent->SetVisibility(bShowWdiget);
}

//to be called in ::Equip(), hence to be called in Input_EKeyPressed(), either in the server or a client (but NOT both, but NOT more than one), if in a client, this client again send RPC to execute in the server gain :D :D
void AWeapon::SetWeaponState(EWeaponState InState)
{
	 WeaponState = InState; 
	 
	 switch (WeaponState)
	 {
	 case EWeaponState::EWS_Equipped : 
		 ShowPickupWidget(false); 
		 //Only server need to touch this, option1 HasAuthority, option2 add or not doesn't matter
		 //if(HasAuthority() && Sphere) Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); //option1
		 if (Sphere) Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); //option2, add or not doesn't matter

		 WeaponMesh->SetSimulatePhysics(false); //TIRE1
		 WeaponMesh->SetEnableGravity(false);   //TIRE3 - no need nor should you do this LOL
		 if (WeaponMesh) WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); //TIRE2 , if =PhysicsAndQuery but disable SimulatePhysics then we can get a warning, so set it back to what it is in Constructor+BeginPlay
		 return;
	 case EWeaponState::EWS_Droppped :
		 //detach this weapon from CHAR::GetMesh();, detachment is replicated itself so yeah! no need to do in OnRep_ -here is currently OnRep_ lol
		 //DetachFromActor(FDetachmentTransformRules::KeepWorldTransform); // WeaponMesh->DetachFromComponent(); - stephen
		 //SetOwner(nullptr);

		 //Only server need to touch this
		 if (Sphere) Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		 //physics group:
		 WeaponMesh->SetSimulatePhysics(true); //TIRE1
		 WeaponMesh->SetEnableGravity(true);   //TIRE3 - dont care what the default, better double-kill
		 if (WeaponMesh) WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); //TIRE2
	 }
}

void AWeapon::SetWeaponState_Only(EWeaponState InState)
{
	WeaponState = InState;
}

//Will trigger as WeaponState is changed (say by SetWeaponState() ), [Server->clients]
//I dont think we need this function, because we have 'SetWeaponState' under 'GOLDEN pattern' (always execute on server) ?
// YES IT WORK WITHOUT THIS FUNCTION LOL, stephen didn't realize this and 'OVERKILL'
//this REDUDANT code will be called on all clients
void AWeapon::OnRep_WeaponState()
{
	ABlasterCharacter* Character = Cast<ABlasterCharacter>(GetOwner());

	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		//May NOT need as PrimativeComp->SetVisibility() can be replicated by UE5 itself even if comp::Replicates = false!
		//ShowPickupWidget(false);
		
		//Surely redudant TIRE2 lol, COLLISION is only in the server so far
		// //if (Sphere) Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		WeaponMesh->SetSimulatePhysics(false); //TIRE1
		WeaponMesh->SetEnableGravity(false);   //TIRE3 - no need nor should you do this LOL
		if (WeaponMesh) WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); 

		//we do this with the hop that Combat::Equip()~>SetOwner(Char) will be replicated on clients before we reach this OnRep_WeaponState, so there is no gaurantee but double-kill does 'INCREASE' the chance of success:

		if (Character) AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::KeepWorldTransform, FName("RightHandSocket"));

		return;
	case EWeaponState::EWS_Droppped:
		//NOR need we enable it back for Clients
		// //if (Sphere) Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		
		////physics group:
		//WeaponMesh->SetSimulatePhysics(true); //TIRE1
		//WeaponMesh->SetEnableGravity(true);   //TIRE3 - dont care what the default, better double-kill
		//if (WeaponMesh) WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); //TIRE2

		return;
	};
}

void AWeapon::Fire(const FVector& HitTarget)
{
	//play Fire animtion:
	if(WeaponMesh && AS_FireAnimation)  WeaponMesh->PlayAnimation(AS_FireAnimation , false);
	
	//spawn ACasing: if you dont pick CasingClass from BP, this block will be skipped! good for flexibility!
	if (CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));

		if (AmmoEjectSocket == nullptr || GetWorld() == nullptr) return;

		FTransform AmmoEjectSocket_Transform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);

		GetWorld()->SpawnActor<ACasing>(CasingClass, AmmoEjectSocket_Transform);

	}

	//Option2: stephen call UpdateHUD_Ammo() here, and say this is called from the server. yes it is, but it is inside a multicast and also called from the clients at the same time too :D :D
}




