#include "stdafx.h"
#include "Inventory.h"
#include "IExamInterface.h"

Inventory::Inventory(IExamInterface* pInterface, UINT inventorySize)
	: m_pInventory{ new ItemInfo[inventorySize]{} }
	, m_pInterface{pInterface}
	, m_InventorySize{inventorySize}
{
}



bool Inventory::EquipItem(eItemType type)
{
	ItemInfo item;
	for (UINT i = 0; i < m_InventorySize; ++i)
	{
		if (GetItem(i, item) && item.Type == type)
		{
			m_CurrentSlot = i;
			return true;
		}
	}

	return false;
}


bool Inventory::HasItem(eItemType type)
{
	ItemInfo item{};
	for (UINT i = 0; i < m_InventorySize; ++i)
	{
		if (GetItem(i, item) && item.Type == type)
		{
			return true;
		}
	}

	return false;
}

float Inventory::CalculateItemValue(ItemInfo item)
{
	//If item doesn't exist return
	if (!IsValid(item))
		return 0.0f;

	float value{ 0.0f };

	const float maxHP{ 10.0f };
	const float maxEnergy{ 10.0f };

	//Calculate value per item type
	switch (item.Type) 
	{
		case eItemType::MEDKIT:
			value += maxHP / m_pInterface->Agent_GetInfo().Health;
			break;

		case eItemType::FOOD:
			value += maxEnergy / m_pInterface->Agent_GetInfo().Energy;
			break;

		case eItemType::SHOTGUN:
		case eItemType::PISTOL:
			if (!HasItem(eItemType::SHOTGUN) || !HasItem(eItemType::PISTOL))
				value += 10;
			value += m_pInterface->Weapon_GetAmmo(item);


		break;
	}



	//Lose value if duplicate
	ItemInfo inventoryItem{};
	for (UINT i = 0; i < m_InventorySize; ++i)
	{
		if (GetItem(i, inventoryItem) && inventoryItem.Type == item.Type)
		{
			--value;
		}
	}

	return 0.0f;
}

UINT Inventory::GetLowestValueItem()
{
	float lowestValue{ FLT_MAX };
	UINT lowestValueIdx{ 0 };
	for (UINT i = 0; i < m_InventorySize; i++)
	{
		const float currentValue{ CalculateItemValue(m_pInventory[i]) };
		if (currentValue < lowestValue)
		{
			lowestValue = currentValue;
			lowestValueIdx = i;
		}
	}

	return lowestValueIdx;
}

bool Inventory::GrabItem(EntityInfo entity)
{
	ItemInfo item;
	if (!m_pInterface->Item_Grab(entity, item))
		return false;

	UINT freeSlot{m_CurrentSlot};
	for (UINT i = 0; i < m_InventorySize; ++i)
	{
		if (!m_pInterface->Inventory_GetItem(i, ItemInfo()))
		{
			freeSlot = i;
			break;
		}

	}

	if (m_pInterface->Inventory_AddItem(freeSlot, item))
	{
		m_pInventory[freeSlot] = item;
		++m_NrItems;
			
		return true;
	}

	return false;
}

bool Inventory::GrabItem(EntityInfo entity, ItemInfo& item)
{
	if (!m_pInterface->Item_Grab(entity, item))
		return false;

	//Find free slot index
	UINT freeSlot{ m_CurrentSlot };
	for (UINT i = 0; i < m_InventorySize; ++i)
	{
		if (!m_pInterface->Inventory_GetItem(i, ItemInfo()))
		{
			freeSlot = i;
			break;
		}

	}

	//Add item to free slot
	if (m_pInterface->Inventory_AddItem(freeSlot, item))
	{
		m_pInventory[freeSlot] = item;
		++m_NrItems;

		return true;
	}

	return false;
}



bool Inventory::DropItem(UINT slot)
{
	if (m_pInterface->Inventory_RemoveItem(slot))
	{
		--m_NrItems;
		m_pInventory[slot].Type = eItemType::INVALID;
		return true;
	}
	return false;
}
bool Inventory::DropItem()
{
	return DropItem(m_CurrentSlot);
}

bool Inventory::UseItem()
{
	return m_pInterface->Inventory_UseItem(m_CurrentSlot);
}

bool Inventory::UseItem(UINT slot)
{
	return m_pInterface->Inventory_UseItem(slot);
}

bool Inventory::UseItem(eItemType type)
{
	if (type == eItemType::INVALID)
		return false;

	ItemInfo item{};
	for (UINT i = 0; i < m_InventorySize; ++i)
	{
		if (GetItem(i, item) && item.Type == type)
		{
			UseItem(i);
			return true;
		}
	}

	return false;
}

void Inventory::DeleteItem(UINT slot)
{
	m_pInventory[slot].ItemHash = 0;
	m_pInventory[slot].Type = eItemType::INVALID;
}

bool Inventory::GetItem(UINT slot, ItemInfo& item)
{
	if (!IsValid(m_pInventory[slot]))
		return false;

	if (!m_pInterface->Inventory_GetItem(slot, item)) //item thought to be valid is not, remove
	{
		DeleteItem(slot);
		return false;
	}

	return true;
}

void Inventory::DropEmptyItems()
{
	ItemInfo item{};
	for (UINT i = 0; i < m_InventorySize; ++i)
	{
		if (GetItem(i, item))
		{
			if (IsItemEmpty(item))
				DropItem(i);
		}
	}
}



bool Inventory::IsItemEmpty(ItemInfo item)
{
	switch (item.Type)
	{
		case eItemType::FOOD:
			if (m_pInterface->Food_GetEnergy(item) <= 0)
				return true;
			break;

		case eItemType::MEDKIT:
			if (m_pInterface->Medkit_GetHealth(item) <= 0)
				return true;
			break;
	}


	return false;
}

bool Inventory::IsItemEmpty(UINT slot)
{
	ItemInfo item{};
	if (!GetItem(slot, item))
		return false;

	return IsItemEmpty(item);
}

bool Inventory::HasEmptyItem()
{
	ItemInfo item{};
	for (UINT i = 0; i < m_InventorySize; ++i)
	{
		if (GetItem(i, item))
		{
			return IsItemEmpty(item);
		}
	}

	return false;
}

