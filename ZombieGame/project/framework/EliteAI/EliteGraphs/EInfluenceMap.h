#pragma once
#include "EIGraph.h"
#include "EGraphNodeTypes.h"
#include "EGraphConnectionTypes.h"
#include <unordered_set>
namespace Elite
{
	template<class T_GraphType>
	class InfluenceMap final : public T_GraphType
	{
	public:
		InfluenceMap(bool isDirectional): T_GraphType(isDirectional) {}
		void InitializeBuffer() { m_InfluenceDoubleBuffer = std::vector<float>(m_Nodes.size()); }
		void PropagateInfluence(float deltaTime);
		void PropagateInfluence(float deltaTime, const Vector2& pos, float radius);
		void SetInfluenceAtPosition(Elite::Vector2 pos, float influence);
		void SetInfluenceAtPosition(int idx, float influence);
		void SetInfluenceAtPosition(const std::unordered_set<int>& indices, float influence);

		void SetScannedAtPosition(int idx, bool scanned);
		void SetScannedAtPosition(const std::unordered_set<int>& indices, bool scanned);

		void Render() const {}
		void SetNodeColorsBasedOnInfluence();

		float GetMomentum() const { return m_Momentum; }
		void SetMomentum(float momentum) { m_Momentum = momentum; }

		float GetDecay() const { return m_Decay; }
		void SetDecay(float decay) { m_Decay = decay; }

		float GetPropagationInterval() const { return m_PropagationInterval; }
		void SetPropagationInterval(float propagationInterval) { m_PropagationInterval = propagationInterval; }

	protected:
		virtual void OnGraphModified(bool nrOfNodesChanged, bool nrOfConnectionsChanged) override;

	private:
		Elite::Color m_NegativeColor{ 1.f, 0.2f, 0.f};
		Elite::Color m_NeutralColor{ 0.f, 0.f, 0.f };
		Elite::Color m_PositiveColor{ 0.f, 0.2f, 1.f};

		float m_MaxAbsInfluence = 100.f;
		float m_PropagationRadius = 100.f;
		float m_Momentum = 0.2f; // a higher momentum means a higher tendency to retain the current influence
		float m_Decay = 0.5f; // determines the decay in influence over distance

		float m_PropagationInterval = .05f; //in Seconds
		float m_TimeSinceLastPropagation = 0.0f;

		std::vector<float> m_InfluenceDoubleBuffer;
	};

	template <class T_GraphType>
	void InfluenceMap<T_GraphType>::PropagateInfluence(float deltaTime)
	{
		//make sure this function only executes once every interval
		m_TimeSinceLastPropagation += deltaTime;
		if (m_TimeSinceLastPropagation < m_PropagationInterval) return;
		m_TimeSinceLastPropagation = 0;

		m_InfluenceDoubleBuffer.clear();

		//go over all the nodes
		for (const auto& node : GetAllNodes()) 
		{
			//get each neighbor node, check their influence
			const auto connections{ GetNodeConnections(node->GetIndex()) };
			
			//check the influence of each neighboring node
			float highestInfluence{ 0 };
			for (const auto& connection : connections)
			{
				const float newInfluence{ GetNode(connection->GetTo())->GetInfluence() * std::expf(-connection->GetCost() * GetDecay())};

				if (abs(newInfluence) > abs(highestInfluence))
				{
					highestInfluence = newInfluence;
				}
			}

			m_InfluenceDoubleBuffer.emplace_back(Lerp(highestInfluence, node->GetInfluence(), GetMomentum()));
		} 


		//copy influences from buffer to the nodes
		for (size_t i = 0; i < m_InfluenceDoubleBuffer.size(); i++)
		{
			GetNode(i)->SetInfluence(m_InfluenceDoubleBuffer[i]);
		}
	}

