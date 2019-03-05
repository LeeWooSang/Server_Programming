#pragma once
#include "GameObject.h"

class Renderer;
class ChessBoard : public GameObject
{
public:
	ChessBoard();
	virtual ~ChessBoard();

	virtual bool Initialize(Renderer*, int, float, float);
	virtual void Update(float);
	virtual void Render(Renderer*);

private:
	// Â¦¼ö, È¦¼ö
	enum TYPE { EVEN, ODD };
};

