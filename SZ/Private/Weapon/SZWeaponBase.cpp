// Fill out your copyright notice in the Description page of Project Settings.


#include "SZWeaponBase.h"
#include "SZCharacter.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "SZClipBase.h"



// Sets default values
ASZWeaponBase::ASZWeaponBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//Mesh1P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("MyWeaponMesh1P"));
	//Mesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	//Mesh1P->bReceivesDecals = false;
	//Mesh1P->CastShadow = false;
	//Mesh1P->SetCollisionObjectType(ECC_WorldDynamic);
	//Mesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//Mesh1P->SetCollisionResponseToAllChannels(ECR_Ignore);
	//RootComponent = Mesh1P;

	WeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponCollision"));
	WeaponCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	WeaponCollision->SetGenerateOverlapEvents(true);
	WeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &ASZWeaponBase::OnComponentBeginOverlap);
	RootComponent = WeaponCollision;

	Mesh3P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("Mesh3P"));
	Mesh3P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Mesh3P->bReceivesDecals = false;
	Mesh3P->CastShadow = true;
	Mesh3P->SetupAttachment(WeaponCollision);





	


	bLoopedMuzzleFX = false;
	bLoopedFireSound = false;
	bLoopedFireAnim = false;
	bPlayingFireAnim = false;
	bIsEquipped = false;
	bWantsToFire = false;
	//bPendingEquip = false;
	CurrentState = EWeaponState::Idle;

	//CurrentAmmo = 0;
	//CurrentAmmoInClip = 0;
	BurstCounter = 0;
	LastFireTime = 0.0f;

	

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	bNetUseOwnerRelevancy = true;

	PickupTexture = CreateDefaultSubobject<UTexture2D>(FName("ItemTexture"));
}



void ASZWeaponBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	//if (WeaponConfig.InitialClips > 0)
	//{
	//	CurrentAmmoInClip = WeaponConfig.AmmoPerClip;
	//	CurrentAmmo = WeaponConfig.AmmoPerClip * WeaponConfig.InitialClips;
	//}

	
	Mesh3P->HideBoneByName(TEXT("b_gun_mag"), PBO_None);
	//CurrentClip->AttachMeshToWeapon();

}

void ASZWeaponBase::Destroyed()
{
	Super::Destroyed();

	StopSimulatingWeaponFire();
}



//////////////////////////////////////////////////////////
//Inventory


//////////////////////////////////////////////////////////
//Input

void ASZWeaponBase::StartFire()
{
	if (Role < ROLE_Authority)
	{
		ServerStartFire();
	}

	if (!bWantsToFire)
	{
		bWantsToFire = true;
		DetermineWeaponState();
	}
}

void ASZWeaponBase::StopFire()
{
	if ((Role < ROLE_Authority) && MyPawn && MyPawn->IsLocallyControlled())
	{
		ServerStopFire();
	}

	if (bWantsToFire)
	{
		bWantsToFire = false;
		DetermineWeaponState();
	}
}

void ASZWeaponBase::StartReload()
{
	if (Role < ROLE_Authority)
	{
		ServerStartReload();
	}

	CurrentClip->AttachMeshToWeapon(this);

	if (MyPawn&&MyPawn->IsLocallyControlled())
	{
		PlayWeaponSound(ReloadSound);
	}

	//Mesh3P->UnHideBoneByName(TEXT("b_gun_mag"));


}



bool ASZWeaponBase::ServerStartFire_Validate()
{
	return true;
}

void ASZWeaponBase::ServerStartFire_Implementation()
{
	StartFire();
}

bool ASZWeaponBase::ServerStopFire_Validate()
{
	return true;
}

void ASZWeaponBase::ServerStopFire_Implementation()
{
	StopFire();
}

bool ASZWeaponBase::ServerStartReload_Validate()
{
	return true;
}

void ASZWeaponBase::ServerStartReload_Implementation()
{
	StartReload();
}

//////////////////////////////////////////////////////////////////////////
//Control

bool ASZWeaponBase::CanFire() const
{
	bool bCanFire = false;
	bool bStateOkToFire = true;
	
	if (CurrentClip&& CurrentClip->IsAttachedToWeapon())
	{
		bCanFire = true;

		if (CurrentClip->GetCurrentAmmoInClip() ==0)
		{
			bStateOkToFire = false;
		}
	}
	 
	return ((bCanFire == true) && (bStateOkToFire == true));
}

