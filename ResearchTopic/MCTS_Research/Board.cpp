#include "pch.h"
#include "Board.h"
#include <iostream>

Board::Board()
    : m_CellSize{ 50.0f }
    , m_NrRows{ static_cast<int>(m_Board.size()) }
    , m_NrColumns{ static_cast<int>(m_Board[0].size()) }
    , m_BoardRect{  }
{
    m_BoardRect.width = m_NrColumns * m_CellSize;
    m_BoardRect.height = m_NrRows * m_CellSize;
}

Board::Board(const Board& other)
    : m_NrPieces{other.m_NrPieces}
    , m_NrRows{other.m_NrRows}
    , m_NrColumns{other.m_NrColumns}
    , m_BoardRect{ other.m_BoardRect }
    , m_BoardColor{ other.m_BoardColor }
    , m_LastMove{ other.m_LastMove }
    , m_Board{other.m_Board}
    , m_CellSize{other.m_CellSize}
{
}

Board& Board::operator=(const Board& other)
{
  m_NrPieces = other.m_NrPieces;
  m_NrRows = other.m_NrRows;
  m_NrColumns = other.m_NrColumns;
  m_BoardRect = other.m_BoardRect;
  m_BoardColor = other.m_BoardColor;
  m_LastMove = other.m_LastMove;
  m_Board = other.m_Board;
  m_CellSize = other.m_CellSize;
  return *this;
}

Board::Board(float cellSize, const Window& window)
    : m_CellSize{ cellSize }
    , m_NrRows{static_cast<int>(m_Board.size())}
    , m_NrColumns{static_cast<int>(m_Board[0].size())}
    , m_BoardRect{  }
{
    m_BoardRect.width = m_NrColumns * m_CellSize;
    m_BoardRect.height = m_NrRows * m_CellSize;
    m_BoardRect.left = window.width / 2.0f - m_BoardRect.width /2.0f;
    m_BoardRect.bottom = window.height / 2.0f - m_BoardRect.height / 2.0f;
}

void Board::Render() const
{
    utils::SetColor(m_BoardColor);
    utils::FillRect(GetBoardRect());

    float circle_size{ m_CellSize * .5f };
    // Fix the position of the ellipse for the first row.
    Ellipsef cell{ GetBoardRect().left + circle_size,
                   GetBoardRect().bottom + circle_size,
                   m_CellSize * .5f,
                   m_CellSize * .5f };
    for (int row = 0; row < m_NrRows; ++row)
    {
        for (int col = 0; col < m_NrColumns; ++col)
        {
            utils::SetColor(m_Board[row][col]);
            utils::FillEllipse(cell);
            cell.center.x += m_CellSize;
        }
        // Reset the x position of the ellipse for the next row.
        cell.center.x = GetBoardRect().left + circle_size;
        cell.center.y += m_CellSize;
    }
}

bool Board::PlacePiece(int column, const Color4f& color) 
{
    // Check if the column is full.
    if (m_Board[m_NrRows-1][column] != EMPTY) {
        return false;
    }

    // Find the lowest empty cell in the column.
    int row = 5;
    while (row >= 0 && m_Board[row][column] == EMPTY) {
        --row;
    }

    std::cout << "placing piece at: " << column << "\n";
    // Place the piece in the cell.
    m_Board[row + 1][column] = color;
    m_LastMove = column;
    ++m_NrPieces;
    return true;
}


bool Board::CheckWin(const Color4f& color) const
{
    // Check for horizontal wins.
    for (int row = 0; row < m_NrRows; ++row) {
        for (int col = 0; col < 4; ++col) {
            if (m_Board[row][col] == color &&
                m_Board[row][col + 1] == color &&
                m_Board[row][col + 2] == color &&
                m_Board[row][col + 3] == color) {
                return true;
            }
        }
    }

    // Check for vertical wins.
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < m_NrColumns; ++col) {
            if (m_Board[row][col] == color &&
                m_Board[row + 1][col] == color &&
                m_Board[row + 2][col] == color &&
                m_Board[row + 3][col] == color) {
                return true;
            }
        }
    }

    // Check for diagonal wins.
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 4; ++col) {
            if (m_Board[row][col] == color &&
                m_Board[row + 1][col + 1] == color &&
                m_Board[row + 2][col + 2] == color &&
                m_Board[row + 3][col + 3] == color) {
                return true;
            }
        }
    }
    for (int row = 0; row < 3; ++row) {
        for (int col = 3; col < m_NrColumns; ++col) {
            if (m_Board[row][col] == color &&
                m_Board[row + 1][col - 1] == color &&
                m_Board[row + 2][col - 2] == color &&
                m_Board[row + 3][col - 3] == color) {
                return true;
            }
        }
    }

    // No win was found.
    return false;
}

bool Board::CheckDraw() const
{
	return m_NrPieces == 42;
}

void Board::Reset()
{
    for (int row = 0; row < m_NrRows; ++row)
    {
        for (int col = 0; col < m_NrColumns; ++col)
        {
            m_Board[row][col] = EMPTY;
        }
    }
}

std::vector<int> Board::GetAvailableActions()
{
    std::vector<int> availableActions{};

    for (int col = 0; col < m_NrColumns; ++col)
    {
        // Check if the column is full.
        if (m_Board[m_NrRows - 1][col] == EMPTY)
            availableActions.push_back(col);

    }
   
    return availableActions;
}

bool Board::InProgress() const
{
    if(CheckWin(PLAYER1) || CheckWin(PLAYER2) || CheckDraw() )
        return false;
    return true;
}
