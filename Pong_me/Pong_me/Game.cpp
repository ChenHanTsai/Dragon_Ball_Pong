#include "Game.h"

Game::Game(void)
{
	bRunning = TRUE;
	hWnd = NULL;
	ZeroMemory(keys, sizeof(keys));

	pD2DFactory = NULL;
	pRT = NULL;
	pBrush = NULL;
	pDWFactory = NULL;
	pTF = NULL;

	// Zero out the frames per second variables:
	dwFrames = 0;
	dwCurrentTime = 0;
	dwLastUpdateTime = 0;
	dwElapsedTime = 0;
	szFPS = _T("");
	gameOver = false;
	titleScreen = true;
	deltaTimeNext = 100;
	
}

Game::~Game(void)
{
	for (int i = 0; i < 3; i++)
		delete fly[i];

	for (int i = 0; i < 4; i++)
	delete racerBackRound[i];
}

void Game::initialPaddle()
{
	//TODO: Initialize Game Components
	float x1 = pRT->GetSize().width / 2;
	float y1 = pRT->GetSize().height / 2;
	float x2 = pRT->GetSize().width / 2 + ball.GetOneFrameLength();
	float y2 = pRT->GetSize().height / 2 + ball.GetOneFrameHeight();
	ball.posRect = D2D1::RectF(x1, y1, x2, y2);
	ball.vecSize = D2D1::SizeF(-7, 3);

	//initial position and velocity
	x1 = pRT->GetSize().width - 190;
	y1 = pRT->GetSize().height / 2 - enemy.GetOneFrameHeight();
	x2 = pRT->GetSize().width - 80 + enemy.GetOneFrameLength() * 2;
	y2 = pRT->GetSize().height / 2 + enemy.GetOneFrameHeight();
	enemy.posRect = D2D1::RectF(x1, y1, x2, y2);
	enemy.vecSize = D2D1::SizeF(0, 5);

	x1 = 10;
	y1 = pRT->GetSize().height / 2;// -player.GetBodyH();
	x2 = x1 + (float)player.GetOneFrameLength() * 2;
	y2 = y1 + player.GetOneFrameHeight() * 2;
	player.posRect = D2D1::RectF(x1, y1, x2, y2);
	player.vecSize = D2D1::SizeF(0, 5);

	background.updateSourceRect();
	//backgroundMoon.updateSourceRect();
	fly[0] = new Paddle(400, 10, 480, 70, 1, 1, 1);
	fly[1] =  new Paddle(40, 10, 330, 76, 6, 1, 6);
	fly[2] = new Paddle(40, 100, 130, 170, 1, 1, 1);
	for (int i = 0; i < 3; i++)
		fly[i]->updateSourceRect();


	for (int i = 0; i < 4; i++)
	{
		racerBackRound[i] = new Paddle(0.0f,200.0f+i*60.0f,619.0f,260.0f+i*60,9,1,9);
		racerBackRound[i]->updateSourceRect();
	}
	dwTime = GetTickCount() + getDeltaTimeNext();

	orginalState(player);

	orginalState(enemy);
	gameOver = false;
}

void Game::orginalState(Paddle& _paddle)
{
	_paddle.SetSuper(false);
	_paddle.SetChi(0);
	_paddle.SetLife(1000);
	_paddle.SetSuperCasePlay(false);
}
void Game::Startup(void)
{
	//TODO: Create Back Buffer

	// Initialize DirectX.
	HRESULT hr = CreateGraphics(hWnd);
	if (FAILED(hr))
	{
		return; // Return -1 to abort Window Creation.
	}

	//TODO: Load Bitmaps/Brushes/Pens/Fonts
	initialMusic();
	//initial paddle
	initialPaddle();
	gameOver = false;
	GameOverCounter = 0;
	howToPlay = 0;
	gameOverSound = false;
	//particles
	for (int i = 0; i < NUM_PARTICLE; i++)
	{
		particleVel[i] = D2D1::SizeF(0.0f, 0.0f);
		particleLife[i] = (float)(rand() % 50 + 50.0f);
	}
}

void Game::initialMusic()
{
	HRESULT hr = CreateGraphics(hWnd);
	//// Delete Music Voice
	if (pSourceVoiceMusic)
	{
		pSourceVoiceMusic->DestroyVoice();
		pSourceVoiceMusic = nullptr;
	}

	//// Music
	hr = music.LoadWave(L"Audio\\Chala.wav");
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to load music.wav."),
			_T("XAudio2 Error"), MB_OK | MB_ICONERROR);
		return;
	}

	hr = pXAudio2->CreateSourceVoice(&pSourceVoiceMusic, (WAVEFORMATEX*)&music.wfx);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed create source voice music."),
			_T("XAudio2 Error"), MB_OK | MB_ICONERROR);
		return;
	}

	//// Loop music
	//music.buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	hr = pSourceVoiceMusic->SubmitSourceBuffer(&music.buffer);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed submit music buffer."),
			_T("XAudio2 Error"), MB_OK | MB_ICONERROR);
		return;
	}


	hr = pSourceVoiceMusic->Start(0, XAUDIO2_COMMIT_NOW);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed start music buffer."),
			_T("XAudio2 Error"), MB_OK | MB_ICONERROR);
		return;
	}
}
void Game::Shutdown(void)
{
	//TODO: Unload Bitmaps/Brushes/Pens/Fonts

	//TODO: Destroy Back Buffer

	// Shutdown DirectX.
	DestroyGraphics();
}

void Game::limitUpInput(Paddle& _paddle)
{
	if (_paddle.posRect.top <= 0)
	{
		_paddle.posRect.top += _paddle.vecSize.height;
		_paddle.posRect.bottom += _paddle.vecSize.height;
	}
}

void Game::limitDownInput(Paddle& _paddle)
{
	if (_paddle.posRect.bottom >= pRT->GetSize().height)
	{
		_paddle.posRect.top -= _paddle.vecSize.height;
		_paddle.posRect.bottom -= _paddle.vecSize.height;
	
	}
}

void Game::InputMoveUp(Paddle& _paddle, CXBOXController& _CXBOXController, char _press)
{
	if (keys[_press] & 0x80 || _CXBOXController.GetState().Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP )
	{
		upExe(_paddle);
	}
}

