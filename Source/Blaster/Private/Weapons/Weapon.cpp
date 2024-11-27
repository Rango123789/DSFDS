// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/Weapon.h"
#include "Weapons/Casing.h" //to be spawned here 
#include "Characters/BlasterCharacter.h"
#include "PlayerController/BlasterPlayerController.h"
#include "CharacterComponents/CombatComponent.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Characters/BlasterCharacter.h"
#include "HUD/Overhead_UserWidget.h" //for testing
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Engine/SkeletalMeshSocket.h"
#include <Kismet/GameplayStatics.h>

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
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); //NEW
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

	//this will finally guarante the movement+location of the gun is indentical in all devices when simulating physics, also physics must be on in all devices, otherwise movement by physics part wont be gauranteed at all:
	//you can also go and check the box 'Replicate Movement' in BP lol:
	SetReplicateMovement(true); //this one, actually set bReplicateMovement = true
	//SetReplicatedMovement(FRepMovement::); //not this one

//PP_Hightlight, CustomDepth Stencil, Render CustomDepth Pass:
	//step1: set Stentil Value DOOR2 first
	WeaponMesh->SetCustomDepthStencilValue(251); //recommended
		//WeaponMesh->CustomDepthStencilValue = 251; //not recommended
	//step2: Enable RenderCustomDepth (Pass) DOOR1
		//option1:
	//WeaponMesh->SetCustomDepthStencilValue(251);
	WeaponMesh->SetRenderCustomDepth(true);
		//option2: 
		//WeaponMesh->bRenderCustomDepth = true;
		//WeaponMesh->MarkRenderStateDirty();
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

	////turn on CustomDepth as well: do client parts in OnRep_WeaponState()~>Dropped case -->better do it in SetWeaponState()
	//if(WeaponMesh) WeaponMesh->SetRenderCustomDepth(true);
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//USceneComponent::SetVisibility() - TIRE1 , the visibility of the TextBlock is still true by default and we dont care:
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter && Pickup_WidgetComponent)
	{
		BlasterCharacter->SetOverlappingWeapon(this);
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

//this is called from the server, so does whatever inside it!
void AWeapon::UpdateHUD_Ammo()
{
	if (OwnerCharacter == nullptr || OwnerCharacter->GetCombatComponent() == nullptr) return;

	if (Ammo <= 0) return;
//NOT the idea to call it here, it need to check Ammo >=0 before it fire outside :)
	//if (Ammo <= 0 && OwnerCharacter->GetCombatComponent()->GetCarriedAmmo() <= 0) return;
	//if (Ammo <= 0 && OwnerCharacter->GetCombatComponent()->GetCarriedAmmo() > 0)
	//{
	//	OwnerCharacter->GetCombatComponent()->Input_Reload();
	//}
	Ammo--;

	CheckAndSetHUD_Ammo();

}

void AWeapon::OnRep_Ammo()
{
	CheckAndSetHUD_Ammo();

	//any weapon can trigger this hosting function should have owner, so dont worry:
	if (OwnerCharacter == nullptr || OwnerCharacter->GetCombatComponent() == nullptr) return;

	if (IsFull() || OwnerCharacter->GetCombatComponent()->GetCarriedAmmo() == 0)
	{
		if (OwnerCharacter) OwnerCharacter->JumpToShotgunEndSection();//helper from Character
	}
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

void AWeapon::SetWeaponState_Only(EWeaponState InState)
{
	WeaponState = InState;
}

void AWeapon::SetWeaponState(EWeaponState InState)
{
	if (WeaponMesh == nullptr) return;

	 WeaponState = InState; 
	 
	 switch (WeaponState)
	 {
	 case EWeaponState::EWS_Equipped : 
		 ShowPickupWidget(false); 
		 //Only server need to touch this, option1 HasAuthority, option2 add or not doesn't matter
		 //if(HasAuthority() && Sphere) Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); //option1
		 if (Sphere) Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); //option2, add or not doesn't matter
		
		 //Our SMG dont need these to locally similated, move it out fix the bug:
		WeaponMesh->SetSimulatePhysics(false); //TIRE1
		WeaponMesh->SetEnableGravity(false);   //TIRE3 - no need nor should you do this LOL

		 if (WeaponType != EWeaponType::EWT_SMG)
		 {
			 WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); //TIRE2 , if =PhysicsAndQuery but disable SimulatePhysics then we can get a warning, so set it back to what it is in Constructor+BeginPlay
		 }
		 if (WeaponType == EWeaponType::EWT_SMG)
		 {
			 //this is the require of 'locally simulated bones in PA'
			 WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				 //WeaponMesh->SetSimulatePhysics(true);  //no need
				 //WeaponMesh->SetEnableGravity(true); //no need
			//GLOBALLY we turn it off all collision responses, hence this become 'PURE locally Simualted', but this has consequence steps:
			 WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		 }
		 //turn off CustomDepth
		 WeaponMesh->MarkRenderStateDirty();
		 WeaponMesh->SetRenderCustomDepth(false);

		 break;
	 case EWeaponState::EWS_Droppped :
		 //detach this weapon from CHAR::GetMesh();, detachment is replicated itself so yeah! no need to do in OnRep_ -here is currently OnRep_ lol
		 //DetachFromActor(FDetachmentTransformRules::KeepWorldTransform); // WeaponMesh->DetachFromComponent(); - stephen
		 //SetOwner(nullptr);

		 //Only server need to touch this
		 if (Sphere) Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		 //physics group: (let it happen to both SMG and non-SMG)
		 WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); //TIRE2
		 WeaponMesh->SetSimulatePhysics(true); //TIRE1
		 WeaponMesh->SetEnableGravity(true);   //TIRE3 - dont care what the default, better double-kill

		 //since the consequence only happen to SMG weapon as did above, so need to 'restore' it for SMG weapon
		 if (WeaponType == EWeaponType::EWT_SMG)
		 {
			 //collision setup: default beviour: block all except pawn, INACTIVE by default - we enable it later
			 WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
			 WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
			 WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); //NEW
			 //WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); //remove this contradiction line

		 }

		 //turn ON CustomDepth:
		 WeaponMesh->SetCustomDepthStencilValue(251);
		 WeaponMesh->SetRenderCustomDepth(true);

		 break;
	 }
}

