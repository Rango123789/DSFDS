// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ProjectileGrenade.h"

AProjectileGrenade::AProjectileGrenade()
{
	//this is only for cosmetic, no need collision, as its cousin ProjectileBullet dont even have a mesh!
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>("ProjectileMesh");
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileGrenade::BeginPlay()
{

}
