#pragma once
#include "framework/EliteAI/EliteNavigation/ENavigation.h"

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class AStar
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);
		//AStar(InfluenceMap<GridGraph<T_NodeType, T_ConnectionType>>* pGraph, Heuristic hFunction);


		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;


		//helper functions
		NodeRecord GetLowestFScoreConnection(std::vector<NodeRecord> fromList)
		{
			return *std::min_element(fromList.begin(), fromList.end());
		}
	};
	

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		//2.D wrong check english slides! -> "Connection to the connections endNode => This connection already points to a previously visited node
		std::vector<T_NodeType*> path;
		std::vector<NodeRecord> openList{}; //connections to be checked
		std::vector<NodeRecord> closedList{}; //connections already checked
		NodeRecord currentRecord{};

		const NodeRecord startRecord{ pStartNode, nullptr, GetHeuristicCost(pStartNode, pGoalNode) };
		openList.push_back(startRecord);

		//--------------STEP 1 COMPLETE---------------\\

		//2.
		while(!openList.empty())
		{
			//get connection with lowest F score
			currentRecord = GetLowestFScoreConnection(openList);

			//check if that connection leads to the end node. if so, end loop
			if (currentRecord.pConnection && currentRecord.pConnection->GetTo() == pGoalNode->GetIndex()) break;

			//else get all connections current node, loop over them
			const auto currentRecordConnections{ m_pGraph->GetNodeConnections(currentRecord.pNode) };

			//For each of the connections…
			for(auto& connectionFromCurrentRecord : currentRecordConnections)
			{
				//---------------STEP 2.1 COMPLETE-----------------\\
				// 
				//calculate g-cost so far
				T_NodeType* nextNode{ m_pGraph->GetNode(connectionFromCurrentRecord->GetTo()) };
				float gCost = currentRecord.costSoFar + 1 + GetHeuristicCost(currentRecord.pNode, pGoalNode) + connectionFromCurrentRecord->GetCost();

				//Check if any of those connections lead to a node already on the closed list
				bool isExistingConnectionCheaper{};
				for (auto& existingRecord : closedList)
				{
					if (connectionFromCurrentRecord->GetTo() == existingRecord.pNode->GetIndex()) // if connection leads to already checked connection
					{
						if (existingRecord.costSoFar < gCost) //Check if the already existing connection is cheaper 
						{
							//don't execute 2.F..
							isExistingConnectionCheaper = true;
							break;
						}
						else
						{
							closedList.erase(std::remove(closedList.begin(), closedList.end(), existingRecord));
						}
						
					} 
				}

				//check if any of those connections lead to a node already on the open list
				for(auto& existingRecord : openList)
				{
					//if a connection to the connections endNode already exist in the open list
					if(connectionFromCurrentRecord->GetTo() == existingRecord.pNode->GetIndex())
					{
						if (existingRecord.costSoFar < gCost) //Check if the already existing connection is cheaper 
						{
							//don't execute 2.F..
							isExistingConnectionCheaper = true;
							break; 
						}
						else
						{
							openList.erase(std::remove(openList.begin(), openList.end(), existingRecord));
						}
					}
				}

				if (isExistingConnectionCheaper) continue;

				//2.F
				//create new nodeRecord, add it to openList
				NodeRecord newNodeRecord{  };
				newNodeRecord.pNode = nextNode;
				newNodeRecord.costSoFar = gCost;
				newNodeRecord.estimatedTotalCost = gCost + GetHeuristicCost(nextNode, pGoalNode);
				newNodeRecord.pConnection = connectionFromCurrentRecord;
				openList.emplace_back(newNodeRecord);
			}

			//move current record to closed list
			closedList.push_back(currentRecord);
			openList.erase(std::remove(openList.begin(), openList.end(), currentRecord));

		}

		//3. Reconstruct path from last connection to start node
		while (currentRecord.pNode != pStartNode)
		{
			path.emplace_back(currentRecord.pNode);
			//look in the closedList for a record where pNode == currentRecord connection startNode
			for(auto& record : closedList)
			{
				if (record.pNode->GetIndex() == currentRecord.pConnection->GetFrom()) // found record
				{
					currentRecord = record; //set new currentRecord to found record
					break;
				} 
			}
		}

		path.emplace_back(pStartNode);
		std::reverse(path.begin(), path.end());
		return path;
	}


	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}