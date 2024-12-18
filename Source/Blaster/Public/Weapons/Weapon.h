// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blaster/WeaponTypes.h"
#include "Weapon.generated.h"


UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	

public:	          
/***exception of data member***/
    //textures for weapon crosshairs
    UPROPERTY(EditAnywhere)
    UTexture2D* CrosshairsCenter;
    UPROPERTY(EditAnywhere)
    UTexture2D* CrosshairsLeft;
    UPROPERTY(EditAnywhere)
    UTexture2D* CrosshairsRight;
    UPROPERTY(EditAnywhere)
    UTexture2D* CrosshairsTop;
    UPROPERTY(EditAnywhere)
    UTexture2D* CrosshairsBottom;

    UPROPERTY(EditAnywhere)
    float RTTFactor = 0.5f;

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
    virtual void Fire(const FVector& HitTarget);

    //MOVE from HitScanWeapon:
    FVector RandomEndWithScatter(const FVector& HitTarget);
    UPROPERTY(EditAnywhere)
    float DistanceToSphere = 800.f;
    UPROPERTY(EditAnywhere)
    float SphereRadius = 75.f;
    UPROPERTY(EditAnywhere)
    bool bUseScatter = false;

    //sound and effects:
    void PlayEquipSound(AActor* InActor);
    //bool functions:

    //BP-callale functions:
    
    //others:
    void Drop();
    void UpdateHUD_Ammo();
    void CheckAndSetHUD_Ammo();
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
    virtual void OnRep_Owner() override;
    
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

    UPROPERTY(EditAnywhere)
    EWeaponType WeaponType = EWeaponType::EWT_AssaultRifle;
    //optional:
    UPROPERTY(EditAnywhere)
    EFireType FireType = EFireType::EFT_Projectile;

    //pointer to external classes:

    //arrays:

    //class type:
    UPROPERTY(EditAnywhere)
    TSubclassOf<class ACasing> CasingClass;

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
    UPROPERTY(EditAnywhere)
    USoundBase* EquipSound;
//category4: basic and primitive types
    //FOV:
    UPROPERTY(EditAnywhere)
    float FOV = 30;
    UPROPERTY(EditAnywhere)
    float FOVInterpSpeed = 20;

    UPROPERTY(EditAnywhere)
    bool bIsAutomatic = true;
    UPROPERTY(EditAnywhere)
    float FireDelay = 0.25;

    //Ammo - HUD relevant:
    uint32 NumOfUnprocessedUpdateAmmo = 0 ;

    //UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
    UPROPERTY(EditAnywhere)
    int32 Ammo = 30 ;
    
    //UFUNCTION()
    virtual void OnRep_Ammo();

    UFUNCTION(Client, Reliable)
    void ClientUpdateHUD_Ammo(int32 CurrentServerAmmo);
    UFUNCTION(Client, Reliable)
    void ClientSetAmmo(int32 CurrentServerAmmo);

    UPROPERTY(EditAnywhere)
    int32 MagCapacity = 30; //not use yet

    UPROPERTY()
    class ABlasterCharacter* OwnerCharacter;//avoid same name as some local var of its method
    UPROPERTY()
    class ABlasterPlayerController* BlasterPlayerController;

    bool bIsDefaultWeapon = false; //only DefaultWeapon spawn with Char::BeginPlay should have this 'true'

    UPROPERTY(EditAnywhere)
    float Damage = 15.f; //move from HitScanWeapon

    UPROPERTY(Replicated, EditAnywhere)
    bool bUseServerSideRewind = false;

    UPROPERTY(Replicated, EditAnywhere)
    bool bUseServerSideRewind_Initial = false;

    UFUNCTION()
    void OnReportPingStatus_Callback(bool bHighPing);

#if WITH_EDITOR
    virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

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

    class USphereComponent* GetSphere() { return Sphere; }

    void ShowPickupWidget(bool bShowWdiget);

    void SetWeaponState(EWeaponState InState);

    void OnWeaponStateSet();

    void OnDropped();

    void OnEquippedSecond();

    void OnEquipped();

    void SetWeaponState_Only(EWeaponState InState);

    float GetPOV() { return FOV; }
    float GetPOVInterpSpeed() { return FOVInterpSpeed; }

    bool GetIsAutomatic() { return bIsAutomatic; }
    float GetFireDelay() { return FireDelay; }
    
    int32 GetAmmo() { return Ammo; }
    void SetAmmo(int32 InAmmo);
    
    int32 GetMagCapacity() { return MagCapacity; }
    //void SetMagCapacity(int32 InMagCapacity) { MagCapacity = InMagCapacity; }

    EWeaponType GetWeaponType() { return WeaponType; }
    USoundBase* GetEquipSound() { return EquipSound; }

    bool IsFull() { return Ammo >= MagCapacity; }
    bool IsEmpty() { return Ammo <= 0; }

    bool GetIsDefaultWeapon() { return bIsDefaultWeapon; }
    void SetIsDefaultWeapon(bool InIsDefaultWeapon) { bIsDefaultWeapon = InIsDefaultWeapon; }
    EFireType GetFireType() { return FireType; }
    bool GetUseScatter() { return bUseScatter; }
    float GetDamge() { return Damage; }
    bool GetUseServerSideRequest() { return bUseServerSideRewind; }
};
