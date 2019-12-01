// Fill out your copyright notice in the Description page of Project Settings.


#include "SZCharacter.h"
#include "SZPlayerAnimInstance.h"
#include "SZWeaponBase.h"
#include "XRMotionControllerBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "SZCharacterListSetting.h" // �׽�Ʈ�� 1
#include "SZPlayerState.h"
#include "SZMotionHandController.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "SZClipBase.h"
#include "Enemy/SZAIController.h"
#include "Weapon/SZClipSpawner.h"
#include "Components/WidgetComponent.h"
#include "SZGameState.h"
#include "Blueprint/UserWidget.h"


// Sets default value 
ASZCharacter::ASZCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bAlwaysRelevant = true;
	bReplicateMovement = true;
	Health = 100.f;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SPRINGARM"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CAMERA"));

	VR_Origin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));

	SpringArm->SetupAttachment(RootComponent);
	Camera->SetupAttachment(SpringArm);

	SpringArm->SetRelativeRotation(FRotator(0.0f, 0.0f, -10.0f));

	ClipSpawn_Actor_M4 = CreateDefaultSubobject<UChildActorComponent>(TEXT("Clip_M4"));
	ClipSpawn_Actor_SMG = CreateDefaultSubobject<UChildActorComponent>(TEXT("Clip_SMG"));
	ClipSpawn_Actor_AKU = CreateDefaultSubobject<UChildActorComponent>(TEXT("Clip_AKU"));

	ClipSpawn_Actor_M4->SetupAttachment(GetMesh());
	ClipSpawn_Actor_SMG->SetupAttachment(GetMesh());
	ClipSpawn_Actor_AKU->SetupAttachment(GetMesh());

	ClipSpawn_Actor_M4->SetChildActorClass(ASZClipSpawner::StaticClass());
	ClipSpawn_Actor_M4->SetRelativeLocation(GetMesh()->GetSocketLocation(TEXT("AmmoSlot1")));
	ClipSpawn_Actor_SMG->SetChildActorClass(ASZClipSpawner::StaticClass());
	ClipSpawn_Actor_SMG->SetRelativeLocation(GetMesh()->GetSocketLocation(TEXT("AmmoSlot2")));
	ClipSpawn_Actor_AKU->SetChildActorClass(ASZClipSpawner::StaticClass());
	ClipSpawn_Actor_AKU->SetRelativeLocation(GetMesh()->GetSocketLocation(TEXT("AmmoSlot3")));

	const FTransform SpawnTransform = FTransform(FRotator(0.0f, 0.0f, 0.0f), FVector(0.0f, 0.0f, 0.0f), FVector(1.0f, 1.0f, 1.0f)); // = FTransform::Identity;

	FTransform OriginTransform = FTransform(FRotator(0.0f, 0.0f, 0.0f), FVector(0.f, 0.f, 0.0f), FVector(1.f, 1.f, 1.f));

	VR_Origin->SetWorldTransform(OriginTransform);
	VR_Origin->SetupAttachment(RootComponent);

	// �޽������� ������ ������ ��ġ�� ������ �� ������Ʈ ��ġ
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -88.0f), FRotator(0.0f, -90.0f, 0.0f));

	eStatecharacter = EStateCharacterEnum::Idle;
	IsGrab = false;

	GetCharacterMovement()->MaxWalkSpeed = 350; // �ӵ� 300���� ����.


	SelectItemNum = -1;
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Custom"));
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

	bIsFire = false;

	Tags.Add(FName(TEXT("Player")));

	AmmoCount_M4 = 5;
	AmmoCount_SMG = 5;
	AmmoCount_AKU = 5;

	//bBtnGrap = false;

	//bBtnFire = false;

	//bBtnChangeItem = false;
}

// Called when the game starts or when spawned
void ASZCharacter::BeginPlay()
{
	Super::BeginPlay();

	WeaponInventory.SetNum(MAX_INVENTORY_ITEMS);
	AmmoInventory.SetNum(MAX_INVENTORY_ITEMS);
	LastSeenItem = nullptr;
}

void ASZCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ASZPlayerState* PS = Cast<ASZPlayerState>(GetPlayerState());

	if (PS)
	{
		PS->SetMaxHealthFromCharacter(GetMaxHealth());
	}

	if (Role == ROLE_Authority)
	{
		Health = GetMaxHealth();

		if (PS)
		{
			PS->SetCurrentHealthFromCharacter(Health);
		}
	}

	bUsingMotionControllers = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();

	FName DeviceName = UHeadMountedDisplayFunctionLibrary::GetHMDDeviceName();

	if (bUsingMotionControllers) // ���� VR��尡 ������̸�
	{

		//const FTransform SpawnTransform = FTransform(FRotator(0.0f, -20.0f, 0.0f), FVector(10.0f, 0.0f, -5.0f), FVector(1.0f, 1.0f, 1.0f)); // = FTransform::Identity;
		//SpringArm->SetRelativeTransform(SpawnTransform);

		bEquipWeapon = false;

		if (DeviceName == "SteamVR" || DeviceName == "OculusHMD")
		{
			// Epic Comment :D // Windows (Oculus / Vive)
			UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Eye);

			SpringArm->AttachToComponent(VR_Origin,FAttachmentTransformRules::KeepRelativeTransform);// �̰ǰ�
			SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
			SpringArm->TargetArmLength = 0;
			SpringArm->SocketOffset = FVector(0, 0, 0);
			SpringArm->bUsePawnControlRotation = false;
		}
		CurrentWeapon = nullptr;

		const FTransform SpawnTransform2 = FTransform(FRotator(0.0f, 0.0f, 0.0f), FVector(0.0f, 0.0f, 0.0f), FVector(1.0f, 1.0f, 1.0f));
		FAttachmentTransformRules AttachRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false);

		MC_Left = GetWorld()->SpawnActorDeferred<ASZMotionHandController>(ASZMotionHandController::StaticClass(), SpawnTransform2, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		MC_Left->MotionController->MotionSource = FXRMotionControllerBase::LeftHandSourceId; // �������Ʈ�� Enum
		MC_Left->MotionController->SetCollisionProfileName(TEXT("NoCollision"));
		MC_Left->SZHand = EControllerHand::Left;
		MC_Left->FinishSpawning(SpawnTransform2);
		MC_Left->AttachToComponent(VR_Origin, AttachRules);
		MC_Left->GrabShpere->OnComponentBeginOverlap.AddDynamic(this, &ASZCharacter::OnBeginOverlapLeftHand);
		MC_Left->GrabShpere->OnComponentEndOverlap.AddDynamic(this, &ASZCharacter::OnEndOverlapLeftHand);

		MC_Right = GetWorld()->SpawnActorDeferred<ASZMotionHandController>(ASZMotionHandController::StaticClass(), SpawnTransform2, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		MC_Right->MotionController->SetCollisionProfileName(TEXT("NoCollision"));
		MC_Right->MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId; // �������Ʈ�� Enum �����̶� �� ������
		MC_Right->SZHand = EControllerHand::Right;
		MC_Right->FinishSpawning(SpawnTransform2);
		MC_Right->AttachToComponent(VR_Origin, AttachRules);
		UE_LOG(LogTemp, Error, TEXT("%f, %f, %f"), MC_Right->GetActorRotation().Pitch, MC_Right->GetActorRotation().Yaw, MC_Right->GetActorRotation().Roll);

		UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();

		//ClipSpawn_Actor_M4->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("AmmoSlot1"));
				//ClipSpawn_Actor_SMG->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("AmmoSlot2"));
		//ClipSpawn_Actor_AKU->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("AmmoSlot3"));


		//ClipSpawn_M4 = GetWorld()->SpawnActorDeferred<ASZClipSpawner>(ASZClipSpawner::StaticClass(), SpawnTransform2, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		//ClipSpawn_M4->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("AmmoSlot1"));


		//ClipSpawn_SMG = GetWorld()->SpawnActorDeferred<ASZClipSpawner>(ASZClipSpawner::StaticClass(), SpawnTransform2, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		//ClipSpawn_SMG->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("AmmoSlot2"));

		//ClipSpawn_AKU = GetWorld()->SpawnActorDeferred<ASZClipSpawner>(ASZClipSpawner::StaticClass(), SpawnTransform2, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		//ClipSpawn_AKU->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("AmmoSlot3"));
	}
	else // �Ϲ� ��Ȳ�̶��
	{
		//const FTransform SpawnTransform = FTransform(FRotator(0.0f, -20.0f, 0.0f), FVector(-50.0f, 40.0f, 70.0f), FVector(1.0f, 1.0f, 1.0f)); // = FTransform::Identity;
		//SpringArm->SetRelativeTransform(SpawnTransform);

		SpringArm->TargetArmLength = 160;
		SpringArm->SocketOffset = FVector(0, 40, 80);
		SpringArm->bUsePawnControlRotation = true;

		if (Role == ROLE_Authority)
		{
			if (WeaponType != nullptr && ClipType != nullptr)
			{
				CurrentWeapon = GetWorld()->SpawnActor<ASZWeaponBase>(WeaponType, FVector::ZeroVector, FRotator::ZeroRotator);
				CurrentWeapon->SetOwningPawn(this);
				FName WeaponSocketName = TEXT("WeaponHand_r");
				CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocketName);

				//int32 AvailableSlot = WeaponInventory.Find(nullptr);

				//if (AvailableSlot != INDEX_NONE)
				//{
					//Add the item to the first valid slot we found
					WeaponInventory.Add(CurrentWeapon);
				//}

				ASZClipBase* NewClip = GetWorld()->SpawnActor<ASZClipBase>(ClipType, FVector::ZeroVector, FRotator::ZeroRotator);
				if (NewClip != nullptr)
				{
					CurrentWeapon->SetOwningClip(NewClip);
					NewClip->ClipConfig.bInfiniteClip = true;
					CurrentWeapon->StartReload();
				}
			}
		}

		bEquipWeapon = true;
	}
}

