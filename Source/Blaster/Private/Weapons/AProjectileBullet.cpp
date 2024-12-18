// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/AProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include <Characters/BlasterCharacter.h>
#include "GameFramework/ProjectileMovementComponent.h"
#include <PlayerController/BlasterPlayerController.h>
#include "CharacterComponents/LagCompensationComponent.h"
#include "Components/BoxComponent.h"
#include "Weapons/Weapon.h"

AAProjectileBullet::AAProjectileBullet()
{
	//the variable is defined from parent but not yet..., now we create default sub object here:
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("Projectile Move Comp");
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->InitialSpeed = InitalSpeed_ProjectilePath;
	ProjectileMovementComponent->MaxSpeed = InitalSpeed_ProjectilePath;

	//Stephen recommend, but we dont need this if we dont already the UPMC, this bullet use the built-in version so dont need this here (but need in Rocket):
	// [UPDATE] to be exact the PURE version also need it in case you  'return from first hit in your code'
	ProjectileMovementComponent->SetIsReplicated(true);

	//if(!HasAuthority() && CollisionBox )
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Enter: AAProjectileBullet::constructor~> !HasAuthority()"))
	//		CollisionBox->SetNotifyRigidBodyCollision(true); //C++ name for Generate Hit Event from BP

	//	//CollisionBox->OnComponentHit.AddDynamic(this, &ThisClass::OnBoxHit);
	//}
}

void AAProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	//TestPredictProjectilePath();

	//to use server side rewind, we need to UNLOCK collision for the rest projectile proxies as well, stephen forget this:
	//AWeapon* OwnerWeapon =Cast<AWeapon>( GetOwner() );
	//&& OwnerWeapon && OwnerWeapon->GetUseServerSideRequest() == true)
	//if (!HasAuthority() && CollisionBox )
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Enter: AAProjectileBullet::BeginPlay() ~> !HasAuthority()"))
	//	CollisionBox->SetNotifyRigidBodyCollision(true); //C++ name for Generate Hit Event from BP

	//	//CollisionBox->OnComponentHit.AddDynamic(this, &ThisClass::OnBoxHit);
	//}
}

void AAProjectileBullet::TestPredictProjectilePath()
{
	FPredictProjectilePathParams PathParams;
	FPredictProjectilePathResult PathResult;

	TArray<AActor*> ActorsToIgnores;
	ActorsToIgnores.AddUnique(this);

	//PathParams.ActorsToIgnore = ActorsToIgnores; //NOT recommended
	PathParams.ActorsToIgnore.Add(this);         //Recommend
	//PathParams.bTraceComplex = false; //trace against simple collision

	//if you use TraceWithChannel rthen set the TraceChanel, if you use TraceWithCollision, then set ObjectTypes:
	//however the fact is that we must set bTraceWithCollision = true so that you see 'RED' sphere where it path hit something!
	PathParams.bTraceWithChannel = true;
	PathParams.TraceChannel = ECollisionChannel::ECC_Visibility;
	//stephen: this allows us to predict projectile path and we can actually generate hit events as we tracing against the collision objects in our world!
	PathParams.bTraceWithCollision = true;
	//PathParams.ObjectTypes = TArray<EObjectTypeQuery>(...);

	//Debuging purpose DrawDebugType = none by default:
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	PathParams.DrawDebugTime = 10.f;

	//this is 2 most important parameters:
	PathParams.LaunchVelocity = GetActorForwardVector() * InitalSpeed_ProjectilePath;
	PathParams.StartLocation = GetActorLocation();

	//this decide the sub accuracy, bigger means more accurate but cost performance:
	PathParams.MaxSimTime = 7.f;
	PathParams.SimFrequency = 30.f;

	PathParams.ProjectileRadius = 4.f;

	//PathParams.OverrideGravityZ; //only use if needed

	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
}

#if WITH_EDITOR
//this function trigger whenever we make a change on UPROPERTY property from UE5:
void AAProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = PropertyChangedEvent.GetPropertyName();
	//long way:
	//FName PropertyName
	//	= PropertyChangedEvent.Property != nullptr ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	//this check if that property is the  InitalSpeed variable, if true, make UPMC::InitialSpeed follow it too!
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AAProjectileBullet, InitalSpeed_ProjectilePath))
	{
		ProjectileMovementComponent->InitialSpeed = InitalSpeed_ProjectilePath;
		ProjectileMovementComponent->MaxSpeed = InitalSpeed_ProjectilePath;
	}

	//this return FName(TEXT("MemberName"))
	FName InitalSpeed_Name = GET_MEMBER_NAME_CHECKED(AAProjectileBullet, InitalSpeed_ProjectilePath);

	UE_LOG(LogTemp, Warning, TEXT("%s"), *InitalSpeed_Name.ToString());
	UE_LOG(LogTemp, Warning, TEXT("%s"), *PropertyName.ToString());
}
#endif

