#pragma once

class Renderer;
class GameObject
{
public:
	GameObject();
	virtual ~GameObject();

	virtual bool Initialize(Renderer*);
	virtual void Update(float);
	virtual void Render(Renderer*);

protected:
	float m_X;
	float m_Y;
	float m_Z;
	float m_Size;

	float m_Color[4] = { 0 };	// R G B A

	float m_BuildLevel;

	float m_ElapsedTime;
};

