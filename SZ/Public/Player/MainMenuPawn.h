// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SZMotionHandController.h"
#include "GameFramework/Pawn.h"
#include "MainMenuPawn.generated.h"

UCLASS()
class AMainMenuPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMainMenuPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);


	//UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Motion Hand Controller")
	//class ASZMotionHandController* Left_MotionHandController;

	//UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Motion Hand Controller")
	//class ASZMotionHandController*	Right_MotionHandController;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VR Origin")
		class USceneComponent* VR_Origin;

	/*UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	class USkeletalMeshComponent* Left_Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	class USkeletalMeshComponent* Right_Mesh;*/
};
