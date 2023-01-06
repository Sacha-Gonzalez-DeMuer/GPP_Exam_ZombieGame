#include "pch.h"
#include "MonteCarloTreeSearch.h"
#include <random>
#include "Board.h"

MonteCarloTreeSearch::MonteCarloTreeSearch()
	: m_RootNode{std::make_shared<MCTSNode>()}
{
}


int MonteCarloTreeSearch::FindNextMove(Board* pBoard)
{
	m_RootNode->State = *pBoard;


	for (int i = 0; i < m_NrIterations; i++)
	{
		std::shared_ptr<MCTSNode> promisingNode{ SelectNode() };
		if (promisingNode->State.InProgress())
			Expand(promisingNode);

		std::shared_ptr<MCTSNode> nodeToExplore{ promisingNode };
		if (promisingNode->Children.size() > 0)
			nodeToExplore = promisingNode->Children[utils::GetRandomInt(static_cast<int>(promisingNode->Children.size()))];

		Simulate(nodeToExplore);
		BackPropagate(nodeToExplore);
	}


	//Find node with most wins? || UCB?
	std::shared_ptr<MCTSNode> bestNode{ nullptr };
	for (const auto& child : m_RootNode->Children)
	{
		if (!bestNode)
		{
			bestNode = child;
			continue;
		}

		if (child->WinCount > bestNode->WinCount)
			bestNode = child;
	}

	if (!bestNode)
		return -1;

	return bestNode->State.GetLastMove();
}

std::shared_ptr<MCTSNode> MonteCarloTreeSearch::SelectNode()
{
	//Start
	std::shared_ptr<MCTSNode> current_node{ m_RootNode };
	std::shared_ptr<MCTSNode> highestUCDNode{};

	// Find leaf node with maximum win rate
	while (!current_node->IsLeaf())
	{

		// Find child node that maximises Upper Confidence Boundary
		for (const auto& child : current_node->Children)
		{
			// On first iteration set highest to first child node
			if (!highestUCDNode)
			{
				highestUCDNode = child;
				continue;
			}

			if (CalculateUCB(*child) > CalculateUCB(*highestUCDNode))
				highestUCDNode = child;
		}

		current_node = highestUCDNode;
	}

	return current_node;
}

void MonteCarloTreeSearch::Expand(std::shared_ptr<MCTSNode> fromNode)
{
	// For each available action from current state, add new state to the tree
	const auto& available_actions{ fromNode->State.GetAvailableActions() };
	std::vector<std::shared_ptr<MCTSNode>> new_children{};
	for (const auto& action : available_actions)
	{
		Board new_state{ fromNode->State };
		new_state.PlacePiece(action, myColor);

		// Create new child node
		std::shared_ptr<MCTSNode> new_node{ std::make_shared<MCTSNode>(new_state) };
		new_node->Parent = fromNode;
		new_children.push_back(new_node);
	}
	for (auto new_child : new_children)
	{
		fromNode->Children.emplace_back(new_child);
	}
	fromNode->Children.insert(fromNode->Children.end(), new_children.begin(), new_children.end());
}


/* After Expansion, the algorithm picks a child node arbitrarily,
and it simulates a randomized game from selected node until it reaches the resulting state of the game.*/
//Simulate game on node randomly, returns winner color if there is one, empty color if draw
Color4f MonteCarloTreeSearch::Simulate(std::shared_ptr<MCTSNode> node)
{
	Color4f current_player{ myColor };
	Board stateCopy{ node->State };

	// Loop forever
	while (true)
	{
		const auto availableActions{ stateCopy.GetAvailableActions() };

		stateCopy.PlacePiece(availableActions[utils::GetRandomInt(static_cast<int>(availableActions.size()))], myColor);

		if (stateCopy.CheckWin(myColor))
			return myColor;
			
		if (stateCopy.CheckDraw())
			return EMPTY;

		current_player = (current_player == myColor) ? current_player = PLAYER1 : current_player = myColor;
	}
}


/*
 Once the algorithm reaches the end of the game, 
 it evaluates the state to figure out which player has won. 
 It traverses upwards to the root and increments visit score for all visited nodes. 
 It also updates win score for each node if the player for that position has won the playout.
*/
void MonteCarloTreeSearch::BackPropagate(std::shared_ptr<MCTSNode> fromNode)
{
	std::shared_ptr<MCTSNode> currentNode{ fromNode };

	do
	{
		++currentNode->VisitCount;

		if (fromNode->State.CheckWin(myColor))
			++currentNode->WinCount;

		currentNode = currentNode->Parent;
	} while (currentNode != nullptr);
	
}

float MonteCarloTreeSearch::CalculateUCB(const MCTSNode& node) const
{
	if (node.VisitCount == 0)
		return 0;

	float UCB{ 0 };
	// Calculate Exploitation
	UCB += static_cast<float>(node.WinCount) / static_cast<float>(node.VisitCount);

	// Calculate Exploration
	UCB += 2 * sqrtf(static_cast<float>(m_RootNode->VisitCount) / static_cast<float>(node.VisitCount)); 

	return UCB;
}

/*
	if (current_node->IsLeaf())
	{
		// Has node been sampled before?
		if(current_node->VisitCount == 0)
		{
			//rollout
		}
		else
		{
			// For each available action from current state, add new state to the tree
			const auto& available_actions{ current_node->State.GetAvailableActions() };
			std::vector<MCTSNode*> new_children{};
			for (const auto& action : available_actions)
			{
				Board newState{ current_node->State };
				newState.PlacePiece(action, Color4f(1.0f, 1.0f, 0, 1.0f));

				new_children.push_back(new MCTSNode(newState));
			}
			current_node->Children.insert(current_node->Children.end(), new_children.begin(), new_children.end());


			// Current = first new child node
			current_node = new_children[0];

			// Rollout
		}


	}
	else {

		// Find child node that maximises Upper Confidence Boundary
		MCTSNode* highestUCDNode{};
		for (const auto& child : current_node->Children)
		{
			// On first iteration set highest to first child node
			if (!highestUCDNode)
			{
				highestUCDNode = child;
				continue;
			}

			if (CalculateUCB(*child) > CalculateUCB(*highestUCDNode))
				highestUCDNode = child;
		}

		current_node = highestUCDNode;
	}


*/
