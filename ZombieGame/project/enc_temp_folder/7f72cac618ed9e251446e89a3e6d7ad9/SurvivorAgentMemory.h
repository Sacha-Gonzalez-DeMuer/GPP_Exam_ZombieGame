#pragma once
#include <unordered_set>
#include "ExtendedStructs.h"

class IExamInterface;

class SurvivorAgentMemory final
{
public:
	SurvivorAgentMemory();

	void Update(float deltaTime, IExamInterface* pInterface, std::vector<EntityInfo*> seenEntities, std::vector<HouseInfo*> seenHouses);

	void DebugRender(IExamInterface* pInterface) const;

	bool HasSeenItems() const { return !m_SeenItems.empty(); };

	std::vector<EntityInfo> GetSeenItems() const { return m_SeenItems; };
	std::vector<HouseInfo> GetSeenHouses() const { return m_SeenHouses; };

	void AddToSeenItems(const EntityInfo& item);
	void OnPickUpItem(const EntityInfo& item);

	void AddToSeenHouses(const HouseInfo& houseInfo);

private:
	void UpdateSeenItems(IExamInterface* pInterface);
	float m_CooldownTime{ 0 };
	float m_CooldownTimer{ 0 };


	std::vector<EntityInfo> m_SeenItems{};
	std::vector<EntityInfo> m_SeenItemsBuffer{};


	std::vector<HouseInfo> m_SeenHouses{};
	std::vector<HouseInfo> m_VisitedHouses{};

};

