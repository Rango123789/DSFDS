// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterComponents/CombatComponent.h"
#include "Weapons/Weapon.h"
#include "Characters/BlasterCharacter.h"
#include "HUD/BlasterHUD.h"
#include "PlayerController/BlasterPlayerController.h"
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
	DOREPLIFETIME(UCombatComponent, bIsFiring); //just added, BUT didn't affect anyway
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
	}

	//TimerDelegate.BindUFunction()
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

//can factorize these in to Combat::Fire() to be used instead of Input_Fire itself in timer_callback
	if (bCanFire == false) return;
	
	//set it back to false as a part of preventing we spam the fire button during WaitTime to reach Timer callback
	bCanFire = false;

	FHitResult HitResult;
	DoLineTrace_UnderCrosshairs(HitResult);

	ServerInput_Fire(bIsFiring, HitResult.ImpactPoint); //rather than member HitPoint

	Start_FireTimer(); //this is the right place to call .SetTimer (which will be recursive very soon)
}

void UCombatComponent::ServerInput_Fire_Implementation(bool InIsFiring, const FVector_NetQuantize& Target)
{
	MulticastInput_Fire(InIsFiring, Target);
}

void UCombatComponent::MulticastInput_Fire_Implementation(bool InIsFiring, const FVector_NetQuantize& Target)
{
	//note that because the machine to be called is different, so put this line here or in the HOSTING function 'could' make a difference generally lol:
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	bIsFiring = InIsFiring; //We can't remove it here as this is for replication perupose :)

	if (bIsFiring)
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

	//only only set bCanfire = true for next chance if you did pass the WaitTime get upto the callback, so that we spam the Fire button, that could cause no-stop in firing

	//option2:  this is still from owning device, not yet via RPC process yet! so it work!
	//I have to call the hosting Input_Fire, rather than 'ServerInput_Fire( , )' because I didn't save the HitTarget
	//but will it work? yes it will for a moment and will be corrected back soon, but it is good enough because we're using it right now when we're still in the owning device, hence stephen save it as member will still work!
	// Even if you do Trace and saved it from Tick every frame or here it will work "momentary" (enough for here)
	//but you must know that after the frame HitTarget will be corrected back to the server value immediately! - we must add this point to UNIVERSEL rule
	//NOTE: I dont do this because I dont even trace it every frame in Combat::Tick, so I call it whenever Input_Fire is called and it is not much expesive as called everyframe so dont worry, my idea is not worst then stephen currently!
	Input_Fire(bIsFiring); 
}



//Instead of doing it from AWeapon::Equip() we do it here, hence it should do all the stuff we usually did here
void UCombatComponent::Equip(AWeapon* InWeapon)
{
	if (InWeapon == nullptr || Character == nullptr) return;
    
	//I move this on top with the hope that it is replicated before OnRep_WeaponState
	EquippedWeapon->SetOwner(Character);

	EquippedWeapon = InWeapon;

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped) ;

	bIsAutomatic = EquippedWeapon->GetIsAutomatic();
	FireDelay = EquippedWeapon->GetFireDelay();

	const USkeletalMeshSocket* RightHandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));

	if(RightHandSocket) RightHandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	
	//we want after we have a weapon on hand, we want Actor facing in the same direction as Camera!
	Character->GetCharacterMovement()->bOrientRotationToMovement = false; //at first it is true
	Character->bUseControllerRotationYaw = true; //at first it is false
}

//this is to fix the owning client can't update these on itself (weird case, can't explain :D )
void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon == nullptr || EquippedWeapon->GetWeaponMesh() || Character == nullptr) return;

	//the condition is optional, but since I know only that owning client have problem, so I only need to let this code run on that client LOL, hell yeah!

	//Note that I've been thinking about extra the code, setting WeaponState, Setting physics, but unfortunately WeaponState is not public member nor did I want to move it to public sesson to break my UNIVERSAL pattern :)
	//Hence the only choice1: is to follow stephen, focus on case=Equipped only
	//Choice2: create another exclusive setter SetWeaponStateOnly()

	//choice1:
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped); //go and restrict SetWeaponState::case_Equipped that the local can't touch Sphere collision -->Go and add If(HasAuthority()), but you dont have to setup physics here.
	
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