// Called every frame
void ASZCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bUsingMotionControllers)
	{
		Raycasting();
	}

	switch (eStatecharacter)
	{
	case EStateCharacterEnum::Idle:
		IdleState();
		break;
	case EStateCharacterEnum::Move:
		MoveState();
		break;
	case EStateCharacterEnum::Attack:
		AttackState();
		break;
	case EStateCharacterEnum::Death:
		DeathState();
		break;
	case EStateCharacterEnum::Hit:
		HitState();
		break;
	case EStateCharacterEnum::Interaction:
		InteractionState();
		break;
	case EStateCharacterEnum::Reload:
		ReloadState();
		break;
	case EStateCharacterEnum::MoveAttack:
		MoveAttackState();
		break;
	default:
		break;
	}
}

// Called to bind functionality to input
void ASZCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ASZCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ASZCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("CameraX"), this, &ASZCharacter::CameraX);
	if (!bUsingMotionControllers) // ���� VR��尡 ������� �ƴ϶��
	{
		PlayerInputComponent->BindAxis(TEXT("CameraY"), this, &ASZCharacter::CameraY);
	}
	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &ASZCharacter::DoFire);
	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Released, this, &ASZCharacter::UnFire);
	PlayerInputComponent->BindAction(TEXT("Reload"), IE_Pressed, this, &ASZCharacter::DoReload);
	PlayerInputComponent->BindAction(TEXT("Reload"), IE_Released, this, &ASZCharacter::UnReload);

	// �޼� ������ ���� �Լ�. ���Ŀ� �Ѽո� ���� �����ϴ���, ��� ���о��� �����ϰ� �۵��ϵ��� �������.
	PlayerInputComponent->BindAction(TEXT("GrabL"), IE_Pressed, this, &ASZCharacter::GrabActorL);
	PlayerInputComponent->BindAction(TEXT("GrabR"), IE_Pressed, this, &ASZCharacter::GrabActorR);
	PlayerInputComponent->BindAction(TEXT("GrabL"), IE_Released, this, &ASZCharacter::UnGrabActorL);
	PlayerInputComponent->BindAction(TEXT("GrabR"), IE_Released, this, &ASZCharacter::UnGrabActorR);

	PlayerInputComponent->BindAction(TEXT("SelectInven"), IE_Pressed, this, &ASZCharacter::WeaponItemSelect);
	PlayerInputComponent->BindAction(TEXT("SelectInven"), IE_Released, this, &ASZCharacter::UnWeaponItemSelect);
	PlayerInputComponent->BindAction(TEXT("PickUpTest"), IE_Pressed, this, &ASZCharacter::ItemPickUp);

	PlayerInputComponent->BindAction(TEXT("ResetCameraPos"), IE_Pressed, this, &ASZCharacter::ResetCameraPos);
}

bool ASZCharacter::IsAlive() const
{
	return Health > 0;
}

bool ASZCharacter::IsEnemyFor(AController * TestPC) const
{
	if (TestPC == Controller || TestPC == NULL)
	{
		return false;
	}
	ASZPlayerState* TestPlayerState = Cast<ASZPlayerState>(TestPC->PlayerState);
	ASZPlayerState* MyPlayerState = Cast<ASZPlayerState>(GetPlayerState());

	bool bIsEnemy = true;
	if (GetWorld()->GetGameState())
	{
		const ASZGameMode* DefGame = GetWorld()->GetGameState()->GetDefaultGameMode<ASZGameMode>();
		if (DefGame && MyPlayerState && TestPlayerState)
		{
			bIsEnemy = DefGame->CanDealDamage(TestPlayerState, MyPlayerState);
		}
	}

	return bIsEnemy;
}

