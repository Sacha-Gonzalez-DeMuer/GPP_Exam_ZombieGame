#include "pch.h"
#include "Game.h"
#include "utils.h"
#include "Board.h"
#include "Player.h"
#include <iostream>

Game::Game( const Window& window ) 
	: m_Window{ window }
	, m_pBoard{  new Board(50.0f, window) }
	, m_pPlayer1{std::make_unique<Player>(PLAYER1, true, "Giuseppe")}
	, m_pPlayer2{std::make_unique<Player>(PLAYER2, false, "Carlos")}
{
	Initialize( );
}

Game::~Game( )
{
	Cleanup( );
}

void Game::Initialize( )
{
	
}

void Game::Cleanup( )
{
	delete m_pBoard;
	m_pBoard = nullptr;
}

void Game::Update(float elapsedSec)
{
	std::shared_ptr<Player> current_player{ m_FirstPlayerTurn ? m_pPlayer1 : m_pPlayer2 };
	int move{};
	if (current_player->GetMove(m_pBoard, move))
	{
		if (!m_pBoard->PlacePiece(move, current_player->GetColor()))
		{
			std::cout << "Column full\n";
			return;
		}

		if (m_pBoard->CheckWin(current_player->GetColor()))
		{
			std::cout << current_player->GetName() << " wins!\n";
			m_GameFinished = true;
			return;
		}

		if (m_pBoard->CheckDraw())
		{
			std::cout << "Draw!\n";
			return;
		}

		m_FirstPlayerTurn = !m_FirstPlayerTurn;
	}
}

void Game::Draw( ) const
{
	ClearBackground( );

	m_pBoard->Render();
}

void Game::ProcessKeyDownEvent( const SDL_KeyboardEvent & e )
{
	//std::cout << "KEYDOWN event: " << e.keysym.sym << std::endl;
	switch (e.keysym.sym)
	{
	case SDLK_r:
		ResetGame();
		break;
	}
}

void Game::ProcessKeyUpEvent( const SDL_KeyboardEvent& e )
{
	//std::cout << "KEYUP event: " << e.keysym.sym << std::endl;
	//switch ( e.keysym.sym )
	//{
	//case SDLK_LEFT:
	//	//std::cout << "Left arrow key released\n";
	//	break;
	//case SDLK_RIGHT:
	//	//std::cout << "`Right arrow key released\n";
	//	break;
	//case SDLK_1:
	//case SDLK_KP_1:
	//	//std::cout << "Key 1 released\n";
	//	break;
	//}
}

void Game::ProcessMouseMotionEvent( const SDL_MouseMotionEvent& e )
{
	//std::cout << "MOUSEMOTION event: " << e.x << ", " << e.y << std::endl;
}

void Game::ProcessMouseDownEvent( const SDL_MouseButtonEvent& e )
{
	if(m_FirstPlayerTurn)
		m_pPlayer1->ProcessMouseDownEvent(e);
	else
		m_pPlayer2->ProcessMouseDownEvent(e);

	if (m_GameFinished)
		ResetGame();
}

void Game::ProcessMouseUpEvent( const SDL_MouseButtonEvent& e )
{
	//std::cout << "MOUSEBUTTONUP event: ";
	//switch ( e.button )
	//{
	//case SDL_BUTTON_LEFT:
	//	std::cout << " left button " << std::endl;
	//	break;
	//case SDL_BUTTON_RIGHT:
	//	std::cout << " right button " << std::endl;
	//	break;
	//case SDL_BUTTON_MIDDLE:
	//	std::cout << " middle button " << std::endl;
	//	break;
	//}
}

void Game::ClearBackground( ) const
{
	glClearColor( 0.0f, 0.0f, 0.3f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT );
}

void Game::ResetGame()
{
	m_pBoard->Reset();
	m_GameFinished = false;
}
