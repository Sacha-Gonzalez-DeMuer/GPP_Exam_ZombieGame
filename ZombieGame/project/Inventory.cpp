#include "stdafx.h"
#include "Inventory.h"
#include "IExamInterface.h"

Inventory::Inventory(IExamInterface* pInterface, UINT inventorySize)
	: m_pInventory{ }
	, m_pInterface{ pInterface }
	, m_InventorySize{ inventorySize }
{
	for (size_t i = 0; i < inventorySize; i++)
	{
		m_pInventory.push_back(ItemInfo());
		m_pInventory[i].Type = eItemType::INVALID;
	}
}

Inventory::~Inventory()
{
	m_pInterface = nullptr;
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


bool Inventory::HasItem(eItemType type) const
{
	for (UINT i = 0; i < m_InventorySize; ++i)
	{
		if (m_pInventory[i].Type == type)
		{
			return true;
		}
	}

	return false;
}

bool Inventory::HasWeapon() const
{
	for (UINT i = 0; i < m_InventorySize; ++i)
	{
		if (m_pInventory[i].Type == eItemType::PISTOL || m_pInventory[i].Type == eItemType::SHOTGUN)
		{
			return true;
		}
	}

	return false;
}


float Inventory::CalculateItemValue(UINT slot)
{
	if (!IsValid(slot))
		return FLT_MAX;

	ItemInfo item{};
	m_pInterface->Inventory_GetItem(slot, item);

	float value{ 0.0f };

	const float maxHP{ 10.0f };
	const float maxEnergy{ 10.0f };
	int medkitHP;
	int foodEnergy;

	// Calculate value per item type
	switch (item.Type)
	{
	case eItemType::MEDKIT:
		value += maxHP / m_pInterface->Agent_GetInfo().Health;
		medkitHP = m_pInterface->Medkit_GetHealth(item);
		if (medkitHP <= 0) value -= FLT_MAX;
		value += medkitHP * 2;
		break;

	case eItemType::FOOD:
		value += maxEnergy / m_pInterface->Agent_GetInfo().Energy;
		foodEnergy = m_pInterface->Food_GetEnergy(item);
		if (foodEnergy <= 0) value -= FLT_MAX;
		value += foodEnergy * 2;
		break;

	case eItemType::SHOTGUN:
		if (!HasWeapon())
			value += FLT_MAX;
		value += m_pInterface->Weapon_GetAmmo(item) * 2;
		break;

	case eItemType::PISTOL:
		if (!HasWeapon())
			value += FLT_MAX;
		value += m_pInterface->Weapon_GetAmmo(item) * 2;
		break;
	}

	// Lose value if duplicate
	ItemInfo inventoryItem{};
	bool isDuplicate{ false };
	for (UINT i = 0; i < m_InventorySize; ++i)
	{
		if (GetItem(i, inventoryItem) && inventoryItem.Type == item.Type)
		{
			value /= 2.0f; 
			isDuplicate = true;
			// Adjust value depending on ammo if item is a weapon
			if (item.Type == eItemType::PISTOL || item.Type == eItemType::SHOTGUN)
			{
				int inventoryWeaponAmmo{ m_pInterface->Weapon_GetAmmo(inventoryItem) };
				int itemAmmo{ m_pInterface->Weapon_GetAmmo(item) };

				// If weapon in inventory has more ammo, decrease value of item on ground
				if (inventoryWeaponAmmo > itemAmmo)
					value -= inventoryWeaponAmmo;
			}
		}
	}

	// Increase value if not duplicate
	if (!isDuplicate)
		value *= 2;

	return value;
}

UINT Inventory::GetLowestValueItem()
{
	float lowestValue{ FLT_MAX };
	UINT lowestValueIdx{ INVALID_INDEX };
	for (UINT i = 0; i < m_InventorySize; i++)
	{
		const float currentValue{ CalculateItemValue(i) };
		if (currentValue < lowestValue)
		{
			lowestValue = currentValue;
			lowestValueIdx = i;
		}
	}

	return lowestValueIdx;
}

std::vector<UINT> Inventory::GetLowestValueDuplicates()
{
	std::vector<UINT> lowestValueDuplicates{};

	// For every item type
	for (UINT type = 0; type < static_cast<int>(eItemType::FOOD); ++type)
	{
		// Go through entire inventory and save every slot which has current item type
		std::vector<UINT> typeSlots{};
		for (UINT slot = 0; slot < m_InventorySize; ++slot)
		{
			if (m_pInventory[slot].Type == static_cast<eItemType>(type))
				typeSlots.emplace_back(slot);
		}
		
		// If holding more than one of current item type
		if (typeSlots.size() > 1)
		{
			// Find which one has the lowest value
			float lowestValue{ FLT_MIN };
			UINT lowestValueSlot{ INVALID_INDEX };
			for (const auto& slot : typeSlots)
			{
				float value{ CalculateItemValue(slot) };
				if (value < lowestValue)
				{
					lowestValue = value;
					lowestValueSlot = slot;
				}
			}

			// Save it
			lowestValueDuplicates.emplace_back(lowestValueSlot);
		}
	}

	return lowestValueDuplicates;
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

bool Inventory::DropItem(eItemType type)
{
	for (UINT i = 0; i < m_InventorySize; ++i)
	{
		if (m_pInventory[i].Type == type)
		{
			return DropItem(i);
		}
	}

	return false;
}

bool Inventory::DropItem(UINT slot)
{
	if (slot == INVALID_INDEX)
		return false;
	
	if (m_pInterface->Inventory_RemoveItem(slot))
	{
		m_pInventory[slot].Type = eItemType::INVALID;
		--m_NrItems;
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
	if (m_pInventory[m_CurrentSlot].Type == eItemType::INVALID
		|| m_pInventory[m_CurrentSlot].Type == eItemType::EMPTY)
		return false;

	return m_pInterface->Inventory_UseItem(m_CurrentSlot);
}

bool Inventory::UseItem(UINT slot)
{
	if (m_pInventory[slot].Type == eItemType::INVALID
		|| m_pInventory[slot].Type == eItemType::EMPTY)
		return false;

	return m_pInterface->Inventory_UseItem(slot);
}

bool Inventory::UseItem(eItemType type)
{
	if (type == eItemType::INVALID)
		return false;

	ItemInfo item{};
	for (UINT i = 0; i < m_InventorySize; ++i)
	{
		if (m_pInventory[i].Type == type)
		{
			if (UseItem(i))
			{
				return true;
			}
		}
	}

	return false;
}

void Inventory::DeleteItem(UINT slot)
{
	m_pInventory[slot].Type = eItemType::INVALID;
}

bool Inventory::GetItem(UINT slot, ItemInfo& item)
{
	if (!IsValid(slot))
		return false;

	if (!m_pInterface->Inventory_GetItem(slot, item)) //item thought to be valid is not, remove
	{
		DeleteItem(slot);
		return false;
	}

	return true;
}

bool Inventory::IsItemEmpty(UINT slot)
{
	// Invalid items are non existent, not empty
	if (m_pInventory[slot].Type == eItemType::INVALID)
		return false;

	if (m_pInventory[slot].Type == eItemType::EMPTY)
		return true;

	int energy{  };
	switch (m_pInventory[slot].Type)
	{
	case eItemType::FOOD:
		energy = m_pInterface->Food_GetEnergy(m_pInventory[slot]);
		if (energy <= 0)
		{
			m_pInventory[slot].Type = eItemType::EMPTY;
			return true;
		}

		break;

	case eItemType::MEDKIT:
		energy = m_pInterface->Medkit_GetHealth(m_pInventory[slot]);
		if (energy <= 0)
		{
			m_pInventory[slot].Type = eItemType::EMPTY;
			return true;
		}
		break;

	case eItemType::PISTOL:
	case eItemType::SHOTGUN:
		if (m_pInterface->Weapon_GetAmmo(m_pInventory[slot]) <= 0)
		{
			m_pInventory[slot].Type = eItemType::EMPTY;
			return true;
		}
		break;
	}

	return false;
}

UINT Inventory::GetEmptyItemIdx()
{ 
	for (UINT i = 0; i < m_InventorySize; ++i)
	{
		if (m_pInventory[i].Type == eItemType::EMPTY)
			return i;
	}

	return INVALID_INDEX;
}

bool Inventory::HasEmptyItem()
{
	for (UINT i = 0; i < m_InventorySize; ++i)
	{
		if (IsItemEmpty(i))
			return true;
	}

	return false;
}

