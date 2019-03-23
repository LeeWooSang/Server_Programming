#pragma once

class Renderer;
class GameObject
{
public:
	GameObject();
	virtual ~GameObject();

	virtual bool Initialize(Renderer*);
	virtual void Update(float, float, float);
	virtual void Render(Renderer*);

	float getX()	const { return m_X; }
	void setX(float x) { m_X = x; }
	
	float getY()	const { return m_Y; }
	void setY(float y) { m_Y = y; }

	Position getPosition()	const { return m_Position; }
	void setPosition(Position position) { m_Position = position; }

protected:
	float m_X;
	float m_Y;
	float m_Z;

	Position m_Position;
	float m_Size;

	float m_Color[4] = { 0 };	// R G B A

	float m_BuildLevel;

	float m_ElapsedTime;
};