	template <class T_GraphType>
	void InfluenceMap<T_GraphType>::PropagateInfluence(float deltaTime, const Vector2& pos, float radius)
	{
		//make sure this function only executes once every interval
		m_TimeSinceLastPropagation += deltaTime;
		if (m_TimeSinceLastPropagation < m_PropagationInterval) return;
		m_TimeSinceLastPropagation = 0;

		auto nodesInRange{ GetNodeIndicesInRadius(pos, radius) };

		//go over all the nodes
		for (int idx : nodesInRange)
		{
			const auto& node{ GetNode(idx) };

			//get each neighbor node, check their influence
			const auto connections{ GetNodeConnections(node->GetIndex()) };

			//check the influence of each neighboring node
			float highestInfluence{ 0 };
			for (const auto& connection : connections)
			{
				const float newInfluence{ GetNode(connection->GetTo())->GetInfluence() * std::expf(-connection->GetCost() * GetDecay()) };

				if (abs(newInfluence) > abs(highestInfluence))
				{
					highestInfluence = newInfluence;
				}
			}

			m_InfluenceDoubleBuffer[idx]=(Lerp(highestInfluence, node->GetInfluence(), GetMomentum()));
		}
	


		//copy influences from buffer to the nodes
		for (size_t i = 0; i < m_InfluenceDoubleBuffer.size(); i++)
		{
			GetNode(i)->SetInfluence(m_InfluenceDoubleBuffer[i]);
		}
	}

	template <class T_GraphType>
	inline void InfluenceMap<T_GraphType>::SetInfluenceAtPosition(Elite::Vector2 pos, float influence)
	{
		auto idx = GetNodeIdxAtWorldPos(pos);
		if (IsNodeValid(idx))
			GetNode(idx)->SetInfluence(influence);
	}

	template <class T_GraphType>
	inline void InfluenceMap<T_GraphType>::SetInfluenceAtPosition(int idx, float influence)
	{
		if (IsNodeValid(idx))
			GetNode(idx)->SetInfluence(influence);
	}


	template <class T_GraphType>
	inline void InfluenceMap<T_GraphType>::SetScannedAtPosition(int idx, bool scanned)
	{
		if (IsNodeValid(idx))
			GetNode(idx)->SetScanned(scanned);
	}

	template <class T_GraphType>
	inline void InfluenceMap<T_GraphType>::SetScannedAtPosition(const std::unordered_set<int>& indices, bool scanned)
	{
		for (const auto& idx : indices)
		{
			SetScannedAtPosition(idx, scanned);
		}
	}

	template <class T_GraphType>
	inline void InfluenceMap<T_GraphType>::SetInfluenceAtPosition(const std::unordered_set<int>& indices, float influence)
	{
		for (const auto& idx : indices)
		{
			SetInfluenceAtPosition(idx, influence);
		}
	}

	template<class T_GraphType>
	inline void InfluenceMap<T_GraphType>::SetNodeColorsBasedOnInfluence()
	{
		const float half = .5f;
		
		for (auto& pNode : m_Nodes)
		{
			Color nodeColor{};
			float influence = pNode->GetInfluence();
			float relativeInfluence = abs(influence) / m_MaxAbsInfluence;

			if (influence < 0)
			{
				nodeColor = Elite::Color{
				Lerp(m_NeutralColor.r, m_NegativeColor.r, relativeInfluence),
				Lerp(m_NeutralColor.g, m_NegativeColor.g, relativeInfluence),
				Lerp(m_NeutralColor.b, m_NegativeColor.b, relativeInfluence)
				};
			}
			else
			{
				nodeColor = Elite::Color{
				Lerp(m_NeutralColor.r, m_PositiveColor.r, relativeInfluence),
				Lerp(m_NeutralColor.g, m_PositiveColor.g, relativeInfluence),
				Lerp(m_NeutralColor.b, m_PositiveColor.b, relativeInfluence)
				};
			}

			pNode->SetColor(nodeColor);
		}
	}


	template<class T_GraphType>
	inline void InfluenceMap<T_GraphType>::OnGraphModified(bool nrOfNodesChanged, bool nrOfConnectionsChanged)
	{
		InitializeBuffer();
	}
}