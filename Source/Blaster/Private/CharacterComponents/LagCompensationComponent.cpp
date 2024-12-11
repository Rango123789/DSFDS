// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterComponents/LagCompensationComponent.h"
#include "Characters/BlasterCharacter.h"
#include "PlayerController/BlasterPlayerController.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Weapons/Weapon.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	


}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SaveFramePackageList();
}

void ULagCompensationComponent::SaveFramePackageList()
{
	//we never get to use the DATA from client, so no point saving it in client!
	if (Character == nullptr || !Character->HasAuthority()) return;

	//can factorize these into a sub function:
	FFramePackage FramePackage;
	SaveFramePackage(FramePackage);

	//UPDATE: current convention of grow direction: [tail -> head]  (doesn't matter C++ usual order, but match the timelife history and so many other reasons, so I choose to follow stephen)
	//either one of if,else if, else could be executed per frame; no matter which one is added, we always add
	//you can in fact merge the first two into into .AddHead() - [tail->head] = STEPHEN
	//add first tail (0)  when there is no element (or just one element = head = tail)
	if (FramePackageList.Num() == 0)
	{
		FramePackageList.AddTail(FramePackage);
	}
	//add first head (1) when there is already one element, meaning we choose this convention: tail (old)-> head (new) ; then continue to remove tail(old) and add head(new) when it goes beyond MaxTimeRecord
	else if (FramePackageList.Num() == 1)
	{
		FramePackageList.AddHead(FramePackage);
	}
	else
	{
		//our current convention is tail LATER, so Time of HEAD must be bigger!
		//after we know that we surely have 2+ elements when enter this, stephen dont even check "GetHead() or GetTail() nullptr or not" - not sure this is safe? 
		float HistoryLength =
			FramePackageList.GetHead()->GetValue().Time - FramePackageList.GetTail()->GetValue().Time;
		//we remove head (or a number of them) until HistoryLength <= MaxRecordTime
		while (HistoryLength > MaxRecordTime)
		{
			FramePackageList.RemoveNode(FramePackageList.GetTail());
			//after we remove one head (old), we recount the time to see if we should need to remove more:
			HistoryLength =
				FramePackageList.GetHead()->GetValue().Time - FramePackageList.GetTail()->GetValue().Time;
		}

		//After that we always add 'the LASTEST FramePackage' of the current frame (could make HistoryLength go beyond MaxRecordTime again, but it doesn't matter, it is just one frame, and it will be removed the tail to compensate in next frame in the while loop above anyway:
		FramePackageList.AddHead(FramePackage);
	}

	//Draw FramePackage per frame
	//this work is absolutely indendendent from how we store above lol, we simply draw 'FramePackage' after the first 2 lines of code! so it doesn't tell whether your storing work is correct
	// So you may need to 'Draw per frame' and interator over the list back to draw all elements of it to see if you're storing it correctly, but anyway we will know in next lesson, so I will folow stephen anway
	// This is expensive, so do not use bPersistent true, let the LifeTime = MaxRecordTime is alsolute perfect!

	//ShowFramePackage(FramePackage, FColor::Orange);
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

	//FFramePackage FramePackage;
	//SaveFramePackage(FramePackage);
	//ShowFramePackage(FramePackage, FColor::Orange);











}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& FramePackage)
{
	//Stephen now confirm that owner of UActorComponent is the hosting actor having it as component (created by CreateDefaultSubobject)
	//Anyway since we did set it in Char::PostInitializeComponents, and Combat didn't even need to re-check this so I think this is REDUDANT for me:
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : Character;
	if (Character == nullptr || GetWorld() == nullptr) return;

	//FramPackage::  Time + TMap<FName, FBoxInfor> ; FBoxInfo = Localtion + Rotation + BoxExtent 
		//Sub member1: Time
			//we only plan to do it from server, so simply do 'GetWorld()->GetTimeSeconds()' will be enough, rather than call PC::GetServerTime() in worst case for any device!
	FramePackage.Time = GetWorld()->GetTimeSeconds();
	//sub member2: TMap<FName, FBoxInfor> , so we need to add elements for the map many times

	FBoxInfo BoxInfo;
	//BoxComponentMap is setup from Char, and all DATA is ready.
	for (const auto& Pair : Character->BoxComponentMap)
	{
		//this is SUB struct, hence we create a temp to store it before assign it to GLOBAL struct
		BoxInfo.Location = Pair.Value->GetComponentLocation();
		BoxInfo.Rotation = Pair.Value->GetComponentRotation();
		BoxInfo.BoxExtent = Pair.Value->GetScaledBoxExtent();
		//the key is the key of BoxComponentMap from Char itself, hell yeah!
		FramePackage.BoxInfoMap.Add(Pair.Key, BoxInfo);
	}
}