int32 ASZCharacter::GetMaxHealth() const
{
	return GetClass()->GetDefaultObject<ASZCharacter>()->Health;
}

void ASZCharacter::EquipWeapon(ASZWeaponBase * WeaponActor)
{
	if (CurrentWeapon == nullptr) // ó���� ���Ⱑ �����ٸ�
	{
		WeaponActor->GetWeaponMesh()->SetSimulatePhysics(false);
		FAttachmentTransformRules AttachmentTransformRules(EAttachmentRule::KeepWorld, false);
		WeaponActor->GetRootComponent()->AttachToComponent(MC_Right->GrabShpere, AttachmentTransformRules);

		WeaponActor->SetActorRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		WeaponActor->SetActorRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
		CurrentWeapon = WeaponActor;
		CurrentWeapon->SetOwningPawn(this);
		FName WeaponSocketName = TEXT("WeaponHand_r");
		CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocketName);
		int32 AvailbleSlot = WeaponInventory.Find(nullptr);
		// ���ڸ��� �߰ߵǸ� �ش� �ε����� ������ �߰�.
		// ���ڸ��� ���ٸ� �ε����� INDEX_NONE�� ��ȯ��.
		if (AvailbleSlot != INDEX_NONE)
		{
			WeaponInventory[AvailbleSlot] = WeaponActor;
			//WeaponInventory.Add(WeaponActor);
		}
		//WeaponInventory.Add(WeaponActor);
	}
	else
	{
		auto Weapon = Cast<ASZWeaponBase>(WeaponActor);
		if (Weapon)
		{
			// ���ڸ��� ã�´�.
			int32 AvailbleSlot = WeaponInventory.Find(nullptr);
			// ���ڸ��� �߰ߵǸ� �ش� �ε����� ������ �߰�.
			// ���ڸ��� ���ٸ� �ε����� INDEX_NONE�� ��ȯ��.
			if (AvailbleSlot != INDEX_NONE)
			{
				WeaponInventory[AvailbleSlot] = WeaponActor;
				//WeaponInventory.Add(WeaponActor);
				WeaponActor->Destroy();
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Inventory Is Full"));
			}
		}
	}
}

void ASZCharacter::SetAnimState(EStateCharacterEnum newState)
{
	if (eStatecharacter == newState)
	{
		return;
	}

	if (Role == ROLE_Authority)
	{
		eStatecharacter = newState;
	}

	if(Role < ROLE_Authority)
	{
		ServerSetAnimState(newState);
	}
}

void ASZCharacter::ServerSetAnimState_Implementation(EStateCharacterEnum NewState)
{
	eStatecharacter = NewState;
	//UE_LOG(LogTemp, Error, TEXT(" %d"), (int)NewState);
}

bool ASZCharacter::ServerSetAnimState_Validate(EStateCharacterEnum NewState)
{
	return true;
}

void ASZCharacter::ServerItemDestroy_Implementation()
{
	LastSeenItem->Destroy();
}

bool ASZCharacter::ServerItemDestroy_Validate()
{
	return true;
}


float ASZCharacter::TakeDamage(float DamageAmount, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
	//�������� ���ʹ̳� ?
	if (Role == ROLE_Authority)
	{
		if (EventInstigator->GetClass() == ASZAIController::StaticClass())
		{
			ASZPlayerState* PS = Cast<ASZPlayerState>(GetPlayerState());

			Health -= DamageAmount;

			if (PS)
			{
				PS->SetCurrentHealthFromCharacter(Health);
			}
		}

		if (Health <= 0)
		{
			eStatecharacter = EStateCharacterEnum::Death;
		}
		else
		{
			SetAnimState(EStateCharacterEnum::Hit);
		}

		return 0.0f;
	}
	return 0.f;
	
}

void ASZCharacter::ReloadOverlap(AActor * AmmoActor)
{
	ASZClipBase* NewClip = Cast<ASZClipBase>(AmmoActor);
	if (NewClip != nullptr)
	{
		CurrentWeapon->SetOwningClip(NewClip);
		CurrentWeapon->StartReload();
	}
}

void ASZCharacter::OnBeginOverlapLeftHand(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	auto ClipSpawn = Cast <ASZClipSpawner>(OtherActor);
	if (ClipSpawn)
	{
		IsClipSpawn = true;
		ClipSpawnerTemp = ClipSpawn;
	}
	//auto CheckSpawnZone = Cast<UChildActorComponent>(OverlappedComp);
	//if (CheckSpawnZone)
	//{
	//	SelectedActor = CheckSpawnZone;
	//	IsClipSpawn = true;
	//}
}

