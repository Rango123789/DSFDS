// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/AProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include <Characters/BlasterCharacter.h>
#include "GameFramework/ProjectileMovementComponent.h"

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
}

void AAProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

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

	//best way: this return the name of UPROPERTY variable that changes its value:
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

	//now we assume other actor is another BlasterCharacter, not BlasterChacter shooting the weapon :D 
	ABlasterCharacter* Damaged_BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (Damaged_BlasterCharacter == nullptr || GetInstigator() == nullptr ) return;
	AController* InstagatorController = GetInstigator()->GetController();
	if (InstagatorController == nullptr) return;

	//Review, we did set "AWeapon = Owner of this Projectile" and "ACharacer holding Weapon = Instigator Pawn of this Projectile" since we spawn this Projectile, so now we need only access and use it :D :D

	UGameplayStatics::ApplyDamage(
		Damaged_BlasterCharacter,
		Damage,
		GetInstigator()->GetController(), //Instigator of this Projectile, of APawn type, was set to the Character holding the Weapon that this Projectile spawn from
		this,
		UDamageType::StaticClass()
	);

	//this must be final, because it has the line 'Destroy()', that cause the extra code ineffective
	Super::OnBoxHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}