//we only need FramePackage.BoxInfoMap to draw them, not need .Time:
void ULagCompensationComponent::ShowFramePackage(const FFramePackage& FramePackage, const FColor& Color)
{
	for (const auto& Pair : FramePackage.BoxInfoMap)
	{
		//For now bPersisten = true, but next lesson Do not draw persisten line that cost performance, Draw it for 4s that is the time of Frame History
		//DrawDebugBox(GetWorld(), Pair.Value.Location, Pair.Value.BoxExtent, FQuat(Pair.Value.Rotation), Color, true, -1.f, 0 , 1.f); 
		DrawDebugBox(GetWorld(), Pair.Value.Location, Pair.Value.BoxExtent, FQuat(Pair.Value.Rotation), Color, false, 4.f, 0, 1.f); //false, 4.f
	}
}

//you need to pass in HitTime = GetServerTime - RTT/2   (OR GetServerTime - RTT, up to you)
void ULagCompensationComponent::ServerScoreRequest_Implementation(const FVector_NetQuantize& Start, const FVector_NetQuantize& HitLocation, ABlasterCharacter* HitCharacter, const float& HitTime, AWeapon* DamageCauser)
{
	if (HitCharacter == nullptr || DamageCauser == nullptr || Character == nullptr) return;
	//at this point, except params to be passed from RPCWrapper, any other values inside the function will use values in the server proxies, including 'Char::LagComp::FrameHistory':
	TPair<bool, bool> Result =  ServerSideRewind(Start, HitLocation, HitCharacter, HitTime);

	if (Result.Key == true)
	{
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			DamageCauser->GetDamge(),
			Character->GetController(),
			DamageCauser,
			UDamageType::StaticClass()		
		);
	}
}

//this function will be executed in the server, either restrict other devices or by sending ServerRPC
//remember consider ( - ping/2 or -ping) when you pass in HitTime, so GetServerTime() - ping/2 is perfect!
TPair<bool, bool>  ULagCompensationComponent::ServerSideRewind(const FVector_NetQuantize& Start, const FVector_NetQuantize& HitLocation, ABlasterCharacter* HitCharacter, const float& HitTime)
{
	//Overal condition:
	if ( GetWorld() == nullptr ||
		HitCharacter == nullptr || HitCharacter->GetLagComponent() == nullptr ||
		HitCharacter->GetLagComponent()->FramePackageList.GetTail() == nullptr ||
		HitCharacter->GetLagComponent()->FramePackageList.GetHead() == nullptr)
		return TPair<bool, bool>(false, false);

//STAGE1: find the WANTED/Interpolated FFramePackage
		//you need to use the HitCharacter::LagComponent::FramePackageList - not ::FramePackageList of this Attacking Chacter LOL:
			//didn't work:
		//TDoubleLinkedList<FFramePackage> HitFramePackageList = HitCharacter->GetLagComponent()->FramePackageList;
			//this work:
	const TDoubleLinkedList<FFramePackage>& HitFramePackageList = HitCharacter->GetLagComponent()->FramePackageList;

	FFramePackage FrameToCheck;

	float OldestTime = HitFramePackageList.GetTail()->GetValue().Time;
	float YoungestTime = HitFramePackageList.GetHead()->GetValue().Time;

	//you can in fact separate them out, however I like to keep it like this for now:
	if (HitTime < OldestTime)
	{
		return TPair<bool, bool>(false, false); //do thing next, you're too lag for the server to consider
	}
	else if (HitTime == OldestTime)
	{
		FrameToCheck = HitFramePackageList.GetTail()->GetValue();
	}
	//should be the server (> is recommended, because who know the time the server record laste frame package and the time server hit other char can be slight later (even more clear if it is projectile weapon - irrelevant)
	else if (YoungestTime <= HitTime)
	{
		FrameToCheck = HitFramePackageList.GetHead()->GetValue();
	}
	//use 'else' is anought, but I want to make it clear:
	else if (OldestTime < HitTime && HitTime < YoungestTime)
	{
		//because 'HitFramePackageList.GetHead()' not null (check at first place) so other = it can't be null, so no need to check according to universal rule:
		TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* YoungerNode = HitFramePackageList.GetHead();
		TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* OlderNode = YoungerNode;

		//step1: after this step you got a perfect position if Younger and Older for step2:
		while (OlderNode->GetValue().Time > HitTime)
		{
			//I forget this rather important line, you want to break; not return, because there is also STAGE2:
			if (HitFramePackageList.GetHead()->GetNextNode() == nullptr) break;

			//if the 'next to the tail' node is still in bounds, then we can assign it as new node for OlderNode:
			OlderNode = HitFramePackageList.GetHead()->GetNextNode();

			//after receive new value, it has smaller time, but if it 'STILL' greater than HitTime, then move the YoungerNode back with it, ready for the next iteration , otherwise it will escape the loop in a perpect position:
			if (OlderNode->GetValue().Time > HitTime)
			{
				YoungerNode = OlderNode;
			}
		}

		//step2: after escape the while loop, we know what possible of Younger Node and OlderNode could be:
		if (OlderNode->GetValue().Time == HitTime)
		{
			FrameToCheck = OlderNode->GetValue(); 
		}
		//stephen use 'bShouldInterpolate' but it is totally redudant LOL:
		else if (OlderNode->GetValue().Time < HitTime)
		{
			//interpolate between YoungerNode.FramePackage and OlderNode.FramePackage will give you the best FramePackage_ForNextStage: (NEXT LESSON)
			FrameToCheck = 
				InterpBetweenFrames(YoungerNode->GetValue(), OlderNode->GetValue(), HitTime);
		}
	}
//STAGE2: use the 'FramePackage_ForNextStage', perhaps use it to move Char::Boxes to and then Trace against it, after that move it back:
	return ConfirmHit(HitCharacter, FrameToCheck, Start, HitLocation);

}