void ASZCharacter::OnEndOverlapLeftHand(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	ClipSpawnerTemp = nullptr;
	IsClipSpawn = false;
}

void ASZCharacter::SetAmmoPlus(EAmmoTypeEnum AmmoType, int Count = 1)
{
	switch (AmmoType)
	{
	case EAmmoTypeEnum::M4:
		AmmoCount_M4 += Count;
		break;
	case EAmmoTypeEnum::SMG:
		AmmoCount_SMG += Count;
		break;
	case EAmmoTypeEnum::AKU:
		AmmoCount_AKU += Count;
		break;
	}
}

// �̵����� ó��.
void ASZCharacter::MoveForward(float flAxisValue)
{
	// ĳ���� �����̼ǿ��� �߰������� ȸ������ add ����
	//AddMovementInput(GetActorForwardVector(), flAxisValue);
	//MoveAnimSpeed = flAxisValue;
	//if (flAxisValue != 0)
	//{
	//	SetAnimState(EAnimCharacterEnum::Move);
	//}

	if (eStatecharacter == EStateCharacterEnum::Death) return;

	if (flAxisValue != 0)
	{
		AddMovementInput(UKismetMathLibrary::GetForwardVector(FRotator(0, GetControlRotation().Yaw, 0)), flAxisValue);
		SetAnimState(EStateCharacterEnum::Move);
	}


}

void ASZCharacter::MoveRight(float flAxisValue)
{

	if (eStatecharacter == EStateCharacterEnum::Death) return;
	if (flAxisValue != 0)
	{
		AddMovementInput(UKismetMathLibrary::GetRightVector(FRotator(0, GetControlRotation().Yaw, 0)), flAxisValue);
		SetAnimState(EStateCharacterEnum::Move);
	}
	// ĳ���� �����̼ǿ��� �߰������� ȸ������ add ����
	//AddMovementInput(GetActorRightVector(), flAxisValue);
	//if (flAxisValue != 0)
	//{
	//	SetAnimState(EAnimCharacterEnum::Move);
	//}
}

void ASZCharacter::CameraX(float flAxisValue)
{
	if (eStatecharacter == EStateCharacterEnum::Death) return;

	AddControllerYawInput(flAxisValue * 2);
	//VR_Origin->AddWorldRotation;

	if (bUsingMotionControllers)
	{
		FRotator LRotator = MC_Left->GetActorRotation();

		LRotator.Yaw = LRotator.Yaw - flAxisValue * 2;

		MC_Left->SetActorRotation(LRotator);

		FRotator RRotator = MC_Right->GetActorRotation();

		RRotator.Yaw = RRotator.Yaw - flAxisValue * 2;

		MC_Right->SetActorRotation(RRotator);
	}
}

void ASZCharacter::DoFire()
{
	if (eStatecharacter == EStateCharacterEnum::Death) return;
//	if (bBtnGrap || bBtnChangeItem) return;

//	bBtnFire = true;

	// ������ �̵��� ���� ����� �ȵǾ��ֱ� ������ ���� �߰��Ǿ���.
	if (CurrentWeapon != nullptr)
	{
		CurrentWeapon->StartFire();
		//if (GetCharacterMovement()->Velocity.Size() <= 0)
		//{
			SetAnimState(EStateCharacterEnum::Attack);
//		}
		//else
		//{
		//	SetAnimState(EAnimCharacterEnum::MoveAttack);
		//}
			if (bIsFire == false)
			{
				if (Role == ROLE_Authority)
				{
					bIsFire = true;
				}

				if (Role < ROLE_Authority)
				{
					bIsFire = true;
					ServerIsFire(true);
				}
			}

	}

	//if (Role < ROLE_Authority)
	//{

	//}
	//UWorld* const World = GetWorld();
	//if (World != NULL)
	//{

	//}

}

void ASZCharacter::UnFire()
{
//	bBtnFire = false;
	if (eStatecharacter == EStateCharacterEnum::Death) return;
//	if (bBtnGrap || bBtnChangeItem) return;



	// ������ �̵��� ���� ����� �ȵǾ��ֱ� ������ ���� �߰��Ǿ���.
	if (GetCharacterMovement()->Velocity.Size() <= 0) // �̵��ӵ��� 0�� �Ǹ� ���̻� �������� ����.
	{
		SetAnimState(EStateCharacterEnum::Idle);
	}
	else
	{
		SetAnimState(EStateCharacterEnum::Move);
	}
	if (CurrentWeapon != nullptr)
	{
		CurrentWeapon->StopFire();
		if (bIsFire == true)
		{
			if (Role == ROLE_Authority)
			{
				bIsFire = false;
			}

			if (Role < ROLE_Authority)
			{
				bIsFire = false;
				ServerIsFire(false);
			}
		}
	}
}

