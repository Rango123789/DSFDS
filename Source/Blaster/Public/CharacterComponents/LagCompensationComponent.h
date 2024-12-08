// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FBoxInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;
	UPROPERTY()
	FRotator Rotation;
	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()
	//the time before, after and when you see you hit other char in your local machine is absolutely important for server-side rewind technique.
	//this time should be the 'server' time (meaning if you're a client, you need to synch the time with the server like we did in last lesson)
	UPROPERTY()
	float Time;

	//Knowing creating an array to store BoxInformations alone is not enough, you must know which BoxInformation belongs to which bone, hence TMap is useful:
	UPROPERTY()
	TMap<FName, FBoxInfo> BoxInfoMap;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
/***functions***/
//category1: auto-generated functions:
	// Sets default values for this component's properties
	ULagCompensationComponent();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
//category2: virtual functions:
	/**<Actor>*/

	/**</Actor>*/

	/**<X>*/

	/**</X>*/

//category3: regular functions: 
	//montages:

	//sound and effects:

	//bool functions:

	//BP-callale functions:
//category4: callbacks 

protected: //base	
/***functions***/
//category1: auto-generated functions:
	// Called when the game starts
	virtual void BeginPlay() override;
//category2: virtual functions:
	/**<Actor>*/

	/**</Actor>*/

	/**<X>*/

	/**</X>*/

//category3: regular functions 
	void SaveFramePackage(FFramePackage& FramePackage); //this means to modify input
	void ShowFramePackage(const FFramePackage& FramePackage, const FColor& Color); //this means to use input to draw things
//category4: callbacks

/***data members****/
//Category1: Enums , arrays, pointers to external classes
	//enum states:

	//pointer to external classes:
	
	UPROPERTY()
	class ABlasterCharacter* Character;
	UPROPERTY()
	class ABlasterPlayerController* Controller;
	//arrays, list, map:
	TDoubleLinkedList<FFramePackage> FramePackageList;

	//TLinkedList<FFramePackage> TEST1;
	//TIntrusiveLinkedList<FFramePackage> TEST2;
	//TList< FFramePackage> TEST3
	//class type:

//category2: UActorComponents   

//category3: Engine types      
	//montages:

	//sound and effects:

//category4: basic and primitive types
	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;


	friend class ABlasterCharacter;
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
