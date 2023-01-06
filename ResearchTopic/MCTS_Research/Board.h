#pragma once
#include <array>

class Board 
{
public:
	// Constructor
	Board();
	Board(const Board& other);
	Board& operator=(const Board& other);

	Board(Board&& other) = delete;
	Board& operator=(const Board&& other) = delete;

	Board(float cellSize, const Window& window);

	void Render() const;

	// Places a piece of the given color at the given column. 
	// True if the move was successful, false if the column is full.
	bool PlacePiece(int column, const Color4f& color);

	// True if the given player has won the game, false otherwise.
	bool CheckWin(const Color4f& color) const;

	// True if the board is full and the game is a draw, false otherwise.
	bool CheckDraw() const;

	void Reset();

	// Getters
	Rectf GetBoardRect() const { return m_BoardRect; };
	float GetCellSize() const { return m_CellSize; };
	std::vector<int> GetAvailableActions();
	int GetLastMove() const { return m_LastMove; };
	int GetNrPieces() const { return m_NrPieces; };

	//MCTS Function => TODO: make base class for state objects
	bool InProgress() const;
private:
	int m_LastMove;
	

	// The number of pieces that have been placed on the board.
	int m_NrPieces{0};
	int m_NrRows{6};
	int m_NrColumns{7};
	Rectf m_BoardRect;

	// Corresponds to m_Board[rows][columns]
	std::array<std::array<Color4f, 7>, 6> m_Board{};
	float m_CellSize{50.0f};

	Color4f m_BoardColor{ 0.f, .5f, 1.0f, 1.0f };
};