//////////////////////////////////////////////////////////////////////////
// Weapon usage
void ASZWeaponBase::GiveAmmo(int AddAmount)
{
	//const int32 MissingAmmo = FMath::Max(0, WeaponConfig.MaxAmmo - CurrentAmmo);
	//AddAmount = FMath::Min(AddAmount, MissingAmmo);
	//CurrentAmmo += AddAmount;

}

void ASZWeaponBase::UseAmmo()
{
	if (CurrentClip)
	{
		CurrentClip->UseAmmo();
	}
}


void ASZWeaponBase::HandleReFiring()
{
	UWorld* MyWorld = GetWorld();

	float SlackTimeThisFrame = FMath::Max(0.0f, (MyWorld->TimeSeconds - LastFireTime) - WeaponConfig.TimeBetweenShots);

	if (bAllowAutomaticWeaponCatchup)
	{
		TimerIntervalAdjustment -= SlackTimeThisFrame;
	}

	HandleFiring();
}

void ASZWeaponBase::HandleFiring()
{

	if (CanFire())//(CurrentClip->getCurrentAmmoInClip > 0 || HasInfiniteClip() || HasInfiniteAmmo()) && CanFire())
	{
		if (GetNetMode() != NM_DedicatedServer)
		{
			SimulateWeaponFire();
		}

		if (MyPawn->IsLocallyControlled())
		{
			FireWeapon();
			UseAmmo();
			BurstCounter++;
		//	UE_LOG(LogClass, Warning, TEXT("%d"), GetCurrentAmmo());
		}
	}
	else if(MyPawn)
	{
		if (!bRefiring)//GetCurrentAmmo() == 0)
		{
			PlayWeaponSound(OutOfAmmoSound);
	
		}
	
		if (BurstCounter > 0)
		{
			OnBurstFinished();
		}
	}
	
	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		// local client will notify server
		if (Role < ROLE_Authority)
		{
			ServerHandleFiring();
		}

		// setup refire timer
		bRefiring = (CurrentState == EWeaponState::Firing && WeaponConfig.TimeBetweenShots > 0.0f);
		if (bRefiring)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &ASZWeaponBase::HandleFiring,
				FMath::Max<float>(WeaponConfig.TimeBetweenShots + TimerIntervalAdjustment, SMALL_NUMBER), false);
			TimerIntervalAdjustment = 0.0f;
		}
	}

	LastFireTime = GetWorld()->GetTimeSeconds();
}

bool ASZWeaponBase::ServerHandleFiring_Validate()
{
	return true;
}

void ASZWeaponBase::ServerHandleFiring_Implementation()
{
	const bool bShouldUpdateAmmo = (CurrentClip->GetCurrentAmmoInClip() > 0 && CanFire());

	HandleFiring();

	if (bShouldUpdateAmmo)
	{
		UseAmmo();

		BurstCounter++;
	}
}

void ASZWeaponBase::SetWeaponState(EWeaponState::Type NewState)
{
	const EWeaponState::Type PrevState = CurrentState;

	if (PrevState == EWeaponState::Firing && NewState != EWeaponState::Firing)
	{
		OnBurstFinished();
	}

	CurrentState = NewState;

	if (PrevState != EWeaponState::Firing && NewState == EWeaponState::Firing)
	{
		OnBurstStarted();
	}
}

void ASZWeaponBase::DetermineWeaponState()
{
	EWeaponState::Type NewState = EWeaponState::Idle;

	
	if (bWantsToFire)//&& CanFire() == true)
	{
		NewState = EWeaponState::Firing;
	}

	SetWeaponState(NewState);
}

void ASZWeaponBase::OnBurstStarted()
{

	const float GameTime = GetWorld()->GetTimeSeconds();
	if (LastFireTime > 0 && WeaponConfig.TimeBetweenShots > 0.0f &&
		LastFireTime + WeaponConfig.TimeBetweenShots > GameTime)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &ASZWeaponBase::HandleFiring, LastFireTime + WeaponConfig.TimeBetweenShots - GameTime, false);
	}
	else
	{
		HandleFiring();
	}
}

