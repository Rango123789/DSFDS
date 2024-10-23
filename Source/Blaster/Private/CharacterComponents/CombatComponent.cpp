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

	if (Character)
	{
		DefaultPOV = Character->GetCamera()->FieldOfView;
		CurrentPOV = DefaultPOV;
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

		Start_FireTimer();
	}
}


void UCombatComponent::Start_FireTimer()
{
	FTimerHandle TimeHandle;
	GetWorld()->GetTimerManager().SetTimer(TimeHandle , this, &ThisClass::FireTimer_Callback , 0.2f);
}

void UCombatComponent::FireTimer_Callback()
{
	if (!bIsFiring) return;
	//if (Character == nullptr || Character->IsLocallyControlled() == false) return;

	FHitResult HitResult;
	DoLineTrace_UnderCrosshairs(HitResult);

	//you can NOT call the multicast here, it may EXECUTE on all devices again if it is called from the server , indeed it is, as we just used golden rule to make sure all devices are executed!
	//MulticastInput_Fire(bIsFiring, HitResult.ImpactPoint);

	//this is the equivalent without side effect of RPCs:
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	if (bIsFiring)
	{
		Character->PlayFireMontage();

		EquippedWeapon->Fire(HitResult.ImpactPoint); //instead of member HitTarget, now you can remove it!

		Start_FireTimer();
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