void Game::InputMoveDown(Paddle& _paddle, CXBOXController& _CXBOXController, char _press)
{
	if (keys[_press] & 0x80 || _CXBOXController.GetState().Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
	{
		downExe(_paddle);
	}
}

void Game::AIAction(Paddle& _paddle)
{
	srand(static_cast<unsigned int>(time(nullptr)));

	int chooseSide = 0;

	if (&_paddle == &player)
		chooseSide = 1;

	switch (chooseSide)
	{
	
	case 0:
		//enemy's term
		//AI up for ball location is upper and 
		if (ball.posRect.top < _paddle.posRect.top && ball.vecSize.width>0)
			upExe(_paddle);
		else if (ball.posRect.top > _paddle.posRect.top && ball.vecSize.width>0)
			downExe(_paddle);

		break;
	case 1:
		//player's term
		//AI up for ball location is upper and 
		if (ball.posRect.top < _paddle.posRect.top && ball.vecSize.width<0)
			upExe(_paddle);
		else if (ball.posRect.top > _paddle.posRect.top && ball.vecSize.width<0)
			downExe(_paddle);

		break;
	}
	
 	if (_paddle.GetAIModeAction() == AI_WaitNextOrder)
	{
		int randNumberAI;

		randNumberAI = rand() % 100;

		if (randNumberAI > 60)
			_paddle.SetAIModeAction(AI_smallAtk);
		else if (randNumberAI > 20)
			_paddle.SetAIModeAction(AI_bigAtk);
		else if (randNumberAI > 0 && _paddle.GetSuper() == false)
			_paddle.SetAIModeAction(AI_Super);
	}

	switch (_paddle.GetAIModeAction())
	{
	case AI_smallAtk:
		if(smallAtkExe(_paddle))
			_paddle.SetAIModeAction(AI_WaitNextOrder);
		break;
	case AI_bigAtk:
		if (bigAtkExe(_paddle))
			_paddle.SetAIModeAction(AI_WaitNextOrder);
		break;
	case AI_Super:
		if (superExe(_paddle))
			_paddle.SetAIModeAction(AI_WaitNextOrder);
		break;
	}


}
void Game::InputEnemy()
{
	enemy.SetAIMode(GetCpu_2p());

	if (enemy.GetAIMode())
	{
		AIAction(enemy);
	}
	else
	{
		InputSmallButton(enemy, Controller2, VK_NUMPAD4);
		InputBigButton(enemy, Controller2, VK_NUMPAD8);
		SuperButton(enemy, Controller2, VK_NUMPAD6);

		InputMoveUp(enemy, Controller2, VK_UP);

		InputMoveDown(enemy, Controller2, VK_DOWN);
	}
}

void Game::InputPlayer()
{
	player.SetAIMode(GetCpu_1p());

	if (player.GetAIMode())
	{
		AIAction(player);
	}
	else
	{
		InputSmallButton(player, Controller1, 'F');
		InputBigButton(player, Controller1, 'T');
		SuperButton(player, Controller1, 'H');

		InputMoveUp(player, Controller1, 'W');

		InputMoveDown(player, Controller1, 'S');
	}



}
bool Game::smallAtkExe(Paddle& _paddle)
{
	if (_paddle.GetChi() >= _paddle.GetChiSmallConsum())
	{
		//whether super or not
		if (_paddle.GetSuper())
			_paddle.setSpriteCondition(Condition::c_smallAttackSuper);
		else
			_paddle.setSpriteCondition(Condition::c_smallAttack);

		_paddle.SetChi(_paddle.GetChi() - _paddle.GetChiSmallConsum());

		for (int i = 0; i < _paddle.getSmallChiNumber(); i++)
		{
			if (_paddle.smallAtk[i].GetShoot() == false)
			{
				//get to know which bullet type
				if (_paddle.GetSuper())
					_paddle.smallAtk[i] = BulletChi(370, 510, 410, 550);
				else
					_paddle.smallAtk[i] = BulletChi(90, 230, 110, 250);

				_paddle.smallAtk[i].SetShoot(true);
				_paddle.smallAtk[i].location.top = _paddle.posRect.top;

				int chooseSide = 0;

				if (&_paddle == &player)
					chooseSide = 1;

				switch (chooseSide)
				{
					//enemy's term
				case 0:
					audioSubmit(smallAtk[0]);
					_paddle.smallAtk[i].location.left = _paddle.posRect.left - 100;
					break;

				case 1:
					//player's term
					audioSubmit(smallAtk[1]);
					_paddle.smallAtk[i].location.left = _paddle.posRect.left + 100;
					break;
				}
				i = _paddle.getSmallChiNumber();
			}
		}
		return true;
	}
	return false;
}

bool Game::bigAtkExe(Paddle& _paddle)
{
	if (_paddle.GetChi() >= _paddle.GetChiBigConsum())
	{
		if (_paddle.GetSuper())
			_paddle.setSpriteCondition(Condition::c_bigAttackSuper);
		else
			_paddle.setSpriteCondition(Condition::c_bigAttack);

		_paddle.SetChi(_paddle.GetChi() - _paddle.GetChiBigConsum());

		for (int i = 0; i < _paddle.getBigChiNumber(); i++)
		{
			if (_paddle.bigAtk[i].GetShoot() == false)
			{
				//get to know which bullet type
				if (_paddle.GetSuper())
					_paddle.bigAtk[i] = BulletChi(360, 580, 438, 644);
				else
					_paddle.bigAtk[i] = BulletChi(370, 220, 420, 261);

				_paddle.bigAtk[i].SetShoot(true);

				int chooseSide = 0;

				if (&_paddle == &player)
					chooseSide = 1;

				switch (chooseSide)
				{
					//enemy's term
				case 0:
					audioSubmit(bigAtk[0]);
					_paddle.bigAtk[i].location.left = _paddle.posRect.left - 200;
					break;
				case 1:
					//player's term
					audioSubmit(bigAtk[1]);
					_paddle.bigAtk[i].location.left = _paddle.posRect.left + 100;
					break;
				}
				_paddle.bigAtk[i].location.top = _paddle.posRect.top - 100;

				i = _paddle.getBigChiNumber();
			}
		}
		return true;
	}
	return false;
}
bool Game::superExe(Paddle& _paddle)
{
	if (_paddle.GetSuper() == false && _paddle.GetChi() >= _paddle.GetChangeConsum())
	{
		_paddle.SetChi(_paddle.GetChi() - _paddle.GetChangeConsum());
		_paddle.setSpriteCondition(c_normalSuper);
		_paddle.SetSuper(true);
		_paddle.SetSuperCasePlay(true);

		int chooseSide = 0;

		if (&_paddle == &player)
			chooseSide = 1;

		switch (chooseSide)
		{
			//enemy's term
		case 0:
			audioSubmit(PowerUp[0]);
			break;

		case 1:
			//player's term
			audioSubmit(PowerUp[1]);
			break;
		}
		return true;
	}

	return false;
}
void Game::upExe(Paddle& _paddle)
{	// TODO: Up Arrow Key down action.
	_paddle.posRect.top -= _paddle.vecSize.height;
	_paddle.posRect.bottom -= _paddle.vecSize.height;

	limitUpInput(_paddle);
}
void Game::downExe(Paddle& _paddle)
{// TODO: DOWN Arrow Key down action.
	_paddle.posRect.top += _paddle.vecSize.height;
	_paddle.posRect.bottom += _paddle.vecSize.height;

	limitDownInput(_paddle);
}
void Game::SuperButton(Paddle& _paddle, CXBOXController& _CXBOXController, char _press)
{
	_paddle.superButton.bPrev = _paddle.superButton.bCurr;
	_paddle.superButton.bCurr = (keys[_press] & 0x80 || _CXBOXController.GetState().Gamepad.wButtons & XINPUT_GAMEPAD_X);// == 0x80;
	if (_paddle.superButton.bPrev == false && _paddle.superButton.bCurr == true)
	{
		superExe(_paddle);
	}
}

//small atk button
void Game::InputSmallButton(Paddle& _paddle, CXBOXController& _CXBOXController, char _press)
{
	_paddle.smallButton.bPrev = _paddle.smallButton.bCurr;
	_paddle.smallButton.bCurr = (keys[_press] & 0x80 || _CXBOXController.GetState().Gamepad.wButtons & XINPUT_GAMEPAD_A
		|| _CXBOXController.GetState().Gamepad.wButtons & XINPUT_GAMEPAD_Y);// == 0x80;
	InputSmallAtk(_paddle, _paddle.smallButton.bPrev, _paddle.smallButton.bCurr);
}

//small atk setting
void Game::InputSmallAtk(Paddle& _paddle, bool bPrevSpace, bool bCurrSpace)
{
	if (bPrevSpace == false && bCurrSpace == true )
	{
		smallAtkExe(_paddle);
	}
}

void Game::InputBigButton(Paddle& _paddle, CXBOXController& _CXBOXController,char _press)
{
	_paddle.bigButton.bPrev = _paddle.bigButton.bCurr;
	_paddle.bigButton.bCurr = (keys[_press] & 0x80 || _CXBOXController.GetState().Gamepad.wButtons & XINPUT_GAMEPAD_B);// == 0x80;
	
	InputBigAtk(_paddle, _paddle.bigButton.bPrev, _paddle.bigButton.bCurr);
	
}
void Game::InputBigAtk(Paddle& _paddle, bool bPrevSpace, bool bCurrSpace)
{
	if (bPrevSpace == false && bCurrSpace == true)
	{
		bigAtkExe(_paddle);
	}
}

void Game::changeStatePre(Paddle& _paddle)
{
	_paddle.pre = _paddle.GetSpriteCondition();
}

GAMESTATE Game::title(void)
{
	syatemSpace.bPrev = syatemSpace.bCurr;
	syatemSpace.bCurr = (keys[VK_SPACE] & 0x80);// == 0x80;
	if (syatemSpace.bPrev == false && syatemSpace.bCurr == true)
	{
		howToPlay++;

		if (howToPlay == 4)
		{
			titleScreen = false;
			return GAME_INPUT;
		}

	}
	
	
	return GAME_DRAW;
	
}
void Game::systemInput()
{
	//TODO: Read User Input
	if (keys[VK_ESCAPE] & 0x80)
	{
		PostMessage(hWnd, WM_CLOSE, 0, 0);
	}


	if (keys[VK_ADD] & 0x80)
	{
		// Increase the volume
		float vol;

		pMasterVoice->GetVolume(&vol);
		vol += 0.1f;
		if (vol > 2.0f)
		{
			vol = 2.0f;
		}

		pMasterVoice->SetVolume(vol);
	}

	if (keys[VK_SUBTRACT] & 0x80)
	{
		// Decrease the volume
		float vol;

		pMasterVoice->GetVolume(&vol);

		vol -= 0.1f;
		if (vol < 0.0f)
		{
			vol = 0.0f;
		}

		pMasterVoice->SetVolume(vol);
	}
}
GAMESTATE Game::Input(void)
{
	if (gameOver)
	{
		syatemSpace.bPrev = syatemSpace.bCurr;
		syatemSpace.bCurr = (keys[VK_SPACE] & 0x80);// == 0x80;
		if (syatemSpace.bPrev == false && syatemSpace.bCurr == true)
		{
			return GAME_INIT;
		}
	}
	else if (player.GetSuperCasePlay())
	{
		//no input update, only draw pic
	}
	else if (enemy.GetSuperCasePlay())
	{
		//no input update, only draw pic
	}
	else
	{
		changeStatePre(player);
		changeStatePre(enemy);

		systemInput();

		if (player.GetLife()>0)
		InputPlayer();

		if (enemy.GetLife()>0)
		InputEnemy();

	}
	return GAME_UPDATE;
}


void Game::ballUpdate()
{
	ball.posRect.left += ball.vecSize.width;
	ball.posRect.top += ball.vecSize.height;
	ball.posRect.right += ball.vecSize.width;
	ball.posRect.bottom += ball.vecSize.height;
}
void Game::ballAndWallSim()
{
	//detect top and bottom
	if (ball.posRect.bottom > pRT->GetSize().height || ball.posRect.top < 0)
	{
		ball.vecSize.height = -ball.vecSize.height;
	}

	//detect left and right
	if (ball.posRect.left < 0 || ball.posRect.right > pRT->GetSize().width)
	{
		//right side(enemy) hurt
		if (ball.vecSize.width >= 0)
		{
			ballHurtPaddle(enemy);
			audioSubmit(hurt[0]);
		}
		else
		{
			//left side(player) hurt
			ballHurtPaddle(player);
			audioSubmit(hurt[1]);
		}
		ball.vecSize.width = -ball.vecSize.width;
	}

}

void Game::ballHurtPaddle(Paddle& _paddle)
{
	if (_paddle.GetSuper())
		_paddle.setSpriteCondition(Condition::c_hitSuper);
	else
	_paddle.setSpriteCondition(Condition::c_hit);

	_paddle.SetChi(_paddle.GetChi() - 1);
}
void Game::loadSpriteCondtion(Paddle& _paddle)
{
	//Note LoadSpriteWH(startX,startY,endX,endY,xFrame,yFrame,Total Frame);
	switch (_paddle.GetSpriteCondition())
	{
	case Condition::c_recounter:
		_paddle.loadSpriteWH(0,  120,  200, 160, 5, 1, 5);
		break;
	case Condition::c_hit:
		_paddle.loadSpriteWH(120, 164, 400, 214, 7, 1, 7);
		break;
	case Condition::c_normal:
		_paddle.loadSpriteWH(240, 50, 440, 110, 3, 1, 3);
		break;
	case Condition::c_smallAttack:
		_paddle.loadSpriteWH(0, 220, 60, 431, 1, 5, 5);
		break;
	case Condition::c_bigAttack:
		_paddle.loadSpriteWH(110, 220, 350, 260, 6, 1, 6);
		break;
	case Condition::c_lose:
		_paddle.loadSpriteWH(110, 270, 310, 310, 4, 1, 4);
		break;
	case Condition::c_recounterSuper:
		_paddle.loadSpriteWH(30, 597, 270, 648, 5, 1, 5);
		break;
	case Condition::c_hitSuper:
		_paddle.loadSpriteWH(20, 534, 260, 590, 6, 1, 6);
		break;
	case Condition::c_normalSuper:
		_paddle.loadSpriteWH(10, 450, 216, 513, 3, 1, 3);
		break;
	case Condition::c_smallAttackSuper:
		_paddle.loadSpriteWH(340, 270, 380, 480, 1, 4, 4);
		break;
	case Condition::c_bigAttackSuper:
		_paddle.loadSpriteWH(80, 321, 320, 370, 6, 1, 6);
		break;
	case Condition::c_loseSuper:
		_paddle.loadSpriteWH(21, 660, 221, 710, 4, 1, 4);
		break;
	}
}
void Game::changeStateDetect(Paddle& _paddle)
{
	//change the state, need to update 
	_paddle.now = _paddle.GetSpriteCondition();

	if (_paddle.pre != _paddle.now)
	{
		_paddle.updateSourceRect();
		_paddle.alreadyUpdate = true;
	}
}


void Game::SetCPU(bool _1p, bool _2p)
{
	SetCpu_1p(_1p);
	SetCpu_2p(_2p);
}
GAMESTATE Game::Simulate(void)
{
	if (enemy.GetLife() <= 0 )
	{
		GameOverCounter += getDeltaTimeNext();

		if (enemy.GetSuper())
			enemy.setSpriteCondition(Condition::c_loseSuper);
		else
			enemy.setSpriteCondition(Condition::c_lose);

		if (gameOverSound == false)
			audioSubmit(hurt[0]);

		gameOverSound = true;

	}

	if (player.GetLife() <= 0)
	{
		GameOverCounter += 100;

		if (player.GetSuper())
			player.setSpriteCondition(Condition::c_loseSuper);
		else
			player.setSpriteCondition(Condition::c_lose);
		
		if (gameOverSound ==false)
		  audioSubmit(hurt[1]);

		gameOverSound = true;
	}


	if (GameOverCounter >= 20000)
	{
		gameOver = true;
	}

	if (gameOver)
	{

	}
	else
	{
		//if it is a animation, then need to detectFrame
		player.detectFrame();
		enemy.detectFrame();
		ball.detectFrame();

		//fot fly backGround
		for (int i = 0; i < 3; i++)
		{
			fly[i]->detectFrame();
		}

		//fot racer backGround
		for (int i = 0; i < 4; i++)
		{
			racerBackRound[i]->detectFrame();
		}

		//stop the vball
		if (player.GetSuperCasePlay() == false && enemy.GetSuperCasePlay() == false)
		{
			ballUpdate();

			ballAndWallSim();
			ballAndRaddleSim();

			chiBulletSim();
			chiAndRaddleSim(player);
			chiAndRaddleSim(enemy);
		}
		

		loadSpriteCondtion(player);
		loadSpriteCondtion(enemy);

		changeStateDetect(player);
		changeStateDetect(enemy);

		//update
		if (GetTickCount() > dwTime)
		{		
			dwTime = GetTickCount() + getDeltaTimeNext();

			//if it can play super play, then update
			if (player.GetSuperCasePlay())
			{
				superPlayCarot.updateSourceRect();
				
				setDeltaTimeNext(1000);

				//play all the super frame
				if (superPlayCarot.GetFrame() == superPlayCarot.GetFrameMax() + 1)
				{
					player.SetSuperCasePlay(false);
					setDeltaTimeNext(100);
				}
				
			}
			else if (enemy.GetSuperCasePlay())
			{
				superPlayCooler.updateSourceRect();

				setDeltaTimeNext(1000);

				//play all the super frame
				if (superPlayCooler.GetFrame() == superPlayCooler.GetFrameMax() + 1)
				{
					enemy.SetSuperCasePlay(false);
					setDeltaTimeNext(100);
				}

			}
			else
			{
				//other normal situation
				setDeltaTimeNext(100);
				
				if (player.alreadyUpdate == false)
					player.updateSourceRect();

				background.updateBackGroundRect(1);

				for (int i = 0; i < 3; i++)
				{
					fly[i]->updateDestinRect(0, 150.0f + i, (float)pRT->GetSize().width, 1.5f + 1.5f*i, 1 + i*0.1f);
					fly[i]->updateSourceRect();
				}

				for (int i = 0; i < 4; i++)
				{
					racerBackRound[i]->updateDestinRect(0 + i * 50.0f, (float)pRT->GetSize().height - 100.0f,
						(float)pRT->GetSize().width, 1.5f + 1.5f*i, 1 + i*0.3f);
					racerBackRound[i]->updateSourceRect();
				}

				enemy.updateSourceRect();
				ball.updateSourceRect();

			}
			
		}

		player.alreadyUpdate = false;
	}

	return GAME_DRAW;
}

void Game::chiAndRaddleSim(Paddle& _paddle)
{
	// Rectangle to Rectangle Intersection:
	RECT rIntersect;
	RECT rPaddle = { (LONG)enemy.posRect.left - 1, (LONG)enemy.posRect.top, (LONG)enemy.posRect.left, (LONG)enemy.posRect.bottom };
	RECT lPaddle = { (LONG)player.posRect.right - 1, (LONG)player.posRect.top, (LONG)player.posRect.right, (LONG)player.posRect.bottom };

	//temp pointer to the opponent
	Paddle* opponentPaddle;
	RECT* opponentRECT;

	//setting the side
	int chooseSide = 0;

	if (&_paddle == &player)
		chooseSide = 1;

	switch (chooseSide)
	{
		//enemy's term
	case 0:
		opponentRECT = &lPaddle;
		opponentPaddle = &player;
		break;

			//player's term
	case 1:
		opponentRECT = &rPaddle;
		opponentPaddle = &enemy;
		break;

	}
	RECT Bullet;
	for (int i = 0; i < _paddle.getSmallChiNumber(); i++)
	{
		//only calculate the valid bullet
		if (_paddle.smallAtk[i].GetShoot() == true)
		{
			Bullet = { (LONG)_paddle.smallAtk[i].location.left, (LONG)_paddle.smallAtk[i].location.top,
				(LONG)_paddle.smallAtk[i].location.right, (LONG)_paddle.smallAtk[i].location.bottom };

			//small chi hurt the opponent
			if (IntersectRect(&rIntersect, &Bullet, opponentRECT))
			{
			//	ballHurtPaddle(*opponentPaddle);
				if (chooseSide == 1)
					audioSubmit(hurt[0]);
				else
					audioSubmit(hurt[1]);

				if (_paddle.GetSuper())
					opponentPaddle->SetLife(opponentPaddle->GetLife() - _paddle.smallAtk[i].GetSmallNormalHurt() - rand() % 50);
				else
					opponentPaddle->SetLife(opponentPaddle->GetLife() - _paddle.smallAtk[i].GetSmallSuperHurt() - rand() % 10);

				//already hurt player, so setting false
				_paddle.smallAtk[i].SetShoot(false);
			}
		}

		//only calculate the valid bullet
		if (_paddle.bigAtk[i].GetShoot() == true)
		{
			Bullet = { (LONG)_paddle.bigAtk[i].location.left, (LONG)_paddle.bigAtk[i].location.top,
				(LONG)_paddle.bigAtk[i].location.right, (LONG)_paddle.bigAtk[i].location.bottom };

			//big chi hurt the opponent
			if (IntersectRect(&rIntersect, &Bullet, opponentRECT))
			{
		//		ballHurtPaddle(*opponentPaddle);
				if (chooseSide == 1)
					audioSubmit(hurt[0]);
				else
					audioSubmit(hurt[1]);

				if (_paddle.GetSuper())
					opponentPaddle->SetLife(opponentPaddle->GetLife() - _paddle.bigAtk[i].GetBigSuperHurt() - rand() % 50);
				else
					opponentPaddle->SetLife(opponentPaddle->GetLife() - _paddle.bigAtk[i].GetBigNormalHurt() - rand() % 50);

				//already hurt player, so setting false
				_paddle.bigAtk[i].SetShoot(false);
			}
		}
	}
}
void Game::ballAndRaddleSim()
{
	// Rectangle to Rectangle Intersection:
	RECT rIntersect;
	RECT rBall = { (LONG)ball.posRect.left, (LONG)ball.posRect.top, (LONG)ball.posRect.right, (LONG)ball.posRect.bottom };
	RECT rPaddle = { (LONG)enemy.posRect.left-1, (LONG)enemy.posRect.top, (LONG)enemy.posRect.left , (LONG)enemy.posRect.bottom };
	RECT lPaddle = { (LONG)player.posRect.right - 1, (LONG)player.posRect.top, (LONG)player.posRect.right, (LONG)player.posRect.bottom };

	
	if (IntersectRect(&rIntersect, &rBall, &rPaddle))
	{
		// TODO: Intersection Response.
		//only this situation need to detect intersection
		if (ball.vecSize.width > 0)
		{
			ball.vecSize.width = -ball.vecSize.width;
			PaddleRecounter(enemy);	
			audioSubmit(hit[0]);
		}
	}


	if (IntersectRect(&rIntersect, &rBall, &lPaddle))
	{
		// TODO: Intersection Response.
		//only this situation need to detect intersection
		if (ball.vecSize.width < 0)
		{
			ball.vecSize.width = -ball.vecSize.width;
			PaddleRecounter(player);
			audioSubmit(hit[1]);
		}
	}
}

void Game::PaddleRecounter(Paddle& _paddle)
{
	if (_paddle.GetSuper())
		_paddle.setSpriteCondition(Condition::c_recounterSuper);
	else
		_paddle.setSpriteCondition(Condition::c_recounter);

	_paddle.SetChi(_paddle.GetChi() + 1);

	if (_paddle.GetChi() >= 7)
		_paddle.SetChi(7);
}
void Game::renderEnviroment(D2D1_COLOR_F _gameBoardColor, D2D1_COLOR_F O_FILL_Color_Start, D2D1_COLOR_F O_FILL_Color_Stop)
{
	drawChi(player);
	drawChi(enemy);

	drawLife(player, _gameBoardColor, O_FILL_Color_Start, O_FILL_Color_Stop);
	drawLife(enemy, _gameBoardColor, O_FILL_Color_Start, O_FILL_Color_Stop);

	drawBullet();
}

void Game::drawChi(Paddle& _paddle)
{
	D2D1_RECT_F temp;
	float offset = 0;

	if (&_paddle == &enemy)
		offset = 600;

	for (int i = 0; i < _paddle.GetChi(); i++)
	{
		temp.left = 100.0f + i*50.0f + offset;
		temp.top = 120.0f;
		temp.right = temp.left + 50.0f ;
		temp.bottom = temp.top + 50.0f;
		LifeBoard.updateSourceRect();
		LifeBoard.detectFrame();

		renderParticle(temp,0);

		pRT->DrawBitmap(LifeBoard.sprite, temp, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, LifeBoard.GetSrcRect());



	}
}
void Game::bigChiSim(int _number, Paddle& _paddle, int countShoot, float speedDirection)
{
	for (int i = 0; i < _number; i++)
	{
		
		if (_paddle.bigAtk[i].GetShoot() == true )
		{
			_paddle.bigAtk[i].counter++;

			//need to prepare the action
			if (_paddle.bigAtk[i].counter > countShoot)
				_paddle.bigAtk[i].location.left += speedDirection;

			//setting the side
			int chooseSide = 0;

			if (&_paddle == &player)
				chooseSide = 1;

			switch (chooseSide)
			{
			case 0:
				//enemy's turn
				if (_paddle.bigAtk[i].location.left < 0)
				{
					_paddle.bigAtk[i].SetShoot(false);
					_paddle.bigAtk[i].counter = 0;
				}
				break;

			case 1:
				//player's term
				if (_paddle.bigAtk[i].location.left > pRT->GetSize().width)
				{
					_paddle.bigAtk[i].SetShoot(false);
					_paddle.bigAtk[i].counter = 0;
				}
				break;

			}

					
		}
	}
}
void Game::smallChiSim(int _number, Paddle& _paddle, int countShoot, float speedDirection)
{
	for (int i = 0; i < _number; i++)
	{
		
		if (_paddle.smallAtk[i].GetShoot() == true )
		{
			_paddle.smallAtk[i].counter++;

			if (_paddle.smallAtk[i].counter > countShoot)
				_paddle.smallAtk[i].location.left += speedDirection;

			//setting the side
			int chooseSide = 0;

			if (&_paddle == &player)
				chooseSide = 1;

			switch (chooseSide)
			{
			case 0:
				//enemy's turn
				if (_paddle.smallAtk[i].location.left < 0)
				{
					_paddle.smallAtk[i].SetShoot(false);
					_paddle.smallAtk[i].counter = 0;
				}
			break;

			case 1:
				//player's term
				if (_paddle.smallAtk[i].location.left > pRT->GetSize().width)
				{
					_paddle.smallAtk[i].SetShoot(false);
					_paddle.smallAtk[i].counter = 0;
				}
				break;
			
			}				
		}
	}
}
void Game::chiBulletSim()
{
	smallChiSim(player.getSmallChiNumber(), player, player.getSmallChiCounter(),4.0f);
	bigChiSim(player.getBigChiNumber(), player, player.getBigChiCounter(), 6.0f);
	
	smallChiSim(enemy.getSmallChiNumber(), enemy, enemy.getSmallChiCounter(), -4.0f);
	bigChiSim(enemy.getBigChiNumber(), enemy, enemy.getBigChiCounter(), -6.0f);
}


void Game::drawSmallBullet(int _number, Paddle& _paddle)
{
	for (int i = 0; i < _number; i++)
	{
		if (_paddle.smallAtk[i].GetShoot() == true)
		{
			_paddle.smallAtk[i].location.right = _paddle.smallAtk[i].location.left + _paddle.smallAtk[i].W * 6;
			_paddle.smallAtk[i].location.bottom = _paddle.smallAtk[i].location.top + _paddle.smallAtk[i].H * 6;
			float tempX = _paddle.smallAtk[i].location.left + _paddle.smallAtk[i].W * 3;
			float tempY = _paddle.smallAtk[i].location.top + _paddle.smallAtk[i].H * 3;
		
			// Render bullet on top of his shadow
			if (_paddle.smallAtk[i].counter > _paddle.getSmallChiCounter())
			{
				pRT->DrawBitmap(_paddle.sprite, _paddle.smallAtk[i].location, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, _paddle.smallAtk[i].sourceLaction);
			
			}
		}
	}
}

void Game::drawBigBullet(int _number, Paddle& _paddle)
{
	for (int i = 0; i < _number; i++)
	{
		if (_paddle.bigAtk[i].GetShoot() == true)
		{
			_paddle.bigAtk[i].location.right = _paddle.bigAtk[i].location.left + _paddle.bigAtk[i].W * 6;
			_paddle.bigAtk[i].location.bottom = _paddle.bigAtk[i].location.top + _paddle.bigAtk[i].H * 6;
			float tempX = _paddle.bigAtk[i].location.left + _paddle.bigAtk[i].W * 3;
			float tempY = _paddle.bigAtk[i].location.top + _paddle.bigAtk[i].H * 3;
			//set the transform
			pRT->SetTransform(D2D1::Matrix3x2F::Skew(5.0f, -10.0f, D2D1::Point2F(tempX, tempY)));
			pBrush->SetColor(D2D1::ColorF(0x000000, 0.4f));
			// Antialias mode must be changed for FillOpacityMask to work
			pRT->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
			pRT->FillOpacityMask(_paddle.sprite, pBrush,
				D2D1_OPACITY_MASK_CONTENT_GRAPHICS,
				_paddle.bigAtk[i].location,
				_paddle.bigAtk[i].sourceLaction);

			// Set antialias mode back to default
			pRT->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

			// Set the transform back to the identity matrix
			pRT->SetTransform(D2D1::Matrix3x2F::Identity());
			
			// Render bullet on top of his shadow
			if (_paddle.bigAtk[i].counter > _paddle.getBigChiCounter())
			{
				pRT->DrawBitmap(_paddle.sprite, _paddle.bigAtk[i].location, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, _paddle.bigAtk[i].sourceLaction);
		//		pRT->DrawRectangle(_paddle.bigAtk[i].location, pBrush, GetLineWidth());
			}
		}
	}
}
void Game::drawBullet()
{
	drawSmallBullet(7, player);
	drawBigBullet(2,player);

	drawSmallBullet(7, enemy);
	drawBigBullet(2, enemy);
	// Set the transform back to the identity matrix
	pRT->SetTransform(D2D1::Matrix3x2F::Identity());

}
void Game::drawLife(Paddle& _paddle, D2D1_COLOR_F _gameBoardColor, D2D1_COLOR_F O_FILL_Color_Start, D2D1_COLOR_F O_FILL_Color_Stop)
{

	//draw the life board

	if (player.GetLife() <=0 )
	{
		player.SetLife(0);
	}

	if (enemy.GetLife() <=0)
	{
		enemy.SetLife(0);
	}

	D2D1_RECT_F rect;
	for (int i = 0; i < 2; i++)
	{
		int temp = 450;
		int bloodL = 400;
		if (i == 0)
		{
			rect.left = pRT->GetSize().width / 2 - temp;
			
		}
		else
		{
			rect.left = pRT->GetSize().width - temp - 120;
	
		}
		rect.top = 20;
		rect.bottom = rect.top + 50;

		if (m_pLinearGradientBrush == NULL)
		{
			gradientStops[0].color = O_FILL_Color_Start;
			gradientStops[0].position = 0.0f;
			gradientStops[1].color = O_FILL_Color_Stop;
			gradientStops[1].position = 1.0f;

			pRT->CreateGradientStopCollection(
				gradientStops,
				2,
				D2D1_GAMMA_2_2,
				D2D1_EXTEND_MODE_CLAMP,
				&pGradientStops
				);

			pRT->CreateLinearGradientBrush(
				D2D1::LinearGradientBrushProperties(
				D2D1::Point2F(100, 0),
				D2D1::Point2F(100, 150)),
				pGradientStops,
				&m_pLinearGradientBrush
				);
		}	

		if (i == 0)
			rect.right = rect.left + player.GetLife() / 1000 * bloodL;

		if (i == 1)
			rect.right = rect.left + enemy.GetLife() / 1000 * bloodL;

	
			pRT->FillRectangle(rect, m_pLinearGradientBrush);
		//pRT->FillRectangle(rect, pBrush);

		rect.right = rect.left + bloodL;
	
		//draw the board first
		pBrush->SetColor(_gameBoardColor);
		pRT->DrawRectangle(rect, pBrush, GetLineWidth());
	
	
	}

	SafeRelease(&pGradientStops);
	SafeRelease(&m_pLinearGradientBrush);
}

void Game::renderAction(Paddle& _paddle)
{
	//need to update different frame x and y
	_paddle.posRect.right = _paddle.posRect.left + _paddle.GetOneFrameLength() * 3;
	_paddle.posRect.bottom = _paddle.posRect.top + _paddle.GetOneFrameHeight() * 3;
	pRT->DrawBitmap(_paddle.sprite, _paddle.posRect, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, _paddle.GetSrcRect());
	
}
void Game::renderPaddle()
{
	renderAction(player);

	D2D1_RECT_F temp;
	temp.top = enemy.posRect.top + enemy.GetOneFrameHeight() * 3 / 2;
	temp.left = enemy.posRect.left + enemy.GetOneFrameLength() * 3 / 2;
	pRT->SetTransform(D2D1::Matrix3x2F::Scale(-1.0f, 1.0f, D2D1::Point2F(temp.left, temp.top)));

	renderAction(enemy);


	// Set the transform back to the identity matrix
	pRT->SetTransform(D2D1::Matrix3x2F::Identity());

	renderParticle(ball.posRect, 1);
	pRT->DrawBitmap(ball.sprite, ball.posRect, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, ball.GetSrcRect());
	

}

void Game::renderBackGround()
{
	//* Render background

	//backGround
	pRT->DrawBitmap(background.sprite, D2D1::RectF(0, 0, pRT->GetSize().width, pRT->GetSize().height),
		1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, background.GetSrcRect());

	//flyer
	for (int i = 0; i < 3; i++)
	{
		pRT->DrawBitmap(backGroundAll, fly[i]->GetDestinactionRect(),
			1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, fly[i]->GetSrcRect());
	}

	//racer
	for (int i = 0; i < 4; i++)
	{
		pRT->DrawBitmap(backGroundAll, racerBackRound[i]->GetDestinactionRect(),
			1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, racerBackRound[i]->GetSrcRect());
	}
	
}

void Game::renderParticle(D2D1_RECT_F& temp,int _type)
{
	// Render Particles
	for (int i = 0; i < NUM_PARTICLE; i++)
	{

		pRT->DrawBitmap(particle[_type], D2D1::RectF(
			particlePos[_type][i].x - 20.0f, particlePos[_type][i].y - 10.0f,
			particlePos[_type][i].x + 50.0f, particlePos[_type][i].y + 60.0f),
			particleLife[i] / 100.0f);
		
		// Update its position (based on velocity)
		particlePos[_type][i].x += particleVel[i].width;
		particlePos[_type][i].y += particleVel[i].height;

		// Decrement its life
		particleLife[i]--;
		// Check if it is dead
		if (particleLife[i] <= 0)
		{
			// Reset its position and velocity and life
			particlePos[_type][i] = D2D1::Point2F(temp.left, temp.top);
			particleVel[i] = D2D1::SizeF(float(rand() % 9) - 4.0f,float(rand() % 3) - 3.0f);
			particleLife[i] = 20;
		}

	}
}
void Game::renderReplay(Paddle _paddle)
{
	if (enemy.GetLife() <= 0)
	{
		pRT->DrawBitmap(gameOverScreen[0], D2D1::RectF(0, 0, pRT->GetSize().width, pRT->GetSize().height));
	}
	else
	{
		pRT->DrawBitmap(gameOverScreen[1], D2D1::RectF(0, 0, pRT->GetSize().width, pRT->GetSize().height));
	}
	float x1 = 400;
	float y1 = pRT->GetSize().height / 2 + 300;
	float x2 = 800;
	float y2 = pRT->GetSize().height / 2 + 300;
	std::wostringstream wos;
	wos << "Press Space To Replay";
	D2D1_RECT_F rect;
	rect = D2D1::RectF(x1, y1, x2, y2);
	pTF->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	pTF->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	pRT->DrawText(wos.str().c_str(), wos.str().length(), pTF, rect, pBrush);
}

void Game::renderTitle(TCHAR szTitle[100],D2D1_COLOR_F& _gameBoardColor)
{
	float x1 = 100;
	float y1 = pRT->GetSize().height / 2 - 100;
	float x2 = 400;
	float y2 = pRT->GetSize().height / 2 + 100;
	std::wostringstream wos;
	switch (howToPlay)
	{
	case 0:
		pRT->DrawBitmap(titlePic[0], D2D1::RectF(0, 0, pRT->GetSize().width, pRT->GetSize().height));
		pBrush->SetColor(_gameBoardColor);
		
		wos << szTitle;
		D2D1_RECT_F rect;
		//																	
	
		rect = D2D1::RectF(x1, y1, x2, y2);
		pTF->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		pTF->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		pRT->DrawText(wos.str().c_str(), wos.str().length(), pTF, rect, pBrush);
	break;
	case 1:
		pRT->DrawBitmap(titlePic[1], D2D1::RectF(0, 0, pRT->GetSize().width, pRT->GetSize().height));
		break;
	case 2:
		pRT->DrawBitmap(titlePic[2], D2D1::RectF(0, 0, pRT->GetSize().width, pRT->GetSize().height));
		break;
	case 3:
		pRT->DrawBitmap(titlePic[3], D2D1::RectF(0, 0, pRT->GetSize().width, pRT->GetSize().height));
		break;
	}
	
	
}


GAMESTATE Game::Render(bool pause, TCHAR szTitle[100],TCHAR szPause[100], D2D1_COLOR_F _gameBoardColor, D2D1_COLOR_F O_FILL_Color_Start, D2D1_COLOR_F O_FILL_Color_Stop)
{
	
	pRT->BeginDraw();

	if (titleScreen)
	{
		renderTitle(szTitle,_gameBoardColor);
	}
	 else if (gameOver == true)
	{
		if (enemy.GetLife() <= 0)
			renderReplay(player);
		else if (player.GetLife() <= 0)
			renderReplay(enemy);
		
	}
	else
	{
		//TODO: Clear Back Buffer
		pRT->Clear(D2DColor(CornflowerBlue));

		renderBackGround();

		renderPaddle();

		renderEnviroment(_gameBoardColor, O_FILL_Color_Start, O_FILL_Color_Stop); 

		if (pause)
			renderPause(szPause, _gameBoardColor);

		//play super case
		if (player.GetSuperCasePlay())
		pRT->DrawBitmap(superPlayCarot.sprite, player.posRect, 1.0,
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, superPlayCarot.GetSrcRect());

		if (enemy.GetSuperCasePlay())
			pRT->DrawBitmap(superPlayCooler.sprite, enemy.posRect, 1.0,
			D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, superPlayCooler.GetSrcRect());
	}

	

	HRESULT hr = pRT->EndDraw();
	if (hr == D2DERR_RECREATE_TARGET)
	{
		//TODO: Destroy Direct2D.
		DestroyGraphics();
		//TODO: Recreate Direct2D.
		CreateGraphics(hWnd);
	}

	
	if (titleScreen)
	return GAME_TITLE;

	return GAME_INPUT;
}
void Game::renderPause(TCHAR szPause[100], D2D1_COLOR_F _gameBoardColor)
{
	pBrush->SetColor(_gameBoardColor);
	std::wostringstream wos;
	wos << szPause;
	D2D1_RECT_F rect;
	//																	
	float x1 = pRT->GetSize().width/2-100;
	float y1 = pRT->GetSize().height/ 2-100;
	float x2 = pRT->GetSize().width / 2 + 100;
	float y2 = pRT->GetSize().height / 2 + 100;
	rect = D2D1::RectF(x1, y1, x2, y2);
	pTF->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	pTF->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	pRT->DrawText(wos.str().c_str(), wos.str().length(), pTF, rect, pBrush);
}

HRESULT Game::CreateGraphics(HWND hWnd)
{
	// Initialize the Direct2D Factory.
	HRESULT hr;
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create the Direct2D Factory."),
			_T("Direct2D Error"), MB_OK | MB_ICONERROR);
		return hr;
	}


	// Get the dimensions of the client.
	RECT rc;
	GetClientRect(hWnd, &rc);

	// Initialize a Direct2D Size Structure.
	D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);


	// Create the Direct2D Render Target.
	hr = pD2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(hWnd, size), &pRT);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create the Direct2D Render Target."),
			_T("Direct2D Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	pRT->SetDpi(96.0f, 96.0f);


	// Create the Direct2D Solid Color Brush.
	hr = pRT->CreateSolidColorBrush(D2D1::ColorF(0x0), &pBrush);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create the Direct2D Solid Color Brush."),
			_T("Direct2D Error"), MB_OK | MB_ICONERROR);
		return hr;
	}


	// Initialize the DirectWrite Factory.
	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
		(IUnknown**)&pDWFactory);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create the DirectWrite Factory."),
			_T("DirectWrite Error"), MB_OK | MB_ICONERROR);
		return hr;
	}


	// Create the DirectWrite Text Format.
	hr = pDWFactory->CreateTextFormat(_T("Veranda"), NULL,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		24, _T(""), &pTF);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create the DirectWrite Text Format."),
			_T("DirectWrite Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	//pTF->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
	//pTF->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	//pTF->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	//* Load in bitmaps
	hr = CoCreateInstance(CLSID_WICImagingFactory,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pWICFactory));
	
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create the WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr
			;
	}
	
	//backgorund 
	hr = LoadBitmapFromFile(L"backgroundNormalLife.png", &background.sprite);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to load the backgroundNormalLife jpg."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}

	hr = LoadBitmapFromFile(L"superPlayCarot.png", &superPlayCarot.sprite);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to load the backgroundNormalLife jpg."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}

	hr = LoadBitmapFromFile(L"superPlayCooler.png", &superPlayCooler.sprite);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to load the backgroundNormalLife jpg."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}

	hr = LoadBitmapFromFile(L"backGroundAll.png", &backGroundAll);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to load the backGroundAll jpg."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}

	
	//load player.png
	hr = LoadBitmapFromFile(L"KidGokuAll.png", &player.sprite);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to load KidGokuAll.png."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	
	hr = LoadBitmapFromFile(L"titlePic.png", &titlePic[0]);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to load titlePic.png."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"joystick.bmp", &titlePic[1]);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to load joystick.png."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"keyboard.jpg", &titlePic[2]);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to load keyboard.png."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"skills.png", &titlePic[3]);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to load skills.png."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}

	hr = LoadBitmapFromFile(L"dragonballChi.png", &LifeBoard.sprite);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to load dragonballChi.png."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}

	hr = LoadBitmapFromFile(L"finish1.jpg", &gameOverScreen[0]);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to load finish1.jpg."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"finish2.jpg", &gameOverScreen[1]);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to load finish1.jpg."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}

	//load enemy.png
	hr = LoadBitmapFromFile(L"Cooler2.png", &enemy.sprite);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to load enemy.png."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"fire64.png", &particle[0]);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to load fire64.png."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}

	hr = LoadBitmapFromFile(L"electric.png", &particle[1]);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to load electric.png."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}

	//fail stuff some stuff
	//load enemy.png
	hr = LoadBitmapFromFile(L"ball2.png", &ball.sprite);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to load enemy.png."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}

	hr =pRT->CreateGradientStopCollection(
		gradientStops,
		2,
		D2D1_GAMMA_2_2,
		D2D1_EXTEND_MODE_CLAMP,
		&pGradientStops
		);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create the CreateGradientStopCollection."),
			_T("Direct2D Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	
	hr = pRT->CreateLinearGradientBrush(
		D2D1::LinearGradientBrushProperties(
		D2D1::Point2F(100, 0),
		D2D1::Point2F(100, 150)),
		pGradientStops,
		&m_pLinearGradientBrush
		);

	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create CreateLinearGradientBrush."),
			_T("Direct2D Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = pRT->CreateSolidColorBrush(D2D1::ColorF(0x0), &pBrushGrayBlack);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create the Direct2D Solid Color Brush."),
			_T("Direct2D Error"), MB_OK | MB_ICONERROR);
		return hr;
	}

	//sound//
	// SAPI
	hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pVoice));
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create the SAPI Interface."),
			_T("SAPI Error"), MB_OK | MB_ICONERROR);
		return hr;
	}

	////pVoice->Speak(L"Lets get started", SPF_ASYNC, NULL);

	//// XAudio2
	//// Factory
	hr = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create XAudio2 Interface."),
			_T("XAudio2 Error"), MB_OK | MB_ICONERROR);
		return hr;
	}

	//// Mastering Voice
	hr = pXAudio2->CreateMasteringVoice(&pMasterVoice);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create Master Voice."),
			_T("XAudio2 Error"), MB_OK | MB_ICONERROR);
		return hr;
	}

	//// Load in files
	for (int i = 0; i < 2; i++)
	{
		if (i == 0)
			hr = smallAtk[i].LoadWave(L"Audio\\smallAtk0.wav");
		else
			hr = smallAtk[i].LoadWave(L"Audio\\smallAtk1.wav");

		if (FAILED(hr))
		{
			MessageBox(HWND_DESKTOP, _T("ERROR: Failed to load smallAtk.wav."),
				_T("XAudio2 Error"), MB_OK | MB_ICONERROR);
			return hr;
		}
	}

	for (int i = 0; i < 2; i++)
	{
		if (i == 0)
			hr = bigAtk[i].LoadWave(L"Audio\\bigAtk0.wav");
		else
			hr = bigAtk[i].LoadWave(L"Audio\\bigAtk1.wav");

		if (FAILED(hr))
		{
			MessageBox(HWND_DESKTOP, _T("ERROR: Failed to load bigAtk.wav."),
				_T("XAudio2 Error"), MB_OK | MB_ICONERROR);
			return hr;
		}
	}
	for (int i = 0; i < 2; i++)
	{
		if (i == 0)
			hr = PowerUp[i].LoadWave(L"Audio\\PowerUp0.wav");
		else
			hr = PowerUp[i].LoadWave(L"Audio\\PowerUp1.wav");

		if (FAILED(hr))
		{
			MessageBox(HWND_DESKTOP, _T("ERROR: Failed to load PowerUp0.wav."),
				_T("XAudio2 Error"), MB_OK | MB_ICONERROR);
			return hr;
		}
	}

	for (int i = 0; i < 2; i++)
	{
		if (i == 0)
			hr = block[i].LoadWave(L"Audio\\block0.wav");
		else
			hr = block[i].LoadWave(L"Audio\\block1.wav");

		if (FAILED(hr))
		{
			MessageBox(HWND_DESKTOP, _T("ERROR: Failed to load block.wav."),
				_T("XAudio2 Error"), MB_OK | MB_ICONERROR);
			return hr;
		}
	}

	for (int i = 0; i < 2; i++)
	{
		if (i == 0)
			hr = hit[i].LoadWave(L"Audio\\hit0.wav");
		else
			hr = hit[i].LoadWave(L"Audio\\hit1.wav");

		if (FAILED(hr))
		{
			MessageBox(HWND_DESKTOP, _T("ERROR: Failed to load hit.wav."),
				_T("XAudio2 Error"), MB_OK | MB_ICONERROR);
			return hr;
		}
	}

	for (int i = 0; i < 2; i++)
	{
		if (i == 0)
			hr = hurt[i].LoadWave(L"Audio\\hurt0.wav");
		else
			hr = hurt[i].LoadWave(L"Audio\\hurt1.wav");

		if (FAILED(hr))
		{
			MessageBox(HWND_DESKTOP, _T("ERROR: Failed to load hurt.wav."),
				_T("XAudio2 Error"), MB_OK | MB_ICONERROR);
			return hr;
		}
	}

	for (int i = 0; i < 2; i++)
	{
		if (i == 0)
			hr = finish[i].LoadWave(L"Audio\\finish0.wav");
		else
			hr = finish[i].LoadWave(L"Audio\\finish1.wav");

		if (FAILED(hr))
		{
			MessageBox(HWND_DESKTOP, _T("ERROR: Failed to load finish.wav."),
				_T("XAudio2 Error"), MB_OK | MB_ICONERROR);
			return hr;
		}
	}

	
	//// Source Voices
	for (int i = 0; i < 10; i++)
	{
		hr = pXAudio2->CreateSourceVoice(&pSourceVoice[i], (WAVEFORMATEX*)&smallAtk[0].wfx);
		if (FAILED(hr))
		{
			MessageBox(HWND_DESKTOP, _T("ERROR: Failed create source voice."),
				_T("XAudio2 Error"), MB_OK | MB_ICONERROR);
			return hr;
		}

		hr = pSourceVoice[i]->Start(0);
		if (FAILED(hr))
		{
			MessageBox(HWND_DESKTOP, _T("ERROR: Failed start source voice."),
				_T("XAudio2 Error"), MB_OK | MB_ICONERROR);
			return hr;
		}
	}

	//gameOverPlayer
	return S_OK; // Success!
}