void ASZWeaponBase::OnBurstFinished()
{
	BurstCounter = 0;

	if (GetNetMode() != NM_DedicatedServer)
	{
		StopSimulatingWeaponFire();
	}

	GetWorldTimerManager().ClearTimer(TimerHandle_HandleFiring);
	bRefiring = false;

	TimerIntervalAdjustment = 0.0f;
}

//////////////////////////////////////////////////////////////////////////
// Weapon usage helpers


UAudioComponent * ASZWeaponBase::PlayWeaponSound(USoundBase * Sound)
{
	UAudioComponent* AC = NULL;
	
	if (Sound)
	{
		AC = UGameplayStatics::SpawnSoundAttached(Sound, MyPawn->GetRootComponent());
		//	GetWeaponMesh()->GetSocketLocation(MuzzleAttachPoint));
	}

	return AC;
}

FVector ASZWeaponBase::GetWeaponAim() const
{
	FVector FinalAim = FVector::ZeroVector;
	USkeletalMeshComponent* UseMesh = GetWeaponMesh();
	FVector WeaponLoc;
	FRotator WeaponRot;
	//UseMesh->GetSocketLocation(MuzzleAttachPoint).ForwardVector;
	UseMesh->GetSocketWorldLocationAndRotation(MuzzleAttachPoint, WeaponLoc, WeaponRot);
	
	
	FinalAim = WeaponRot.Vector();
	

	return FinalAim;
}

FVector ASZWeaponBase::GetWeaponDamageStartLocation(const FVector & AimDir) const
{
	FVector OutStartTrace = FVector::ZeroVector;

	OutStartTrace = GetWeaponMesh()->GetSocketLocation(MuzzleAttachPoint);

	OutStartTrace = OutStartTrace + AimDir * ((GetWeaponMesh()->GetSocketLocation(MuzzleAttachPoint) - OutStartTrace) | AimDir);

	//OutStartTrace = GetMuzzleLocation();
	return OutStartTrace;
}

FVector ASZWeaponBase::GetMuzzleLocation() const
{
	USkeletalMeshComponent* UseMesh = GetWeaponMesh();
	return UseMesh->GetSocketLocation(MuzzleAttachPoint);
}

FVector ASZWeaponBase::GetMuzzleDirection() const
{
	USkeletalMeshComponent* UseMesh = GetWeaponMesh();
	return UseMesh->GetSocketRotation(MuzzleAttachPoint).Vector();
}

FHitResult ASZWeaponBase::WeaponTrace(const FVector & TraceFrom, const FVector & TraceTo) const
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_PhysicsBody));
	
	TArray<AActor*> IgnoreActors;
	//IgnoreActors.Add(this);
	

	FHitResult OutHit;

	//FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, Instigator);
	//TraceParams.bReturnPhysicalMaterial = true;

	//FHitResult Hit(ForceInit);
	//GetWorld()->LineTraceSingleByChannel(Hit, TraceFrom, TraceTo, COLLISION_WEAPON, TraceParams);

	bool IsResult = UKismetSystemLibrary::LineTraceSingleForObjects(
		GetWorld(),
		TraceFrom,
		TraceTo,
		ObjectTypes,
		false,
		IgnoreActors,
		EDrawDebugTrace::None,
		OutHit,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		5.0f
	);

	//UE_LOG(LogClass, Warning, TEXT("%s"), *OutHit.BoneName.ToString());

	return OutHit;
}

FName ASZWeaponBase::GetClipAttachPoint() const
{
	return ClipAttachPoint;
}

void ASZWeaponBase::SetEquipping()
{

}

void ASZWeaponBase::OnComponentBeginOverlap(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (OtherActor->ActorHasTag(TEXT("Ammo")))
	{
		auto NewClip = Cast<ASZClipBase>(OtherActor);
		if (NewClip->bAttachedHand == true)
		{
			auto Character = Cast<ASZCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
			Character->ReloadOverlap(OtherActor);
		}
	}
}

bool ASZWeaponBase::GetEquip()
{
	return IsEquip;
}



//////////////////////////////////////////////////////////////////////////
// Replication & effects

void ASZWeaponBase::OnRep_MyPawn()
{
}

void ASZWeaponBase::OnRep_BurstCounter()
{
	if (BurstCounter > 0)
	{
		SimulateWeaponFire();
	}
	else
	{
		StopSimulatingWeaponFire();
	}
}

