#include "Defines.h"
#include "GameObject.h"

GameObject::GameObject()
	: m_X(0), m_Y(0), m_Z(0), m_Size(0), m_BuildLevel(0), m_ElapsedTime(0)
{
	m_Position.m_X = 0;
	m_Position.m_Y = 0;
}

GameObject::~GameObject()
{
}

bool GameObject::Initialize(Renderer* pRenderer)
{
	return true;
}

void GameObject::Update(float elapsedTime, float x, float y)
{
	float elapsedTimeInSecond = elapsedTime * 0.001f;
}

void GameObject::Render(Renderer* pRenderer)
{

}