void Game::DestroyGraphics(void)
{
	//destroy wics
	
	//left right ball paddle
	SafeRelease(&player.sprite);
	SafeRelease(&enemy.sprite);
	SafeRelease(&ball.sprite);

	//super play
	SafeRelease(&superPlayCooler.sprite);
	SafeRelease(&superPlayCarot.sprite);

	//background
	SafeRelease(&background.sprite);
	
	//flyer and racer
	SafeRelease(&backGroundAll);

	//particle
	SafeRelease(&particle[0]);
	SafeRelease(&particle[1]);

	//wic
	SafeRelease(&pWICFactory);

	//title screen
	for (int i = 0; i < 3; i++)
	{
		SafeRelease(&titlePic[i]);
	}
	
	for (int i = 0; i < 2; i++)
	{
		SafeRelease(&gameOverScreen[i]);
	}
	
	//gradient
	SafeRelease(&pGradientStops);
	SafeRelease(&m_pLinearGradientBrush);

	//word
	SafeRelease(&pTF);
	SafeRelease(&pDWFactory);

	//brush
	SafeRelease(&pBrush);

	//Render target
	SafeRelease(&pRT);


	// Release the Direct2D Factory.
	SafeRelease(&pD2DFactory);

	//// Delete Music Voice
	if (pSourceVoiceMusic)
	{
		pSourceVoiceMusic->DestroyVoice();
		pSourceVoiceMusic = nullptr;
	}

	// Delete Source Voices
	for (int i = 0; i < 10; i++)
	{
		if (pSourceVoice[i])
		{
			pSourceVoice[i]->DestroyVoice();
			pSourceVoice[i] = nullptr;
		}
	}


	// Master Voice
	if (pMasterVoice)
	{
		pMasterVoice->DestroyVoice();
		pMasterVoice = nullptr;
	}

	// XAudio2
	SafeRelease(&pXAudio2);

}

