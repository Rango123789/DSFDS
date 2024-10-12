// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:
/***functions***/
//category1: auto-generated functions:
    AProjectile();
//category2: virtual functions:
    /**<Actor>*/
    virtual void Tick(float DeltaTime) override;
    /**</Actor>*/

    /**<X>*/

    /**</X>*/

//category3: regular functions: 
    //montages:

    //sound and effects:

    //bool functions:

    //BP-callale functions:
//category4: callbacks 
    UFUNCTION()
    virtual void OnBoxHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    void virtual Destroyed() override;

protected: //base	
/***functions***/
//category1: auto-generated functions:
virtual void BeginPlay() override;
//category2: virtual functions:
    /**<Actor>*/

    /**</Actor>*/

    /**<X>*/

    /**</X>*/

//category3: regular functions 

//category4: callbacks

/***data members****/
//Category1: Enums , arrays, pointers to external classes
    //enum states:

    //pointer to external classes:

    //arrays:

    //class type:

//category2: UActorComponents   
    UPROPERTY(VisibleAnywhere)
    class UBoxComponent* CollisionBox;

    UPROPERTY(VisibleAnywhere)
    class UProjectileMovementComponent* ProjectileMovementComponent;

//category3: Engine types      
    //montages:

    //sound and effects:
    UPROPERTY(EditAnywhere)
    UParticleSystem* Tracer; //to be picked as 'P_AssaultRiffle_Tracer' from BP_Projectile

    UParticleSystemComponent* TracerComponent; //to store temp object return by SpawnEmitter

    UPROPERTY(EditAnywhere)
    USoundBase* HitSound;

    UPROPERTY(EditAnywhere)
    UParticleSystem* HitParticle;

//category4: basic and primitive types


private: //FINAL child
/***functions***/
//category1: auto-generated functions:

//category2: virtual functions:

//category3: regular functions 

//category4: callbacks

/***data members****/
//Category1: Enums , arrays, pointers to external classes

//category2: UActorComponents   

//category3: Engine types      
    //montages:

    //sound and effects:

//category4: basic and primitive types

public:
/***Setters and Getters***/


};
