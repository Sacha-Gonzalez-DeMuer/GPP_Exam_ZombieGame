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
	bool UseItem();
	bool UseItem(UINT slot);

private:
	IExamInterface* m_pInterface;
	UINT m_InventorySize{5};
	UINT m_CurrentSlot{0};
	std::shared_ptr<ItemInfo[]> m_pInventory;
};

