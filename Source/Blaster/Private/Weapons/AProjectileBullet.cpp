// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/AProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include <Characters/BlasterCharacter.h>

void AAProjectileBullet::OnBoxHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//now we assume other actor is another BlasterCharacter, not BlasterChacter shooting the weapon :D 
	ABlasterCharacter* Damaged_BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (Damaged_BlasterCharacter == nullptr) return;
    
	//Review, we did set "AWeapon = Owner of this Projectile" and "ACharacer holding Weapon = Instigator Pawn of this Projectile" since we spawn this Projectile, so now we need only access and use it :D :D
	UGameplayStatics::ApplyDamage(
		Damaged_BlasterCharacter,
		Damage,
		GetInstigator()->GetController(), //Instigator of this Projectile was set to the Character holding the Weapon that this Projectile spawn from
		this,
		UDamageType::StaticClass()
	);

	//this must be final, because it has the line 'Destroy()', that cause the extra code ineffective
	Super::OnBoxHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}
