// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "SZWidgetActor.h"
#include "Engine/World.h"
#include "SZGameMode.generated.h"

class ASZAIController;
class ASZPlayerState;

//class FSpawnWidgetSingleton
//{
//private:
//	FSpawnWidgetSingleton() {};
//	~FSpawnWidgetSingleton() {};
//
//	static ASZWidgetActor* instance;
//	FActorSpawnParameters SpawnInfo;
//public:
//	static FSpawnWidgetSingleton* GetInstance()
//	{
//		if (instance == nullptr)
//		{
//			SpawnInfo.Instigator = nullptr;
//			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
//			SpawnInfo.OverrideLevel = nullptr;
//
//			UWorld* World = GetWorld();
//
//			if (instance == nullptr)
//			{
//				instance = World->SpawnActor<ASZWidgetActor>(SpawnInfo);
//				atexit(World->DestroyActor(instance));
//				return instance;
//			}
//		}
//		return instance;
//	}
//};
//
//FSpawnWidgetSingleton* FSpawnWidgetSingleton::instance = nullptr;

UCLASS(config=Game)
class ASZGameMode : public AGameMode
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameMode)
	TSubclassOf<APawn> EnemyPawnClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameMode")
		TSubclassOf<APawn> PlayerPawnClass;

	virtual void Logout(AController* exiting) override;
	virtual void Tick(float DeltaTime) override;

	//for spawning appropriate character
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	/** can players damage each other? */
	virtual bool CanDealDamage(ASZPlayerState* DamageInstigator, ASZPlayerState* DamagedPlayer) const;

	/* called before startmatch */
	virtual void HandleMatchIsWaitingToStart() override;

	/*starts new match */
	//virtual void HandleMatchHasStarted() override;

	/* Accept or reject a player attempting to join the server*/
	//virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	/* Starts Match Warmup*/
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/** prevents friendly fire */
	virtual float ModifyDamage(float Damage, AActor* DamagedActor, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const;

	class ASZWidgetActor* CreateWidgetActor();
	//class AMainMenuPawn* CreateMainMenuPawn();

	
	class ASZCharacter* CreatePlayerAIPawn();

	/*Post Login �Լ��� ����, ���� ���ӿ� ������ PlayerController ���� �� �ִ�.
	Post Login���� ��� Seamless Travel�� �ϰ� �ɶ� �� �̵��� �ѹ��� PC�� ȣ����� �� ������
	�� �Լ��� �ѹ� ���ӿ� ���� PC�� �˷��ְ� �ȴ�.
	�� �Ű������� ������ �����͸� �迭�� �����ϰ� �Ǹ� 
	���� ���ӿ� ������ �÷��̾��� ��Ʈ�ѷ��� �ٷ�� ����������.*/
	//virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

private:
	float TimeSinceStart;

	float TimeProgressStart;

	class ASZWidgetActor* AWA;
};
