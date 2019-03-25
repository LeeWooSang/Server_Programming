#pragma once
#include "GameObject.h"

class Renderer;
class Player : public GameObject
{
public:
	Player();
	virtual ~Player();

	int ProcessInput();
	int ProcessInput(int, int, int);

	virtual bool Initialize(Renderer*);
	virtual bool Initialize(Renderer*, int);

	virtual void Update(float, float, float);
	virtual void Render(Renderer*);

	byte getID()	const { return m_ID; }
	void setID(byte id) { m_ID = id; }


	bool getCheck()	const { return m_Check; }
	void setCheck(bool check) { m_Check = check; }

private:
	enum Type { Pawn, Rookie, Knight, Bishop, Queen, King };

	float m_HpBar_width;
	float m_HpBar_height;
	float m_Speed;

	unsigned int m_TextureID;

	byte m_ID;
	bool m_Check;
};

