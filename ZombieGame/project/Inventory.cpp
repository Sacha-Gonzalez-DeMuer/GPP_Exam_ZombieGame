#include "stdafx.h"
#include "Inventory.h"
#include "IExamInterface.h"

Inventory::Inventory(IExamInterface* pInterface, UINT inventorySize)
	: m_pInventory{ new ItemInfo[inventorySize]{} }
	, m_pInterface{pInterface}
	, m_InventorySize{inventorySize}
{
	InitializeInventory(pInterface);
}

void Inventory::InitializeInventory(IExamInterface* pInterface)
{
	ItemInfo item{};
	for (UINT i = 0; i < m_InventorySize; ++i)
	{
		if (pInterface->Inventory_GetItem(i, item))
			m_pInventory[i] = item;
	}
}

void Inventory::EquipWeapon()
{
	for (UINT i = 0; i < m_InventorySize; ++i)
	{
		if (m_pInventory[i].Type == eItemType::SHOTGUN)
		{
			m_CurrentSlot = i;
			return;
		} 
	}

	for (UINT i = 0; i < m_InventorySize; ++i)
	{
		if (m_pInventory[i].Type == eItemType::PISTOL)
		{
			m_CurrentSlot = i;
			return;
		}
	}
}

bool Inventory::HasWeapon() const
{
	ItemInfo item{};
	for (UINT i = 0; i < m_NrItems; ++i)
	{
		if (m_pInventory[i].Type == eItemType::PISTOL || m_pInventory[i].Type == eItemType::SHOTGUN && m_pInterface->Inventory_GetItem(i, item))
		{
			std::cout << "has weapon\n";
			return true;
		}
	}

	return false;
}

bool Inventory::GrabItem(EntityInfo entity)
{
	UINT freeSlot{m_CurrentSlot};
	for (UINT i = 0; i < m_InventorySize; ++i)
	{
		if (!m_pInterface->Inventory_GetItem(i, ItemInfo()))
		{
			freeSlot = i;
			break;
		}

	}

	ItemInfo item;
	if (!m_pInterface->Item_Grab(entity, item))
		return false;

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
	if (m_pInterface->Inventory_RemoveItem(m_CurrentSlot))
	{
		--m_NrItems;
		return true;
	}
	return false;
}

bool Inventory::UseItem()
{
	return m_pInterface->Inventory_UseItem(m_CurrentSlot);
}

bool Inventory::UseItem(UINT slot)
{
	return m_pInterface->Inventory_UseItem(slot);
}

