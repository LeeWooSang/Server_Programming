#include "stdafx.h"
#include "ChessBoard.h"
#include "Renderer.h"

ChessBoard::ChessBoard()
	: GameObject()
{
}

ChessBoard::~ChessBoard()
{
}

bool ChessBoard::Initialize(Renderer* pRenderer, int type, float x, float y)
{
	m_X = x;
	m_Y = y;
	m_Size = 100.f;

	if (type == EVEN)
	{
		int i = 0;
		m_Color[i++] = 0.f;
		m_Color[i++] = 0.3f;
		m_Color[i++] = 0.3f;
		m_Color[i++] = 1.f;
	}
	else if (type == ODD)
	{
		int i = 0;
		m_Color[i++] = 1.f;
		m_Color[i++] = 0.f;
		m_Color[i++] = 1.f;
		m_Color[i++] = 1.f;
	}

	m_BuildLevel = 0.2f;

	return true;
}

void ChessBoard::Update(float elapsedTime, float x, float y)
{
	float elapsedTimeInSecond = elapsedTime * 0.001f;
	m_ElapsedTime = elapsedTimeInSecond;
}

void ChessBoard::Render(Renderer* pRenderer)
{
	int i = 0;
	pRenderer->DrawSolidRect(m_X, m_Y, m_Z, m_Size, m_Color[i++], m_Color[i++], m_Color[i++], m_Color[i++], m_BuildLevel);	
	//glColor4f(m_Color[0], m_Color[1], m_Color[2], m_Color[3]);
	//glBegin(GL_LINE);
	//glVertex2f(0.f, 0.f);
	//glVertex2f(1.f, 1.f);
	//glEnd();
}