HRESULT Game::LoadBitmapFromFile(LPCTSTR strFileName, ID2D1Bitmap** ppBitmap)
{
	HRESULT hr;

	IWICBitmapDecoder *pDecoder = NULL;
	IWICBitmapFrameDecode *pSource = NULL;
	IWICFormatConverter *pConverter = NULL;


	hr = pWICFactory->CreateDecoderFromFilename(
		strFileName,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&pDecoder
		);
	if (SUCCEEDED(hr))
	{
		// Create the initial frame.
		hr = pDecoder->GetFrame(0, &pSource);
	}
	if (SUCCEEDED(hr))
	{

		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = pWICFactory->CreateFormatConverter(&pConverter);
	}
	if (SUCCEEDED(hr))
	{
		hr = pConverter->Initialize(
			pSource,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			NULL,
			0.f,
			WICBitmapPaletteTypeMedianCut
			);
	}
	if (SUCCEEDED(hr))
	{
		// Create a Direct2D bitmap from the WIC bitmap.
		hr = pRT->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			ppBitmap
			);
	}

	SafeRelease(&pDecoder);
	SafeRelease(&pSource);
	SafeRelease(&pConverter);

	return hr;
}

int Game::GetFreeVoice()
{
	XAUDIO2_VOICE_STATE vs;
	int free = -1;

	for (int i = 0; i < 10; i++)
	{
		pSourceVoice[i]->GetState(&vs);
		if (vs.BuffersQueued == 0)
		{
			free = i;
			break;
		}
	}

	return free;
}

void  Game::audioSubmit(AudioData& _sound)
{
	int free = GetFreeVoice();
	if (free > -1)
	{
		pSourceVoice[free]->SubmitSourceBuffer(&_sound.buffer);
	}
}
GAMESTATE Game::gameReplay()
{
	gameOver = true;
	return GAME_DRAW;
}