TPair<bool, bool> ULagCompensationComponent::ConfirmHit(ABlasterCharacter* HitCharacter, FFramePackage& FrameToCheck, const FVector_NetQuantize& Start, const FVector_NetQuantize& HitLocation)
{
	//step1: CacheBoxes() - save their bare-bones information into FFramePackage
	FFramePackage CurrentFrame;
	//You must call it from HitCharacter, not Directly call 'SaveFramePackage(CurrentFrame)' that is of this Attacking Char: 
	HitCharacter->GetLagComponent()->SaveFramePackage(CurrentFrame);
	//step2: MoveBoxes, enable HeadBox -> trace, enable the rest -> trace
	MoveBoxes(FrameToCheck, HitCharacter);

	UBoxComponent* HeadBox = HitCharacter->BoxComponentMap[FName("head")];

	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	//MISSING step: the trace is done in the server in REAL time and a lot of chars may move around and block the trace (meant for the past), so you want to avoid it as well!
	//this will be enabled back in RestoreBoxes as well (hence no need to worry to reset it):
	if (HitCharacter->GetMesh())
	{
		Character->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HitCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	//i think we should set this to Block Visisbility even from Char::BoxComp_i, avoiding so much work in real time!
	//UPDATE: now it has been set from there!
	//HeadBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	FHitResult HitResult;
	FVector End = Start + (HitLocation - Start) * 1.25;
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility);

	//Stephen didn't have && Cast<ABlasterCharacter>(HitResult.GetActor()), but we need it;
	if (HitResult.bBlockingHit && Cast<ABlasterCharacter>(HitResult.GetActor()) )
	{
		//resetBoxes to where it was and return early:
		ResetBoxes(CurrentFrame, HitCharacter);
		return TPair<bool, bool>(true, true); //hit, headshot
	}

	//if it reach this line, we doesn't have head shot:
	//turn on Collision for the rest box (not that it is set to block Visibility in Char already:
	for (auto& Pair : HitCharacter->BoxComponentMap)
	{
		Pair.Value->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility);

	//no matter we hit any of the rest or not, we always need to ResetBoxes:
	ResetBoxes(CurrentFrame, HitCharacter);

	if (HitResult.bBlockingHit && Cast<ABlasterCharacter>(HitResult.GetActor()))
	{
		return TPair<bool, bool>(true, false); //hit, not headshot
	}

	return TPair<bool, bool>(false, false); //not hit, not headshot

}

