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

private:
	float m_HpBar_width;
	float m_HpBar_height;
	float m_Speed;

	unsigned int m_TextureID;
};