void AAProjectileBullet::OnBoxHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("Enter: AAProjectileBullet::OnBoxHit"))
	ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(OtherActor);
	ABlasterPlayerController* InstigatorController = nullptr;

	ABlasterCharacter* AttackCharacter = Cast<ABlasterCharacter>(GetInstigator());
	if (AttackCharacter) InstigatorController =Cast<ABlasterPlayerController>(AttackCharacter->GetController());

	//in insimulated proxies you can't never get to pass 'InstagatorController' (it is null there), so this is perhaps a good news:
	if (HitCharacter && InstigatorController)
	{
		//Review, we did set "AWeapon = Owner of this Projectile" and "ACharacer holding Weapon = Instigator Pawn of this Projectile" since we spawn this Projectile, so now we need only access and use it :D :D
		
		//UPDATE: bServerHoldWeapon, so that server wont try to cause second damage when the CD client already try to send RPC already!
		// The reason why we try to limit server case instead is that according to code from ProjectileWeapon::Fire, currently there is a lot of case can go pass 'HasAuthority() && !bUseServerSideRewind_TIRE2':
		// 1 - Replicated projectile spawned by the server
		// 2 - non-Replicated projectile spawnd by the server = this is dangerous = we need to limit this case by 'bServerHoldWeapon', hell yeah!!!
		// 3 - non-CD non-Replicated projectile spawned by the non-CD clients = this doesn't affect what we doing here, because it can't even pass the if(InstigatorController) above (reason: only char in server or CD can have its valid PC)

		//if (HasAuthority() && !bUseServerSideRewind_TIRE2) //will cause double damage by non-Replicated projectile spawnd by the server 
		if (HasAuthority() && !bUseServerSideRewind_TIRE2 && bServerHoldWeapon) //this FIX
		{
			UGameplayStatics::ApplyDamage(HitCharacter,Damage,GetInstigator()->GetController(), this, UDamageType::StaticClass()
			);
		}

		//I think IsLocallyControlled() is 'doudle of safety' if it is not the server alreadt, then only CD can pass if(PC) check above!
		//also there is only one case that has bUseServerSideRewind_TIRE2 = true in last lesson:
		
		//UPDATE: for non-replicated actor, it has HasAuthority() true no matter what devive have it, meaning !HasAuthority() now will block non-replicated actors in clients that is NOT wanted here :D :D 
		// if (!HasAuthority() && GetInstigator()->IsLocallyControlled() && bUseServerSideRewind_TIRE2 && !bServerHoldWeapon)
		// if (!HasAuthority() && GetInstigator()->IsLocallyControlled() && bUseServerSideRewind_TIRE2 )
		//if (!HasAuthority() && GetInstigator()->IsLocallyControlled()) // cause double damage 
		//if (!HasAuthority() && bUseServerSideRewind_TIRE2 ) //this can't pass , non-replicated actor HasAuthority() instead
		if (bUseServerSideRewind_TIRE2 && GetInstigator()->IsLocallyControlled())// only this can pass , cause double damage 
		{
			UE_LOG(LogTemp, Warning, TEXT("Enter: AAProjectileBullet::OnBoxHit && !HasAuthority() ") )
			float HitTime = InstigatorController->GetServerTime_Synched() - InstigatorController->RTT / 2;

			//we did set Owner of projectile = the weapon (rather than = Character itself like stephen, that instigator is enough), DO NOT confuse: owner of weapon is still character, we talking about projectile TIRE3 here:
			AWeapon* DamageCauser = Cast<AWeapon>(GetOwner());

			//However in worst case, we can still pass in AttackCharacter->Combat->EquippedWeapon right? :D :D
			//okay now I know why we must do it like stephen did, because I still in fact use 'Weapon->Damage' from ServerScoreRequest LOL, so if i create AProjectile* Projectile instead, I fail to pass it into the server because it is NOT a replicated actor -->PUZZLE solved!!!! 

			if (AttackCharacter && AttackCharacter->GetLagComponent())
			{
				AttackCharacter->GetLagComponent()->ServerScoreRequest_Projectile(
					TraceStart, InitialVelocity, HitCharacter, HitTime, DamageCauser );
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("Exit AAProjectileBullet::OnBoxHit"))
	}
	//this must be final, because it has the line 'Destroy()', that cause the extra code ineffective
	//you want to play HitParticle no matter you did hit any char or not:
	Super::OnBoxHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}
