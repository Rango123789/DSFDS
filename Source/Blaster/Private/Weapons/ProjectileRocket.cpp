// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include <Characters/BlasterCharacter.h>
#include "NiagaraComponent.h"
#include <NiagaraFunctionLibrary.h>

AProjectileRocket::AProjectileRocket()
{
	//this is only for cosmetic, no need collision, as its cousin ProjectileBullet dont even have a mesh!
	RocketMesh1 = CreateDefaultSubobject<UStaticMeshComponent>("RocketMesh1");
	RocketMesh1->SetupAttachment(RootComponent);
	RocketMesh1->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	UNiagaraFunctionLibrary::SpawnSystemAttached(
		NS_SmokeTracer,
		RootComponent,
		FName(),
		GetActorLocation(),
		FRotator(),
		EAttachLocation::KeepWorldPosition,
		true //bAutoDestroy
	);
}

void AProjectileRocket::OnBoxHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	////now we assume other actor is another BlasterCharacter, not BlasterChacter shooting the weapon :D 
	//ABlasterCharacter* Damaged_BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	//if (Damaged_BlasterCharacter == nullptr || GetInstigator()) return;

	//it must be shot from someone, it must have an instigator LOL
	AController* InstagatorController = GetInstigator()->GetController();
	if (InstagatorController == nullptr) return;

	////We dont do this for Rocket , we need to apply Radial Damage instead:
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
		200.f, //DamgeInnerRadius, this is radius NOT Damage LOL
		500.f,  //DamageOuterRadius
		1.f,   // 1^[X] , here X = 1.f
		UDamageType::StaticClass(),
		TArray<AActor*>{},     //IgnoreActors
		this,                  //DamageCauser
		InstagatorController,  //InstigatedByController
		ECollisionChannel::ECC_Visibility
	);

	//this must be final, because it has the line 'Destroy()', that cause the extra code ineffective
	Super::OnBoxHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}

