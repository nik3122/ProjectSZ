// Fill out your copyright notice in the Description page of Project Settings.


#include "SZPlayerState.h"
#include "UnrealNetwork.h"
#include "SZCharacter.h"

ASZPlayerState::ASZPlayerState()
{
	NumberOfCoinCollected = 0;
	MaxHealth = 3;
	CurrentHealth = MaxHealth;
	NumberOfDeaths = 0;

	bIsReady = false;
	bSpectator = false;
	SelectedCharacterIndex = 0;
}

/*
UPROPERTY�� Replicated��ũ�θ� �߰��ϴ� �͸����δ� �����ϰ� ���ø�����Ʈ�Ǿ��ٰ� �� �� ����.
������Ƽ ���ø����̼��� �ϼ��ϱ� ���ؼ���,
GetLifetimeREplicatedProps�Լ��� �����ؾ��Ѵ�.
�̸� ����, UnrealNetwork.h������ ��Ŭ��� �ؾ��Ѵ�.
*/
void ASZPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASZPlayerState, CurrentHealth);
	DOREPLIFETIME(ASZPlayerState, NumberOfCoinCollected);
	DOREPLIFETIME(ASZPlayerState, NumberOfDeaths);
	DOREPLIFETIME(ASZPlayerState, bIsReady);
	DOREPLIFETIME(ASZPlayerState, bSpectator);
	DOREPLIFETIME(ASZPlayerState, SelectedCharacterIndex);
	DOREPLIFETIME(ASZPlayerState, TeamNumber);
}

void ASZPlayerState::ToggleReady_Implementation()
{
	bIsReady = !bIsReady;
}

bool ASZPlayerState::ToggleReady_Validate()
{
	return true;
}

void ASZPlayerState::ToggleSpectator_Implementation()
{
	bSpectator = !bSpectator;
}

bool ASZPlayerState::ToggleSpectator_Validate()
{
	return true;
}

void ASZPlayerState::SelectCharacter_Implementation(int index)
{
	SelectedCharacterIndex = index;
}

bool ASZPlayerState::SelectCharacter_Validate(int index)
{
	return true;
}


int ASZPlayerState::GetCurrentHealth()
{
	return CurrentHealth;
}

void ASZPlayerState::UpdateTeamColors()
{
	AController* OwnerController = Cast<AController>(GetOwner());
	if (OwnerController != NULL)
	{
		ASZCharacter* MyCharacter = Cast<ASZCharacter>(OwnerController->GetCharacter());
		if (MyCharacter != NULL)
		{
			//MyCharacter->UpdateTeamColorsAllMIDs();
		}
	}
}



int32 ASZPlayerState::GetTeamNum() const
{
	return TeamNumber;
}

void ASZPlayerState::PlayerRespawnedAfterDeath_Implementation()
{
	if (Role == ROLE_Authority)
	{
		CurrentHealth = MaxHealth;

		//NumberOfCoinCollected = NumberOfCoinCollected / 2;
		NumberOfDeaths++;
	}
}

bool ASZPlayerState::PlayerRespawnedAfterDeath_Validate()
{
	return true;
}
void  ASZPlayerState::ReceiveDamage(int amount)
{
	CurrentHealth -= amount;

	CurrentHealth = FMath::Clamp(CurrentHealth, 0, MaxHealth);
}

void ASZPlayerState::OnRep_TeamColor()
{
	UpdateTeamColors();
}

void ASZPlayerState::CollectCoin()
{
	NumberOfCoinCollected += 1;
}

void ASZPlayerState::CollectHeart()
{
	CurrentHealth += 1;

	CurrentHealth = FMath::Clamp(CurrentHealth, 0, MaxHealth);
}