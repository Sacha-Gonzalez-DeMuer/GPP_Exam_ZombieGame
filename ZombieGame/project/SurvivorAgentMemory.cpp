#include "stdafx.h"
#include "SurvivorAgentMemory.h"
#include "IExamInterface.h"

SurvivorAgentMemory::SurvivorAgentMemory()
{
}

void SurvivorAgentMemory::Update(float deltaTime, IExamInterface* pInterface, std::vector<EntityInfo*> seenEntities)
{
	if (m_CooldownTimer > m_CooldownTime)
	{
		for (auto entity : seenEntities)
		{
			AddToSeenItems(entity);
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
		pInterface->Draw_Circle(item, 5, { 1,.5f,.5f });
	}
}

void SurvivorAgentMemory::AddToSeenItems(EntityInfo* item)
{
	if (item->Type == eEntityType::ITEM)
		m_SeenItems.insert(item->Location);
}

void SurvivorAgentMemory::OnPickUpItem(EntityInfo* item)
{
	m_SeenItems.erase(item->Location);
	m_CooldownTimer = 0;
}

void SurvivorAgentMemory::UpdateSeenItems(IExamInterface* pInterface)
{
	if (m_SeenItems.empty())
		return;
}

