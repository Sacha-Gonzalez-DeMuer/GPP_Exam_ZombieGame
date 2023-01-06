#pragma once
#include <string>

// Forward Declarations
class Board;
class MonteCarloTreeSearch;

class Player {
public:
	Player(const Color4f& color, bool isHuman, const std::string& name);
	~Player();

	Color4f GetColor() const { return m_Color; };
	std::string GetName() { return m_Name; };
	bool IsHuman() const { return m_IsHuman; };


	// Gets the player's next move.
	bool GetMove(Board* pBoard, int& i);

	void ProcessMouseDownEvent(const SDL_MouseButtonEvent& e);

private:
	std::string m_Name;
	Color4f m_Color;
	bool m_IsHuman;

	bool m_WaitingForMove{ false };
	Vector2f m_ClickPos{INVALID_POSITION};


	MonteCarloTreeSearch* m_pMCTS;
};