void ASZCharacter::DoReload()
{
	if (eStatecharacter == EStateCharacterEnum::Death) return;
	// 1ȸ �Է¿� ���� ó���� �ʿ���. ���� ���濹��
	SetAnimState(EStateCharacterEnum::Reload);
	if (CurrentWeapon != nullptr)
	{
		int32 AvailbleSlot = AmmoInventory.Find(nullptr);
		// ���ڸ��� �߰ߵǸ� �ش� �ε����� ������ �߰�.
		// ���ڸ��� ���ٸ� �ε����� INDEX_NONE�� ��ȯ��.
		if (AvailbleSlot == INDEX_NONE)
		{
			UE_LOG(LogTemp, Error, TEXT("Inventory Is Empty"));
			return;
		}
		else
		{
			// �ϴ� ������ ������ ���߿� ����� ���������� ��ȯ�ϴ� ��ƾ�� ¥�ý��ϴ�.
			// ���߿� ��ȫ������ ��ȿ�� ������ �ִ� ���� ��ȯ���ִ� ��ƾ ������ּ�.
			SelectItemNum++;
			if (SelectItemNum >= AmmoInventory.Num()) // ������ �������� ������
			{
				SelectItemNum = 0; // �ε����� ó������ �ٲ���.
			}
			if (AmmoInventory[SelectItemNum] != nullptr)
			{
				ASZClipBase* NewClip = GetWorld()->SpawnActor<ASZClipBase>(AmmoInventory[SelectItemNum]->GetClass(), FVector::ZeroVector, FRotator::ZeroRotator);
				if (NewClip != nullptr)
				{
					CurrentWeapon->SetOwningClip(NewClip);
					CurrentWeapon->StartReload();
				}
			}
		}



	}
}

void ASZCharacter::UnReload()
{
	if (eStatecharacter == EStateCharacterEnum::Death) return;
	SetAnimState(EStateCharacterEnum::Idle);
}

void ASZCharacter::CameraY(float flAxisValue)
{
	AddControllerPitchInput(flAxisValue);
}

void ASZCharacter::IdleState()
{
	// �����¿� ���� ó��.
}

void ASZCharacter::MoveState()
{
	/*float MoveSpeed;
	MoveSpeed = GetCharacterMovement()->Velocity.Size();*/
	if (GetCharacterMovement()->Velocity.Size() <= 0) // �̵��ӵ��� 0�� �Ǹ� ���̻� �������� ����.
	{
		SetAnimState(EStateCharacterEnum::Idle);
	}
	
	if (bIsFire)
	{
		SetAnimState(EStateCharacterEnum::Attack);
	}
}

void ASZCharacter::AttackState()
{
	// ���ݿ� ���� ó��. ������Ÿ�� �����ؼ� ������.
	//if (bIsFire == false)
	//{
	//	if (Role == ROLE_Authority)
	//	{
	//		bIsFire = true;
	//	}

	//	if (Role < ROLE_Authority)
	//	{
	//		bIsFire = true;
	//		ServerIsFire(true);
	//	}
	//}
}

void ASZCharacter::DeathState()
{
	// ��� ó��.ü������ �߰� �ʿ�
}

void ASZCharacter::HitState()
{
	// �ǰݿ� ���� ó��.
	// UI �߰��� �ʿ���
	UE_LOG(LogTemp, Error, TEXT("Hit! %f"), Health);
	SetAnimState(EStateCharacterEnum::Idle);
}

void ASZCharacter::InteractionState()
{
	// ��ȣ�ۿ뿡 ���� ó��.
}

void ASZCharacter::ReloadState()
{
	// �������� ���� ó��.
}

void ASZCharacter::MoveAttackState()
{
	//UE_LOG(LogTemp, Error, TEXT("TEst"));
}

float ASZCharacter::PlayAnimMontage(UAnimMontage * AnimMontage, float InPlayRate, FName StartSectionName)
{
	return 0.0f;
}

void ASZCharacter::StopAnimMontage(UAnimMontage * AnimMontage)
{
}