void AWeapon::OnRep_WeaponState()
{
	if (WeaponMesh == nullptr) return;

	ABlasterCharacter* Character = Cast<ABlasterCharacter>(GetOwner());
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		//May NOT need as PrimativeComp->SetVisibility() can be replicated by UE5 itself even if comp::Replicates = false!
		//ShowPickupWidget(false);
		//Surely redudant TIRE2 lol, COLLISION is only in the server so far
		// //if (Sphere) Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//SMG dont need these turn on to locally simulated at all, move it out fix the bug "try to move a fully simulated mesh:
		WeaponMesh->SetSimulatePhysics(false); //TIRE1
		WeaponMesh->SetEnableGravity(false);   //TIRE3 - no need nor should you do this LOL

		if (WeaponType != EWeaponType::EWT_SMG)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); 
		}

		if (WeaponType == EWeaponType::EWT_SMG)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				//WeaponMesh->SetSimulatePhysics(true); //no need
				//WeaponMesh->SetEnableGravity(true);	//no need
			//GLOBALLY we turn it off all collision responses, hence this become 'PURE locally Simualted', but this has consequence steps:
			WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}

		//we do this with the hop that Combat::Equip()~>SetOwner(Char) will be replicated on clients before we reach this OnRep_WeaponState
		//so there is no gaurantee but double-kill does 'INCREASE' the chance of success:
		if (Character) AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::KeepWorldTransform, FName("RightHandSocket"));

		//turn OFF CustomDepth:
		//WeaponMesh->MarkRenderStateDirty(); //doesn't really help LOL
		WeaponMesh->SetRenderCustomDepth(false);
		UE_LOG(LogTemp, Warning, TEXT("OnRep_WeaponState~>Equipped"));
		break;
	case EWeaponState::EWS_Droppped:
		//NOR need we enable it back for Clients
		// //if (Sphere) Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		
		//physics group: (let it happen to both SMG and non-SMG)
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); //TIRE2
		WeaponMesh->SetSimulatePhysics(true); //TIRE1
		WeaponMesh->SetEnableGravity(true);   //TIRE3 - dont care what the default, better double-kill

		//since the consequence only happen to SMG weapon as did above, so need to 'restore' it for SMG weapon
		if (WeaponType == EWeaponType::EWT_SMG)
		{
			//collision setup: default beviour: block all except pawn, INACTIVE by default - we enable it later
			WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
			WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
			WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); //NEW
			//WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); //remove this contradiction line
		}

		//turn ON CustomDepth:
		//WeaponMesh->MarkRenderStateDirty(); //doesn't really help lol 
		WeaponMesh->SetCustomDepthStencilValue(251);
		WeaponMesh->SetRenderCustomDepth(true);
		UE_LOG(LogTemp, Warning, TEXT("OnRep_WeaponState~>Dropped"));
		break;
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

void AWeapon::PlayEquipSound(AActor* InActor)
{
	if (InActor == nullptr) return;
	UGameplayStatics::PlaySoundAtLocation(this, EquipSound, InActor->GetActorLocation());
}




