#pragma once
#include "ExtendedStructs.h"
#include <memory>

class IExamInterface;

class Inventory
{
public:
	Inventory(IExamInterface* pInterface, UINT inventorySize = 5);

	void InitializeInventory(IExamInterface* pInterface);
	void EquipWeapon();
	bool HasWeapon() const;
	bool GrabItem(EntityInfo entity);
	bool DropItem(UINT slot);
	bool UseItem();
	bool UseItem(UINT slot);
	bool IsFull() const { return m_NrItems >= m_InventorySize; };

private:
	IExamInterface* m_pInterface;
	UINT m_InventorySize{4};
	UINT m_NrItems{0};
	UINT m_CurrentSlot{0};
	std::shared_ptr<ItemInfo[]> m_pInventory;
};

