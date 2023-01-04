#pragma once
#include "ExtendedStructs.h"
#include <memory>

class IExamInterface;

class Inventory
{
public:
	Inventory(IExamInterface* pInterface, UINT inventorySize = 5);

	//Inventory management methods
	bool GrabItem(const ItemInfo& item);
	bool GrabItem(EntityInfo entity);
	bool GrabItem(EntityInfo entity, ItemInfo& item);
	bool DropItem(UINT slot);
	bool DropItem();
	void DeleteItem(UINT slot);
	bool GetItem(UINT slot, ItemInfo& item);
	bool DropEmptyItem();

	//Item usage methods
	bool UseItem(); //uses current slot
	bool UseItem(UINT slot);
	bool UseItem(eItemType type);
	bool EquipItem(eItemType type);

	//Checks
	bool IsItemEmpty(UINT slot);
	bool IsItemEmpty(ItemInfo item);
	bool HasEmptyItem();
	bool HasItem(eItemType type);
	bool IsFull() const { return m_NrItems >= m_InventorySize; };
	bool IsValid(const ItemInfo& item) const { return item.ItemHash != 0; };

	//Utils
	float CalculateItemValue(ItemInfo item);
	UINT GetLowestValueItem();

private:
	IExamInterface* m_pInterface;
	UINT m_InventorySize{4};
	UINT m_NrItems{0};
	UINT m_CurrentSlot{0};
	std::shared_ptr<ItemInfo[]> m_pInventory;
};