void ASZCharacter::Raycasting()
{
	// ���ۺ���, ������
	FVector StartLocation = Camera->GetComponentLocation();
	FVector EndLocation = StartLocation + (Camera->GetForwardVector() * RaycastRange);
	// ����ĳ��Ʈ �� ���
	FHitResult RaycastHit;

	FCollisionQueryParams CQP;
	CQP.AddIgnoredActor(this); // �ݶ��̴� üŷ ��� �� �ݶ��̴��� ��-��

	ECollisionChannel ECC = ECollisionChannel::ECC_PhysicsBody;

	// Raycast �̱� ���̸� ��.
	GetWorld()->LineTraceSingleByChannel(RaycastHit, StartLocation, EndLocation, ECC, CQP);

	//DrawDebugLine(
	//	GetWorld(),
	//	StartLocation,
	//	EndLocation,
	//	FColor(255, 0, 0),
	//	false,
	//	0.f,
	//	0.f,
	//	10.f
	//);
	AActor* Pickup = Cast<AActor>(RaycastHit.GetActor());

	if (LastSeenItem && LastSeenItem != Pickup)
	{
		//TestSeenItem->SetGlowEffect(false);
	}

	if (Pickup)
	{
		LastSeenItem = Pickup;
		//Pickup->SetGlowEffect(true);
	}
	else
	{
		LastSeenItem = nullptr;
	}
}

void ASZCharacter::ResetCameraPos() // ī�޶� ��ġ ���� ��ư ���� YŰ
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

FRotator ASZCharacter::GetAimOffset() const
{
	return ActorToWorld().InverseTransformVectorNoScale(GetBaseAimRotation().Vector()).Rotation();
}

void ASZCharacter::ServerIsFire_Implementation(bool IsFire)
{
	bIsFire = IsFire;
}

bool ASZCharacter::ServerIsFire_Validate(bool IsFire)
{
	return true;
}

void ASZCharacter::WeaponItemSelect()
{
	if (eStatecharacter == EStateCharacterEnum::Death) return;

	//if (bBtnFire || bBtnGrap)
	//{
	//	return;
	//}
	//bBtnChangeItem = true;

	int32 AvailbleSlot = WeaponInventory.Find(nullptr);
	// ���ڸ��� �߰ߵǸ� �ش� �ε����� ������ �߰�.
	// ���ڸ��� ���ٸ� �ε����� INDEX_NONE�� ��ȯ��.
	//if (WeaponInventory.Num() <= 0)
	//{
	//	UE_LOG(LogTemp, Error, TEXT("Inventory Is Empty"));
	//	return;
	//}
	//else
	{
		// �ϴ� ������ ������ ���߿� ����� ���������� ��ȯ�ϴ� ��ƾ�� ¥�ý��ϴ�.
		// ���߿� ��ȫ������ ��ȿ�� ������ �ִ� ���� ��ȯ���ִ� ��ƾ ������ּ�.
		SelectItemNum++;
		if (SelectItemNum > WeaponInventory.Num() - 1) // ������ �������� ������
		{
			SelectItemNum = 0; // �ε����� ó������ �ٲ���.
		}
		if (WeaponInventory[SelectItemNum] != nullptr)
		{
			//UE_LOG(LogTemp, Error, TEXT("WeaponInventory %s "), WeaponInventory[SelectItemNum]->GetName());
			ASZWeaponBase* NewWeapon = GetWorld()->SpawnActor<ASZWeaponBase>(WeaponInventory[SelectItemNum]->GetClass(), FVector::ZeroVector, FRotator::ZeroRotator);
			if (NewWeapon != nullptr)
			{
				if (CurrentWeapon != nullptr)
				{
					if (CurrentWeapon->CurrentClip)
					{
						CurrentWeapon->CurrentClip->Destroy();
					}
					CurrentWeapon->Destroy();
				}
				if (bUsingMotionControllers)
				{
					NewWeapon->GetWeaponMesh()->SetSimulatePhysics(false);
					FAttachmentTransformRules AttachmentTransformRules(EAttachmentRule::KeepWorld, false);
					NewWeapon->GetRootComponent()->AttachToComponent(MC_Right->GrabShpere, AttachmentTransformRules);
					NewWeapon->SetActorRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
					NewWeapon->SetActorRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
				}
				else
				{
					//NewWeapon = GetWorld()->SpawnActor<ASZWeaponBase>(WeaponType, FVector::ZeroVector, FRotator::ZeroRotator);
					NewWeapon->SetOwningPawn(this);

				}
				CurrentWeapon = NewWeapon;
				CurrentWeapon->SetOwningPawn(this);
				FName WeaponSocketName = TEXT("WeaponHand_r");
				NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocketName);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("WeaponInventory[Select  null "));
			}
		}

	}
}

