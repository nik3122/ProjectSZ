// Fill out your copyright notice in the Description page of Project Settings.


#include "SZPlayerAIController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "AI/NavigationSystemBase.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

//ASZPlayerAIController::ASZPlayerAIController()
//{
//	RepeatInterval = 3.0f;
//}
//
//void ASZPlayerAIController::OnPossess(APawn* InPawn)
//{
//	Super::OnPossess(InPawn);
//	GetWorld()->GetTimerManager().SetTimer(RepeatTimerHandle, this, &ASZPlayerAIController::OnRepeatTimer, RepeatInterval, true);
//
//}
//
//void ASZPlayerAIController::OnUnPossess()
//{
//	Super::OnUnPossess();
//	GetWorld()->GetTimerManager().ClearTimer(RepeatTimerHandle);
//}
//
//void ASZPlayerAIController::OnRepeatTimer()
//{
//	auto CurrentPawn = GetPawn();
//	
//	
//}