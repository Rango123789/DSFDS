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
    //for server-side rewind:
    UPROPERTY(EditAnywhere)
    bool bUseServerSideRewind_TIRE2 = false;
    
    FVector_NetQuantize TraceStart;
    FVector_NetQuantize100 InitialVelocity; //we need it more accurate because not only it contain value but also direction! I even think we should use FVector!

    //This is meant to __ = UPMC::InitalSpeed and vice versa, otherwise the path is in one way and the actual projectile fly in other way :D :D :
    UPROPERTY(EditAnywhere)
    float InitalSpeed_ProjectilePath = 1300.f;

    //Move it up here for ease of use:    
    UPROPERTY(EditAnywhere)
    float Damage = 15.f;

    UPROPERTY(EditAnywhere)
    float Damage_HeadShot = 45.f;

    //my idea so not we wont accept it do ServerRewind if server hold the weapon in all cases:
    bool bServerHoldWeapon = true;

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

    void StartDestroyTimer();

    void virtual Destroyed() override;

protected: //base	
/***functions***/
//category1: auto-generated functions:
virtual void BeginPlay() override;
void SpawnSmokeTrailSystem();
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

    //move from child: so now only whater child need it should CreateDefaultObject there!
    UPROPERTY(VisibleAnywhere)
    class UStaticMeshComponent* ProjectileMesh;
//category3: Engine types      
    //montages:

    //sound and effects:
    UPROPERTY(EditAnywhere)
    UParticleSystem* Tracer; //to be picked as 'P_AssaultRiffle_Tracer' from BP_Projectile

    UPROPERTY(EditAnywhere)
    UParticleSystemComponent* TracerComponent; //to store temp object return by SpawnEmitter

    UPROPERTY(EditAnywhere)
    USoundBase* HitSound;

    UPROPERTY(EditAnywhere)
    UParticleSystem* HitParticle;

    //Move from child
    UPROPERTY(EditAnywhere)
    class UNiagaraSystem* NiagaraSystem_SmokeTrail; //SmokeEffect for N asset Vs SmokeParticle for P asset
    UPROPERTY()
    class UNiagaraComponent* NiagaraComponent_SmokeTrail;

    //Move from child:
        //no need to create callback, the callback is 'void Destroy()' itself satisied signature requirement? No it require UFUNCTION(), where AActor::Destroy() is not a UFunction()
        //you see it require TFunction ~> I guess it is 'function marked with UFUNCTION'
    FTimerHandle TimerHandle_Destroy;
    UPROPERTY(EditAnywhere)
    float DelayTime_Destroy = 3.f;
    UFUNCTION()
    void TimerCallback_Destroy();

//category4: basic and primitive types


    //Only make sense for childs applying RadialRadisu
    UPROPERTY(EditAnywhere)
    float DamageInnerRadius = 200.f;
    UPROPERTY(EditAnywhere)
    float DamageOuterRadius = 500.f;

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
