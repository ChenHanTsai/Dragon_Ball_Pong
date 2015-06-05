#pragma once

#include <d2d1.h> // Include the Direct2D API.
#pragma comment(lib, "d2d1.lib") // Connect to the Direct2D Import Library.

enum Condition{c_normal,c_hit,c_recounter,c_smallAttack,c_bigAttack,c_hurt,
	c_normalSuper, c_hitSuper, c_recounterSuper, c_smallAttackSuper, c_bigAttackSuper, c_hurtSuper, c_lose, c_loseSuper
};

enum AIActionDecide{ AI_smallAtk, AI_bigAtk, AI_Super, AI_WaitNextOrder };
#include "BulletChi.h"
#include "PressDetect.h"
class Paddle
{
private:
	
	//one frame unit
	int nFrame;
	int nBackGroundFrame;
	float oneFrameHeight;
	float oneFrameLength;
	int nFrameMax;

	//total sprite
	float fSpriteHeight;
	float fSpriteWidth;

	//unit length
	int fSpriteFrameX;
	int fSpriteFrameY;

	//for drawing

	D2D1_RECT_F srcRect;

	
	int counter;
	int chi;
	float life;

	Condition m_condition;
	float spriteStartX;
	float spriteStartY;

	//bullet movement
	float bulletX;
	int smallChiNumber;
	int bigChiNumber;
	int smallChiCounter;
	int bigChiCounter;
	int chiBigConsum;
	int chiSmallConsum;
	bool superCasePlay;
	bool superType;
	int changeConsum;

	bool AIMode;
	AIActionDecide AIModeAction;
public:
	
	AIActionDecide GetAIModeAction()const
	{
		return AIModeAction;
	}

	void SetAIModeAction(AIActionDecide _AIModeAction)
	{
		AIModeAction = _AIModeAction;
	}

	bool GetAIMode()const
	{
		return AIMode;
	}

	void SetAIMode(bool _AIMode)
	{
		AIMode = _AIMode;
	}
	int GetFrameMax()const
	{
		return nFrameMax;
	}
	
	int GetChangeConsum()const
	{
		return changeConsum;
	}
	bool GetSuperCasePlay()const
	{
		return superCasePlay;
	}

	void SetSuperCasePlay(bool _play)
	{
		superCasePlay = _play;
	}
	Condition pre;
	Condition now;

	void SetSuper(bool _super)
	{
		superType =  _super;
	}

	bool GetSuper()const
	{
		return superType;
	}
	bool alreadyUpdate;

	PressDetect bigButton, smallButton,superButton;

	int GetChiBigConsum()const
	{
		return chiBigConsum;
	}
	int GetChiSmallConsum()const
	{
		return chiSmallConsum;
	}
	int getSmallChiNumber()const
	{
		return smallChiNumber;
	}
	int getBigChiNumber()const
	{
		return bigChiNumber;
	}
	int getSmallChiCounter()const
	{
		return smallChiCounter;
	}
	int getBigChiCounter()const
	{
		return bigChiCounter;
	}
	BulletChi smallAtk[7];
	ID2D1Bitmap* sprite;
//	ID2D1Bitmap* superCase;
//	ID2D1Bitmap* smallAtk;
	BulletChi bigAtk[2];


	Condition GetSpriteCondition()const
	{
		return m_condition;
	}
	void SetLife(float _life)
	{
		life = _life;
		if (life <= 0)
			life = 0;
	}
	float GetLife()const
	{
		return life;
	}
	void SetChi(int _chi)
	{
		chi = _chi;
		if (chi <= 0)
			chi = 0;
	}
	int GetChi()const
	{
		return chi;
	}
	void setSpriteCondition(Condition _condition);

	

	D2D1_RECT_F GetSrcRect()const
	{
		return srcRect;
	}

	int GetfSpriteFrameX()const
	{
		return fSpriteFrameX;
	}
	int GetfSpriteFrameY()const
	{
		return fSpriteFrameY;
	}
	void SetnFrameMax(int _nFrame)
	{
		nFrameMax = _nFrame;
	}

	float GetOneFrameLength()const
	{
		return oneFrameLength;
	}
	float GetOneFrameHeight()const
	{
		return oneFrameHeight;
	}
	void SetFrame(int _nFrame)
	{
		nFrame = _nFrame;
	}

	int GetFrame()const
	{
		return nFrame;
	}

	Paddle::Paddle(float _spriteStartX, float _spriteStartY, float _spriteEndX, float _spriteEndY, 
		int _fSpriteFrameX, int _fSpriteFrameY, int _frameMax);

	void detectFrame();

	void updateSourceRect();
	void updateBackGroundRect(int stepSpeed);

	void updateDestinRect(float startX, float Y, float endX, float speed, float distFactor);
	D2D1_RECT_F GetDestinactionRect()const
	{
		return desRect;
	}
	//use too much, set at public
	D2D1_RECT_F posRect;
	D2D1_RECT_F desRect;
	D2D1_SIZE_F vecSize;

	void simulation();

	void loadSpriteWH(float _spriteStartX, float _spriteStartY, float _spriteEndX, float _spriteEndY, int _frameX, int _frameY, int _frameMax);
	Paddle();
	~Paddle();


};