void ASZCharacter::UnWeaponItemSelect()
{
	//bBtnChangeItem = false;
	//if (bBtnFire || bBtnGrap)
	//{
	//	return;
	//}

}

void ASZCharacter::ItemPickUp()
{
	if (eStatecharacter == EStateCharacterEnum::Death) return;
	if (!bUsingMotionControllers)
	{
		if (LastSeenItem)
		{
			if (LastSeenItem->ActorHasTag(TEXT("Weapon")))
			{
				//Find the first available slot
				int32 AvailableSlot = WeaponInventory.Find(nullptr);

				if (AvailableSlot != INDEX_NONE)
				{
					//Add the item to the first valid slot we found
					WeaponInventory[AvailableSlot] = LastSeenItem;
					//Destroy the item from the game
					if (Role == ROLE_Authority)
					{
						LastSeenItem->Destroy();
					}

					if (Role < ROLE_Authority)
					{
						ServerItemDestroy();
					}
				}
				else GLog->Log("You can't carry any more Weapon!");
			}
			else if (LastSeenItem->ActorHasTag(TEXT("Ammo")))
			{
				//Find the first available slot
				int32 AvailableSlot = AmmoInventory.Find(nullptr);

				if (AvailableSlot != INDEX_NONE)
				{
					//Add the item to the first valid slot we found
					AmmoInventory[AvailableSlot] = LastSeenItem;
					//Destroy the item from the game
					if (Role == ROLE_Authority)
					{
						LastSeenItem->Destroy();
					}

					if (Role < ROLE_Authority)
					{
						ServerItemDestroy();
					}
				}
				else GLog->Log("You can't carry any more Ammo!");
			}
		}
	}
}


void ASZCharacter::GrabActorL()
{
	if (eStatecharacter == EStateCharacterEnum::Death) return;
	//if (bBtnFire || bBtnChangeItem) return;

	//bBtnGrap = true;
	if (IsClipSpawn)
	{
		switch (ClipSpawnerTemp->AmmoType)
		{
		case EAmmoTypeEnum::M4:
			if (AmmoCount_M4 > 0)
			{
				AmmoCount_M4--;
				if (AmmoCount_M4 <= 0)
				{
					AmmoCount_M4 = 0;
				}
				ClipSpawnerTemp->ClipSpawn(MC_Left->GrabShpere);
			}
			break;
		case EAmmoTypeEnum::SMG:
			if (AmmoCount_SMG > 0)
			{
				AmmoCount_SMG--;
				if (AmmoCount_SMG <= 0)
				{
					AmmoCount_SMG = 0;
				}
				ClipSpawnerTemp->ClipSpawn(MC_Left->GrabShpere);
			}
			break;
		case EAmmoTypeEnum::AKU:
			if (AmmoCount_AKU > 0)
			{
				AmmoCount_AKU--;
				if (AmmoCount_AKU <= 0)
				{
					AmmoCount_AKU = 0;
				}
				ClipSpawnerTemp->ClipSpawn(MC_Left->GrabShpere);
			}
			break;
		}
		//ASZClipSpawner* SZClipSpawner = Cast<ASZClipSpawner>((SelectedActor->GetChildActor()));

		//SZClipSpawner->ClipSpawn(MC_Left->GrabShpere);
	}
	else
	{
		MC_Left->GrabActor();
	}
}

void ASZCharacter::GrabActorR()
{
	//if (eStatecharacter == EStateCharacterEnum::Death) return;
	//if (bBtnFire || bBtnChangeItem) return;

	//bBtnGrap = true;
	//if (CurrentWeapon == nullptr)
	//{
	//	MC_Right->GrabActor();
	//}
}

void ASZCharacter::UnGrabActorL()
{
	MC_Left->UnGrabActor();
//	bBtnGrap = false;
	if (eStatecharacter == EStateCharacterEnum::Death) return;
//	if (bBtnFire || bBtnChangeItem) return;

}

void ASZCharacter::UnGrabActorR()
{
	//if (eStatecharacter == EStateCharacterEnum::Death) return;
	//if (CurrentWeapon == nullptr)
	//{
	//	MC_Right->UnGrabActor();
	//}
}

void ASZCharacter::Interaction()
{

}

void ASZCharacter::OnRep_CurrentWeapon(ASZWeaponBase * LastWeapon)
{
}

/////////////////////// replication
void ASZCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASZCharacter, Health);
	DOREPLIFETIME(ASZCharacter, CurrentWeapon);
	DOREPLIFETIME(ASZCharacter, eStatecharacter);
	DOREPLIFETIME(ASZCharacter, LastSeenItem);
}