// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PackUpInterface.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "SZTypes.h"
#include "SZPickEnableActor.generated.h"

UCLASS()
class SZ_API ASZPickEnableActor : public AActor, public IPackUpInterface
{
	GENERATED_BODY()


	
public:	
	// Sets default values for this actor's properties
	ASZPickEnableActor();

	UPROPERTY(VisibleAnyWhere, Category = Actor)
	UStaticMeshComponent* ActorMesh; // ����Ī �޽�

	/*�Ⱦ� ���ɽ� ������ ��*/
	void SetGlowEffect(bool Status);

	/*Returns the Texture of our Pickup*/
	FORCEINLINE UTexture2D* GetPickupTexture(){ return PickupTexture; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Pickup_Implemetation(class USceneComponent* AttachTo);
	virtual void Drop_Implemetation();

	// �׳� ������ �������̽� ���� ȣ���ϴ� �Լ���.
	void Pickup_compulsion(class USceneComponent* AttachTo); // ����

	void Drop_compulsion(); // ������

	void Reload_compulsion(class USceneComponent* AttachTo); // ������(źâ�� �B������)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemType")
	EItemEnum eItemtype;

	UPROPERTY(EditAnywhere, Category = "PickupTexture")
		UTexture2D* PickupTexture;

	UPROPERTY(EditAnywhere, Category = "PickupTexture")
		FString ItemName;
};
