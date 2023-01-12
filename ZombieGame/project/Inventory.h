#pragma once
#include "ExtendedStructs.h"
#include <memory>

class IExamInterface;

class Inventory final
{
public:
	Inventory(IExamInterface* pInterface, UINT inventorySize = 5);
	Inventory(const Inventory& other) = delete;
	Inventory(Inventory&& other) = delete;
	Inventory& operator=(const Inventory& other) = delete;
	Inventory& operator=(Inventory&& other) = delete;
	~Inventory();

	// Inventory management methods
	bool GrabItem(EntityInfo entity, ItemInfo& item);
	bool DropItem(eItemType type);
	bool DropItem(UINT slot);
	bool DropItem();
	void DeleteItem(UINT slot);
	bool GetItem(UINT slot, ItemInfo& item);

	// Item usage methods
	bool UseItem(); //uses current slot
	bool UseItem(UINT slot);
	bool UseItem(eItemType type);
	bool EquipItem(eItemType type);

	// Checks
	bool IsItemEmpty(UINT slot);
	UINT GetEmptyItemIdx();
	bool HasEmptyItem();
	bool HasItem(eItemType type) const;
	bool HasWeapon() const;
	bool IsFull() const { return m_NrItems >= m_InventorySize; };
	bool IsValid(UINT slot) const { return m_pInventory[slot].Type != eItemType::INVALID; };

	// Utils
	float CalculateItemValue(UINT slot);
	UINT GetLowestValueItem();
	std::vector<UINT> GetLowestValueDuplicates();

private:
	IExamInterface* m_pInterface;
	UINT m_InventorySize{ 4 };
	UINT m_NrItems{ 0 };
	UINT m_CurrentSlot{ 0 };
	std::vector<ItemInfo> m_pInventory;
};



