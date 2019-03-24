#pragma once
#include "GameObject.h"

class Renderer;
class Player : public GameObject
{
public:
	Player();
	virtual ~Player();

	int ProcessInput();

	virtual bool Initialize(Renderer*);
	virtual void Update(float, float, float);
	virtual void Render(Renderer*);

	byte getID()	const { return m_ID; }
	void setID(byte id) { m_ID = id; }

	bool getCheck()	const { return m_Check; }
	void setCheck(bool check) { m_Check = check; }

private:
	float m_HpBar_width;
	float m_HpBar_height;
	float m_Speed;

	unsigned int m_TextureID;

	byte m_ID;
	bool m_Check;
};

