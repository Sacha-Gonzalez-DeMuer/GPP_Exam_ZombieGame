#pragma once
#include <unordered_set>
#include "ExtendedStructs.h"

class IExamInterface;

class SurvivorAgentMemory final
{
public:
	SurvivorAgentMemory();

	void Update(float deltaTime, IExamInterface* pInterface, std::vector<EntityInfo*> seenEntities);

	void DebugRender(IExamInterface* pInterface) const;

	bool HasSeenItems() const { return !m_SeenItems.empty(); };

	std::unordered_set<Elite::Vector2, Elite::Vector2::Hash> GetSeenItems() const { return m_SeenItems; };

	void AddToSeenItems(EntityInfo* item);
	void OnPickUpItem(EntityInfo* item);

private:
	void UpdateSeenItems(IExamInterface* pInterface);
	float m_CooldownTime{ .1f };
	float m_CooldownTimer{ 0 };
	std::unordered_set<Elite::Vector2, Elite::Vector2::Hash> m_SeenItems{};
	std::unordered_set<Elite::Vector2, Elite::Vector2::Hash> m_SeenItemsBuffer{};


};