//enter this function we surely know OlderFrame.Timer < HitTime < YoungerFrame.Time
FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, const float& HitTime)
{
	FFramePackage InterpFrame;

//step1: assign the Time for TIRE1
	InterpFrame.Time = HitTime;

//step2: assign .BoxInfoMap for TIRE1 (each element is Pair<Fname, FBoxInfo>, So we should need create a TemFBoxInfo TIRE3, you may want to create TempBoxInfoMap TIRE2 as well, but no need LOL)

	//bring it out for better performance.
	const float Factor = FMath::Clamp( (HitTime - OlderFrame.Time) / (YoungerFrame.Time - OlderFrame.Time), 0.f, 1.f);

	//We only need to iterate over either one of the Frame.BoxInfoMap, the 2nd one will use direct access feature of map to find the right box:
	//for each box to interp between .submember1 and .submember2 and then assign it for 'InterpFrame', to be return
	for (const auto& Pair : OlderFrame.BoxInfoMap)
	{
		const FVector Location1 = Pair.Value.Location;
		//this is the KEY to solve the problem, it will interate through the second map and find the element matching the key of the first map.
		//they must share the same key, so we dont even need to check YoungerFrame.BoxInfoMap.Contain(Pair.Key):
		const FVector Location2 = YoungerFrame.BoxInfoMap[Pair.Key].Location;

		const FRotator Rotation1 = Pair.Value.Rotation;
		const FRotator Rotation2 = YoungerFrame.BoxInfoMap[Pair.Key].Rotation;
		
		//According to UNIVERSAL rule, we need to create a TIRE2 temp var, before we can assign it for TIRE1:
		FBoxInfo TempInterpBoxInfo;

		TempInterpBoxInfo.Location = FMath::VInterpTo(Location1, Location2, 1.f, Factor);
		TempInterpBoxInfo.Rotation = FMath::RInterpTo(Rotation1, Rotation2, 0.5f, Factor);
		
		//this dont need to be interpolated, for a specific box in the box map, younger.box_i and older.box_i of the same key always has the same size.
		TempInterpBoxInfo.BoxExtent = Pair.Value.BoxExtent;

		//finally we add each element PER iteration:
		InterpFrame.BoxInfoMap.Add(Pair.Key, TempInterpBoxInfo);
	}
	return InterpFrame;
}

//You can in fact move it into FrameToCheck, and then Move it back to CurentFrame/BackupFrame using this very same function, but you may want to do it separately as Stephen also include 'Disable collision part' when move it back:
void ULagCompensationComponent::MoveBoxes(const FFramePackage& FrameToMoveTo, ABlasterCharacter* HitCharacter)
{
	if (HitCharacter == nullptr) return;

	for (auto& Pair : HitCharacter->BoxComponentMap)
	{
		const FVector& BoxLocation = FrameToMoveTo.BoxInfoMap[Pair.Key].Location;
		const FRotator& BoxRotation = FrameToMoveTo.BoxInfoMap[Pair.Key].Rotation;
		const FVector& BoxExtent = FrameToMoveTo.BoxInfoMap[Pair.Key].BoxExtent;

		Pair.Value->SetWorldLocation(BoxLocation);
		Pair.Value->SetWorldRotation(BoxRotation);
		Pair.Value->SetBoxExtent(BoxExtent); //this is absolutely not needed in our case since our box was there and didn't change its size at all, however in the case of Capsule, we may need this
	}
}

void ULagCompensationComponent::ResetBoxes(const FFramePackage & FrameToRestoreTo, ABlasterCharacter* HitCharacter)
{
	if (HitCharacter == nullptr) return;

	for (auto& Pair : HitCharacter->BoxComponentMap)
	{
		const FVector& BoxLocation = FrameToRestoreTo.BoxInfoMap[Pair.Key].Location;
		const FRotator& BoxRotation = FrameToRestoreTo.BoxInfoMap[Pair.Key].Rotation;
		const FVector& BoxExtent = FrameToRestoreTo.BoxInfoMap[Pair.Key].BoxExtent;

		Pair.Value->SetWorldLocation(BoxLocation);
		Pair.Value->SetWorldRotation(BoxRotation);
		Pair.Value->SetBoxExtent(BoxExtent); //this is absolutely not needed in our case since our box was there and didn't change its size at all, however in the case of Capsule, we may need this

		Pair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	Character->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); //QueryOnly?
	HitCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); //QueryOnly?
}



