#pragma once

#include <d2d1.h> // Include the Direct2D API.
#pragma comment(lib, "d2d1.lib") // Connect to the Direct2D Import Library.

class BulletChi
{
private:
	int smallNormalHurt ;
	int bigNormalHurt;
	int smallSuperHurt ;
	int bigSuperHurt;
public:
	
	BulletChi(float _spriteStartX, float _spriteStartY, float _spriteEndX, float _spriteEndY)
	{
		sourceLaction.left = _spriteStartX;
		sourceLaction.top = _spriteStartY;
		sourceLaction.right = _spriteEndX ;
		sourceLaction.bottom = _spriteEndY;

		W = _spriteEndX - _spriteStartX;
		H = _spriteEndY - _spriteStartY;
		counter = 0;
		shoot = false;
		smallNormalHurt = 50;
		bigNormalHurt = 200;
		smallSuperHurt = 150;
		bigSuperHurt = 350;
	}

	float W;
	float H;
	int counter;
	//seven small chi
	D2D1_RECT_F location;
	D2D1_RECT_F sourceLaction;

	bool shoot;

	bool GetShoot()const
	{
		return shoot;
	}

	void SetShoot(bool _shoot)
	{
		shoot = _shoot;
	}
	BulletChi();
	~BulletChi();
	
	int GetSmallNormalHurt()const
	{
		return smallNormalHurt;
	}
	int GetBigNormalHurt()const
	{
		return bigNormalHurt;
	}
	int GetSmallSuperHurt()const
	{
		return smallSuperHurt;
	}
	int GetBigSuperHurt()const
	{
		return bigSuperHurt;
	}
};

