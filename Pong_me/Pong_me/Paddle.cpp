#include "Paddle.h"


Paddle::Paddle()
{
	life = 1000.0f;
	chi = 0;
	superType = false;
	superCasePlay = true;
}



Paddle::Paddle(float _spriteStartX, float _spriteStartY, float _spriteEndX, float _spriteEndY, int _fSpriteFrameX, int _fSpriteFrameY, int _frameMax)
{
	life = 1000.0f;
	chi = 0;
	
	superType = false;
	superCasePlay = true;

	//for the frame position
	nFrame = 0;
	nBackGroundFrame = 0;
	bulletX = 0;
	spriteStartX = _spriteStartX;
	spriteStartY = _spriteStartY;

	fSpriteWidth = _spriteEndX - _spriteStartX;// 100~200
	fSpriteHeight = _spriteEndY - _spriteStartY; //50~70	

	fSpriteFrameX = _fSpriteFrameX;
	fSpriteFrameY = _fSpriteFrameY;

	oneFrameLength = fSpriteWidth / fSpriteFrameX; //around 40
	oneFrameHeight = fSpriteHeight / fSpriteFrameY; //around 50

	nFrameMax = _frameMax;
	
	smallChiNumber = 7;
	bigChiNumber = 2;
	smallChiCounter = 10;
	bigChiCounter = 15;

	changeConsum = 5;
	chiBigConsum = 3;
	chiSmallConsum = 1;
	AIMode = false;
	AIModeAction = AI_WaitNextOrder;
}



void Paddle::setSpriteCondition(Condition _condition)
{
	nFrame = 0;

	m_condition = _condition;

}

void Paddle::loadSpriteWH(float _spriteStartX, float _spriteStartY, float _spriteEndX, float _spriteEndY, int _frameX, int _frameY, int _frameMax)
{
	spriteStartX = _spriteStartX;
	spriteStartY = _spriteStartY;

	fSpriteWidth = _spriteEndX - _spriteStartX;// 100~200
	fSpriteHeight = _spriteEndY - _spriteStartY; //50~70	

	fSpriteFrameX = _frameX;
	fSpriteFrameY = _frameY;

	oneFrameLength = fSpriteWidth / _frameX; //around 40
	oneFrameHeight = fSpriteHeight / _frameY; //around 50

	nFrameMax = _frameMax;
	alreadyUpdate = false;
}


Paddle::~Paddle()
{
}

//setting the source format
void Paddle::updateSourceRect()
{
	srcRect.left = spriteStartX + (float)(nFrame % fSpriteFrameX) * oneFrameLength;//0~200
	srcRect.top = spriteStartY + (float)(nFrame /fSpriteFrameX)* oneFrameHeight; // 0~100
	srcRect.right =  (float)srcRect.left + (float)oneFrameLength;
	srcRect.bottom = (float)srcRect.top + oneFrameHeight;

	nFrame++;
}




void Paddle::updateDestinRect(float startX, float startY, float endX,float speed,float distFactor)
{
	desRect.left = startX + nBackGroundFrame*speed;
	desRect.top = startY;
	desRect.right = desRect.left + oneFrameLength*distFactor;
	desRect.bottom = desRect.top + oneFrameHeight*distFactor;

	if (desRect.left > endX)
	{
		nBackGroundFrame = 0;
	}
	nBackGroundFrame++;
}
//setting the source format
void Paddle::updateBackGroundRect(int stepSpeed)
{
	srcRect.left = srcRect.left + stepSpeed;// spriteStartX + (float)(nFrame % fSpriteFrameX) * oneFrameLength;//0~200
	srcRect.top = spriteStartY;// +(float)(nFrame / fSpriteFrameX)* oneFrameHeight; // 0~100
	srcRect.right = (float)srcRect.left + oneFrameLength;
	srcRect.bottom = (float)srcRect.top + oneFrameHeight;

	//at the end, go back to the origin
	if (srcRect.right > fSpriteWidth)
	{
		srcRect.left = 0;
		srcRect.right = (float)srcRect.left + oneFrameLength;
	}
	
}

void Paddle::detectFrame()
{
	if (nFrame == nFrameMax)
	{
		if (GetSuper())
		{
			setSpriteCondition(c_normalSuper);
		}
		else
		{
			setSpriteCondition(c_normal);
		}
		
		nFrame = 0;
	}
}