////my way different in the while loop, but stephen is better, MORE performance!
//void ULagCompensationComponent::ServerSideRewind(const FVector_NetQuantize& Start, const FVector_NetQuantize& HitLocation, ABlasterCharacter* HitCharacter, float HitTime)
//{
//	//Overal condition:
//	if (HitCharacter == nullptr || HitCharacter->GetLagComponent() == nullptr ||
//		HitCharacter->GetLagComponent()->FramePackageList.GetTail() == nullptr ||
//		HitCharacter->GetLagComponent()->FramePackageList.GetHead() == nullptr)
//		return;
//
//	//STAGE1: find the WANTED/Interpolated FFramePackage
//		//you need to use the HitCharacter::LagComponent::FramePackageList - not ::FramePackageList of this Attacking Chacter LOL:
//			//didn't work:
//		//TDoubleLinkedList<FFramePackage> HitFramePackageList = HitCharacter->GetLagComponent()->FramePackageList;
//			//this work:
//	const TDoubleLinkedList<FFramePackage>& HitFramePackageList = HitCharacter->GetLagComponent()->FramePackageList;
//
//	FFramePackage FramePackage_ForNextStage;
//
//	float OldestTime = HitFramePackageList.GetTail()->GetValue().Time;
//	float YoungestTime = HitFramePackageList.GetHead()->GetValue().Time;
//
//	if (HitTime < OldestTime)
//	{
//		return; //do thing next, you're too lag for the server to consider
//	}
//	else if (HitTime = OldestTime)
//	{
//		FramePackage_ForNextStage = HitFramePackageList.GetTail()->GetValue();
//	}
//	//should be the server (> is recommended, because who know the time the server record laste frame package and the time server hit other char can be slight later (even more clear if it is projectile weapon - irrelevant)
//	else if (YoungestTime <= HitTime)
//	{
//		FramePackage_ForNextStage = HitFramePackageList.GetHead()->GetValue();
//	}
//	//use 'else' is anought, but I want to make it clear:
//	else if (OldestTime < HitTime && HitTime < YoungestTime)
//	{
//		//because 'HitFramePackageList.GetHead()' not null (check at first place) so other = it can't be null, so no need to check according to universal rule:
//		TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* YoungerNode = HitFramePackageList.GetHead();
//		TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* OlderNode = YoungerNode;
//
//		while (OlderNode->GetValue().Time > HitTime)
//		{
//			//step1: if time in OlderNode > Hittime, then let it point to older element (that is 'Next' in stephen's convention LOL): THIS MUST BE A 'SEPARATE' IF
//			if (OlderNode->GetValue().Time > HitTime) //this is redudant as we just check it in while() lol, so you can remove this line!
//			{
//				//I forget this rather important line, you want to break; not return, because there is also STAGE2:
//				if (HitFramePackageList.GetHead()->GetNextNode() == nullptr) break;
//				//if the 'next to the tail' node is still in bounds, then we can assign it as new node for OlderNode:
//				OlderNode = HitFramePackageList.GetHead()->GetNextNode();
//			}
//
//			//step2: now it got new DATA, check if it is now equal (rare but possible):
//			if (OlderNode->GetValue().Time == HitTime)
//			{
//				FramePackage_ForNextStage = OlderNode->GetValue(); break;
//			}
//			//step3: if it didn't equal, but it 'STILL' greater than HitTime, then move the YoungerNode back with it, ready for the next iteration LOL:
//			else if (OlderNode->GetValue().Time > HitTime)
//			{
//				YoungerNode = OlderNode;
//			}
//			//step4: if it happen to smaller than HitTime, then now: Time in Older <  HitTime < Time in Younger, that's where we need to interpolate ( we create extra function for this purpose in next lesson):
//			else if (OlderNode->GetValue().Time < HitTime)
//			{
//				//interpolate between YoungerNode.FramePackage and OlderNode.FramePackage will give you the best FramePackage_ForNextStage: (NEXT LESSON)
//
//			}
//		}
//
//
//
//	}
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//}

