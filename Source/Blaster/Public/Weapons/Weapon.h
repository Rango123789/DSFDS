// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
    EWS_Initial UMETA(DisplayName = "Initial"), //Stephen add "Initial State"
    EWS_Equipped UMETA(DisplayName = "Equipped"),   //stephen: equipped - sound too violent to me
    EWS_Droppped UMETA(DisplayName = "Dropped"),

    EWP_MAX UMETA(DisplayName = "DefaultMAX"), //for the sake of knowing now many sematic values of this enum
};


UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	

public:	              
/***functions***/
//category1: auto-generated functions:	
    AWeapon();
	virtual void Tick(float DeltaTime) override;
//category2: virtual functions:
    /**<Actor>*/                                                                               
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    /**</Actor>*/

    /**<X>*/

    /**</X>*/

//category3: regular functions: 
    //montages:
    void PlayFireAnimation();

    //sound and effects:

    //bool functions:

    //BP-callale functions:
    

//category4: callbacks 
    UFUNCTION()
    void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    UFUNCTION()
    void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

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
    UPROPERTY(ReplicatedUsing=OnRep_WeaponState , EditAnywhere, BlueprintReadWrite)
    EWeaponState WeaponState{EWeaponState::EWS_Initial};

    UFUNCTION()
    void OnRep_WeaponState();

    //pointer to external classes:

    //arrays:

    //class type:

//category2: UActorComponents
    UPROPERTY(VisibleAnywhere)
    USkeletalMeshComponent* WeaponMesh; //NEW1: look at assets, guns are all SKM instead of SM for good
                                        //no need to forward declare no include it in UE5.2+ ?
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class USphereComponent* Sphere;

    //UPROPERTY(VisibleAnywhere, Replicated)
    UPROPERTY(VisibleAnywhere)
    class UWidgetComponent* Pickup_WidgetComponent;

    UPROPERTY(VisibleAnywhere)
    class UWidgetComponent* Overhead_WidgetComponent; //for testing

//category3: Engine types      
    //montages:
    UPROPERTY(EditAnywhere)
    class UAnimationAsset* AS_FireAnimation; //AS = Animation Sequence , AA = Animation Asset


    //sound and effects:

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
    UWidgetComponent* GetPickupWidgetComponent() { return Pickup_WidgetComponent; }

    USkeletalMeshComponent* GetWeaponMesh(){ return WeaponMesh; }

    USphereComponent* GetSphere() { return Sphere; }

    void ShowPickupWidget(bool bShowWdiget);

    void SetWeaponState(EWeaponState InState);
};
