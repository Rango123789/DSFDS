// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Projectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	//setup CollisionBox/ProjectileBox
	CollisionBox = CreateDefaultSubobject<UBoxComponent>("Weapon Box");
	RootComponent = CollisionBox; 
	
	//it is collision-capable comp, let's give it some settings:
	   //it is moved by logic, also by physics so we choose WorldDynamic:
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	   //we want it also have physics, we enable it right here rather than in BeginPlay() like for AWeapon::Sphere previously in this Multiplayer game.
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); 
	   //ignore all first, and block some later: 
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	   //so we want this projectile to be blocked by Static objects in world
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	   //we need it to block Visibility (not talk about Camera) so that we can do some trace on it I guess?
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	
	//Setup ProjectileMovementComp:
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("Projectile Move Comp");
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	if (Tracer)
	{
		TracerComponent =	
		UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			CollisionBox, //even if it is RootComp, but you can't pass in this=Actor type
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepRelativeOffset //I test this
		);
	}
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

