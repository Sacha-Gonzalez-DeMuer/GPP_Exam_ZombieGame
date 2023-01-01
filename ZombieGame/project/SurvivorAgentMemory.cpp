#include "stdafx.h"
#include "SurvivorAgentMemory.h"
#include "IExamInterface.h"

SurvivorAgentMemory::SurvivorAgentMemory()
{
}

void SurvivorAgentMemory::Update(float deltaTime, IExamInterface* pInterface, std::vector<EntityInfo*> seenEntities, std::vector<HouseInfo*> seenHouses)
{
	if (m_CooldownTimer > m_CooldownTime)
	{
		for (const auto& entity : seenEntities)
		{
			AddToSeenItems(*entity);
		}

		for (const auto& house : seenHouses)
		{
			AddToSeenHouses(*house);
		}
	}
	else
	{
		m_CooldownTimer += deltaTime;
	}
}

void SurvivorAgentMemory::DebugRender(IExamInterface* pInterface) const
{
	for (const auto& item : m_SeenItems)
	{
		pInterface->Draw_Circle(item.Location, 5, { 1,.5f,.5f });
	}
}

void SurvivorAgentMemory::AddToSeenItems(const EntityInfo& item)
{
	bool alreadySeen{ false };
	for (auto seenItem : m_SeenItems)
	{
		if (seenItem.Location == item.Location)
			alreadySeen = true;
	}

	if (item.Type == eEntityType::ITEM && !alreadySeen)
		m_SeenItems.emplace_back(item);
}


void SurvivorAgentMemory::OnPickUpItem(const EntityInfo& item)
{
	// Iterate over all items in the m_SeenItems set
	for (auto it = m_SeenItems.begin(); it != m_SeenItems.end(); )
	{
		// Calculate the distance between the given item and the current seen item
		float distance = item.Location.DistanceSquared(it->Location);
		float threshold{ 5.f };
		// If the distance is within a certain threshold, delete the item from the set
		if (distance < threshold)
		{
			it = m_SeenItems.erase(it);
		}
		else
		{
			++it;
		}
	}

	m_CooldownTimer = 0;
}

void SurvivorAgentMemory::AddToSeenHouses(const HouseInfo& houseInfo) 
{
	bool alreadySeen{ false };
	for (auto seenHouse : m_SeenHouses)
	{
		if (seenHouse.Center == houseInfo.Center)
			alreadySeen = true;
	}

	if(!alreadySeen)
		m_SeenHouses.emplace_back(houseInfo);
}

void SurvivorAgentMemory::UpdateSeenItems(IExamInterface* pInterface)
{
	if (m_SeenItems.empty())
		return;
}

