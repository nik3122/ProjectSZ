// Fill out your copyright notice in the Description page of Project Settings.


#include "SZInventorySlotWidget.h"
#include "SZWeaponBase.h"

void USZInventorySlotWidget::SetEquippedItem()
{
}

void USZInventorySlotWidget::SetItemTexture(AActor * Item)
{
	//if (Item)
	//{
	//	// �� �����غ�����.
	//	ASZWeaponBase* Weapon = Cast<ASZWeaponBase>(Item);
	//	if (Weapon)
	//	{
	//		ItemTexture = Weapon->GetPickupTexture();
	//	}
	//}
	//else
	//{
	//	ItemTexture = nullptr;
	//}
}

void USZInventorySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
}
