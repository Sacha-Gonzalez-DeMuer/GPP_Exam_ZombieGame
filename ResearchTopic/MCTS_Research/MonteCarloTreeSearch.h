#pragma once
#include <array>
#include <memory>
#include "Board.h"
// Forward declarations




//TODO: make class
struct MCTSNode
{
	MCTSNode() {};

	MCTSNode(const Board& state)
		: State(state) {};

	MCTSNode(const MCTSNode& other)
		: WinCount(other.WinCount), VisitCount(other.VisitCount), Children(other.Children) {};

	MCTSNode& operator=(const MCTSNode& other)
	{
		VisitCount = other.VisitCount;
		WinCount = other.WinCount;
		State = other.State;
		Children = other.Children;
		return *this;
	}

	// State of the game in this node
	Board State;
	UINT VisitCount{ 0 };
	UINT WinCount{ 0 };
	std::shared_ptr<MCTSNode> Parent{nullptr};
	std::vector<std::shared_ptr<MCTSNode>> Children{};

	bool IsLeaf() const { return Children.empty(); }
	void AddChild() { Children.emplace_back(new MCTSNode()); };
};



class MonteCarloTreeSearch final
{
public:
	MonteCarloTreeSearch();

	int FindNextMove(Board* pBoard);


private:

	std::shared_ptr<MCTSNode> m_RootNode;

	std::shared_ptr<MCTSNode> SelectNode();
	void Expand(std::shared_ptr<MCTSNode> fromNode);
	Color4f Simulate(std::shared_ptr<MCTSNode> node);
	void BackPropagate(std::shared_ptr<MCTSNode> fromNode);


	float CalculateUCB(const MCTSNode& node) const;

	int m_NrIterations{ 100 };

	//tmp
	Color4f myColor{ PLAYER2 };
};



/*
//Sources
https://youtu.be/UXW2yZndl7U
https://youtu.be/xmImNoDc9Z4
https://medium.com/swlh/tic-tac-toe-at-the-monte-carlo-a5e0394c7bc2
https://pranav-agarwal-2109.medium.com/game-ai-learning-to-play-connect-4-using-monte-carlo-tree-search-f083d7da451e
https://towardsdatascience.com/monte-carlo-tree-search-an-introduction-503d8c04e168
https://www.baeldung.com/java-monte-carlo-tree-search
*/