void ASZWeaponBase::OnRep_CurrentClip()
{
}

void ASZWeaponBase::SimulateWeaponFire()
{
	if (Role == ROLE_Authority && CurrentState != EWeaponState::Firing)
	{
		return;
	}

	if (MuzzleFX)
	{
		USkeletalMeshComponent* UseWeaponMesh = GetWeaponMesh();
		if (!bLoopedMuzzleFX || MuzzlePSC == NULL)
		{
			Mesh3P->GetSocketLocation(MuzzleAttachPoint);
			MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, UseWeaponMesh, MuzzleAttachPoint);
		}
	}

	//if (!bLoopedFireAnim || !bPlayingFireAnim)
	//{
	//	PlayWeaponAnimation(FireAnim);
	//	bPlayingFireAnim = true;
	//}

	if (bLoopedFireSound)
	{
		if (FireAC == NULL)
		{
			FireAC = PlayWeaponSound(FireLoopSound);
		}
	}
	else
	{
		PlayWeaponSound(FireSound);
	}

	//shake
}



void ASZWeaponBase::StopSimulatingWeaponFire()
{
	if (bLoopedMuzzleFX)
	{
		if (MuzzlePSC != NULL)
		{
			MuzzlePSC->DeactivateSystem();
			MuzzlePSC = NULL;
		}
	}

	//if (bLoopedFireAnim && bPlayingFireAnim)
	//{
	//	//StopWeaponAnimation(FireAnim);
	//	bPlayingFireAnim = false;
	//}

	if (FireAC)
	{
		FireAC->FadeOut(0.1f, 0.0f);
		FireAC = NULL;

		PlayWeaponSound(FireFinishSound);
	}
}

void ASZWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASZWeaponBase, MyPawn);
	DOREPLIFETIME(ASZWeaponBase, CurrentClip);

	//DOREPLIFETIME_CONDITION(ASZWeaponBase, CurrentAmmo, COND_OwnerOnly);
	//DOREPLIFETIME_CONDITION(ASZWeaponBase, CurrentAmmoInClip, COND_OwnerOnly);

	DOREPLIFETIME_CONDITION(ASZWeaponBase, BurstCounter, COND_SkipOwner);


}


USkeletalMeshComponent * ASZWeaponBase::GetWeaponMesh() const
{
	return Mesh3P;
	//return (MyPawn != NULL && MyPawn->IsFirstPerson()) ? Mesh1P : Mesh3P;
}

class ASZCharacter* ASZWeaponBase::GetPawnOwner() const
{
	return MyPawn;
}


EWeaponState::Type ASZWeaponBase::GetCurrentState() const
{
	return CurrentState;
}

//int32 ASZWeaponBase::GetCurrentAmmo() const
//{
//	return CurrentAmmo;
//}

//int32 ASZWeaponBase::GetCurrentAmmoInClip() const
//{
//	return CurrentAmmoInClip;
//}

//int32 ASZWeaponBase::GetAmmoPerClip() const
//{
//	return 1;
//}

//int32 ASZWeaponBase::GetMaxAmmo() const
//{
//	return 1;
//}

//bool ASZWeaponBase::HasInfiniteAmmo() const
//{
//	return WeaponConfig.bInfiniteAmmo;
//}
//
//bool ASZWeaponBase::HasInfiniteClip() const
//{
//	return WeaponConfig.bInfiniteClip;
//}

void ASZWeaponBase::SetOwningPawn(ASZCharacter * NewOwner)
{
	if (MyPawn != NewOwner)
	{
		Instigator = NewOwner;
		MyPawn = NewOwner;
		// net owner for RPC calls
		SetOwner(NewOwner);
		IsEquip = true;
	}
}

void ASZWeaponBase::SetOwningClip(ASZClipBase * NewOwner)
{
	if (CurrentClip != NewOwner)
	{
		CurrentClip = NewOwner;
		//SetOwner(NewOwner);
	}
}



//void ASZWeaponBase::EquipWeapon(USceneComponent * AttachTo) // ������ ���
//{
//	Mesh3P->SetPhysics(false);
//	FAttachmentTransformRules AttachmentTransformRules(EAttachmentRule::KeepWorld, false);
//	GetRootComponent()->AttachToComponent(AttachTo, AttachmentTransformRules);
//}

