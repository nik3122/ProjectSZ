// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SZPlayerController.h"
#include "UI/SZInventoryWidget.h"
#include "UI/SZHUD.h"

void ASZPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ItemWidgetClass && IsLocalController())
	{
		//BP Ŭ������ �ν��Ͻ� ����(UMGŬ���� �������� ����)
		ItemWidget = CreateWidget<USZInventoryWidget>(this, ItemWidgetClass);
		//ȭ�鿡 ���̱�
		if (ItemWidget->IsValidLowLevel())
		{
			ItemWidget->AddToViewport();
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("NotHave"));
	}
}

ASZPlayerController::ASZPlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void ASZPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	

}

void ASZPlayerController::HandleInventoryInput()
{

}

void ASZPlayerController::SetPlayer(UPlayer * InPlayer)
{
	Super::SetPlayer(InPlayer);
}

//void ASZPlayerController::Possess(APawn* aPawn)
//{
//	ASZGameState* SZGameState = Cast<ASZGameState>(GameState);
//
//	SZGameState->Pawns.PawnClass = aPawn;
//
//	//for (auto &Player : SZGameState->PlayerArray)
//	//{
//	//	Player->
//	//}
//
//}
void ASZPlayerController::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
}





void ASZPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ASZPlayerController::OnPossess(APawn * aPawn)
{
	Super::OnPossess(aPawn);
}

ASZHUD* ASZPlayerController::GetSZHUD() const
{
	return Cast<ASZHUD>(GetHUD());
}