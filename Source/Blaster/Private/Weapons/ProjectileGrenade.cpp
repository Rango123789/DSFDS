// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ProjectileGrenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include <Kismet/GameplayStatics.h>

AProjectileGrenade::AProjectileGrenade()
{
//copy from AprojectileBullet
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>("ProjectileMesh");
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

//copy from AProjectileBullet, beasuse they both use 'PURE UPMC', but this ProjectileGrenade will take the use of UPMC::OnProjectileBounce delegate:
	//the variable is defined from parent but not yet..., now we create default sub object here:
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("Projectile Move Comp");
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->InitialSpeed = 600;
	ProjectileMovementComponent->MaxSpeed = 1200;
	//[UPDATE] to be exact the PURE version also need it in case you  'return from first hit in your code'
	ProjectileMovementComponent->SetIsReplicated(true);
//Now we want grenade to bounce: should we let all devices bounce or only the server bouce first?
	ProjectileMovementComponent->bShouldBounce = true; //default = false -> stop moving when detecting hit!
	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &ThisClass::OnProjectileBounce);

}

void AProjectileGrenade::BeginPlay()
{
	//Stephen say we dont need this any more
	//Super::BeginPlay(); //call AProjectile::BeginPlay() ~> Spawn Tracer (no need here) + bind callback for CollisionBox (no need any more as grenade explosion time is hit && bounces independent

	AActor::BeginPlay();

	SpawnSmokeTrailSystem();

	//For Grenade, calll StartDestroyTimer() right in beginplay, as it is 'hit && bounces' independent.
	//when time reaches ->AActor::Destroy() -> AProjectile::Destroyed(): currently play HitSound, HitPArticle - that can be used in place of ExplodeSound , ExplodePartible - hell yeah!
	StartDestroyTimer();
}

void AProjectileGrenade::Destroyed()
{
	//Stephen say we should to this final, But I think we can even do it first, not sure it will cause any side effect, for example for replication? 
	//try to swap the order and see if it fix some old issue with my AssaultRiffle when spawning sound and effect when firing?
	Super::Destroyed(); //acceting the fact that SmokeTrailSystem will disappear as the grenade explode when time reach

//can factorize this is to parent::ExplodeDamage() and re-use them 2 childs Rocket and Grenade
	//this is where we must apply Damage for grenade: we dont even need OnBoxHit, hence dont even need to override nor did call Super::BeginPlay (we only call AActor::BeginPlay() ) to bind OnBoxHit for it to trigger the Super::OnBoxHit version at all, anyway AProjectile::OnBoxHit{ Destroy();} - just that LOL, there is more crowded in child's versions !
	if (HasAuthority())
	{
		AController* InstagatorController = nullptr;
		if(GetInstigator()) InstagatorController = GetInstigator()->GetController();
		if (InstagatorController == nullptr) return;
		////We dont do this for Rocket nor Grenade , we need to apply Radial Damage instead:
		//UGameplayStatics::ApplyDamage(
		//	Damaged_BlasterCharacter,
		//	Damage,
		//	GetInstigator()->GetController(), //Instigator of this Projectile, of APawn type, was set to the Character holding the Weapon that this Projectile spawn from
		//	this,
		//	UDamageType::StaticClass()
		//);
		
		//we need to apply Radial Damage instead:
		UGameplayStatics::ApplyRadialDamageWithFalloff(
			this, //WorldContextObject
			Damage, //BaseDamage 
			10.f,   //MinimumDanage
			GetActorLocation(), //Origin of the radial range
			DamageInnerRadius, //DamageInnerRadius, this is radius NOT Damage LOL
			DamageOuterRadius,  //DamageOuterRadius
			1.f,   // 1^[X] , here X = 1.f
			UDamageType::StaticClass(),
			TArray<AActor*>{},     //IgnoreActors
			this,                  //DamageCauser
			InstagatorController,  //InstigatedByController
			ECollisionChannel::ECC_Visibility
		);
	}
}

//trigger per bounce I guess, this currently call on all proxies:
void AProjectileGrenade::OnProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	UGameplayStatics::PlaySoundAtLocation(this, BounceSound, GetActorLocation());
}
