#pragma once

#include <Windows.h> // Include the Win32 API
#include <tchar.h> // Include support for UNICODE
#include <sstream> // Include wostringstream support
using std::wostringstream;

#include <d2d1.h> // Include the Direct2D API.
#pragma comment(lib, "d2d1.lib") // Connect to the Direct2D Import Library.

#include <dwrite.h> // Include the DirectWrite API.
#pragma comment(lib, "dwrite.lib") // Connect to the DirectWrite Import Library.

#include <mmsystem.h> // Include the Multimedia header file.
#pragma comment(lib, "winmm.lib") // Connect to the Multimedia import library.

//WIC
#include <wincodec.h>
#pragma comment(lib,"windowsCodecs.lib")


#define NUM_PARTICLE 150

#include "Paddle.h"
#include "BulletChi.h"
#include "CXBOXController.h"

//=============sound//
#include <mmsystem.h> // Include the Multimedia header file.
#pragma comment(lib, "winmm.lib") // Connect to the Multimedia import library.

// SAPI
#include <sapi.h>

// XAudio2

#include <xaudio2.h>
//#pragma comment(lib, "xaudio2.lib")
#include "audiodata.h"
#include <ctime>
#include "PressDetect.h"
//controller
//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------

//#include <XInput.h>
//#pragma comment(lib,"Xinput9_1_0.lib")
#define MAX_CONTROLLERS 4  // XInput handles up to 4 controllers 
#define INPUT_DEADZONE  ( 0.24f * FLOAT(0x7FFF) )  // Default to 24% of the +/- 32767 range.   This is a reasonable default value but can be altered if needed.

enum GAMESTATE {
	GAME_INIT, GAME_TITLE, GAME_MENU, GAME_PLAY, GAME_END, GAME_INPUT, ROUND_END
	, GAME_OPTIONS, GAME_UPDATE, GAME_REPLAY, Game_Final_Screen, GAME_SORT_TYPE, GAME_DRAW, GAME_PLAYER_WIN, GAME_ENEMY_WIN
};


class Game
{
	// SAPI
	ISpVoice* pVoice;

	// XAudio2
	IXAudio2* pXAudio2;
	IXAudio2MasteringVoice* pMasterVoice;
	IXAudio2SourceVoice* pSourceVoice[10];
	IXAudio2SourceVoice* pSourceVoiceMusic;
	AudioData smallAtk[2];
	AudioData bigAtk[2];
	AudioData PowerUp[2];
	AudioData block[2];
	AudioData hit[2];
	AudioData hurt[2];
	AudioData finish[2];
	AudioData music;

	// Window Handle:
	HWND hWnd;

	// DirectX Helper Functions

	// The SafeRelease Pattern
	template <class T> void SafeRelease(T **ppT)
	{
		if (*ppT)
		{
			(*ppT)->Release();
			*ppT = NULL;
		}
	}

	// Programmer defined macro to make using the known colors easier.
	#define D2DColor(clr) D2D1::ColorF(D2D1::ColorF::clr)

	HRESULT CreateGraphics(HWND hWnd);
	void DestroyGraphics(void);

	// DirectX Interfaces:
	ID2D1Factory* pD2DFactory;
	ID2D1HwndRenderTarget* pRT;
	ID2D1SolidColorBrush *pBrush;

	D2D1_COLOR_F custom;
	D2D1_POINT_2F ptmouse;

	// DirectWrite Interfaces:
	IDWriteFactory* pDWFactory;
	IDWriteTextFormat* pTF;

	//* WIC InterFace
	IWICImagingFactory* pWICFactory;
	Paddle background = Paddle(0, 0, 1629, 1025, 2, 1, 2);

	ID2D1Bitmap* backGroundAll;
	ID2D1Bitmap* gameOverScreen[2];
	ID2D1Bitmap* titlePic[4];
	Paddle *fly[3] ;

	Paddle *racerBackRound[4];


	//TODO
	Paddle superPlayCooler = Paddle(0, 0, 1366, 512, 3, 2, 6);
	Paddle superPlayCarot = Paddle(0, 0, 1387, 447, 3, 2, 6);
	ID2D1Bitmap* particle[2];


	DWORD dwTime;

	//particle*
	D2D1_POINT_2F particlePos[2][NUM_PARTICLE];
	D2D1_SIZE_F particleVel[NUM_PARTICLE];
	float particleLife[NUM_PARTICLE];

	// Game Components:
	
	Paddle ball = Paddle(0,0,539,68,7,1,7);
	Paddle player = Paddle(240, 50, 450,110,3,1,3);
	Paddle enemy = Paddle(240,90,460, 150, 3,1,3);

	//chi collection
	Paddle LifeBoard = Paddle(0,0,468, 67, 7, 1, 7);

	// Variables used to calculate frames per second:
	DWORD dwFrames;
	DWORD dwCurrentTime;
	DWORD dwLastUpdateTime;
	DWORD dwElapsedTime;
	std::wstring szFPS;

	float LineWidth = 10;

	ID2D1LinearGradientBrush *m_pLinearGradientBrush;

	// Create an array of gradient stops to put in the gradient stop
	// collection that will be used in the gradient brush.
	ID2D1GradientStopCollection *pGradientStops;// = NULL;

	D2D1_GRADIENT_STOP gradientStops[2];


	ID2D1SolidColorBrush* pBrushGrayBlack;

	int fPosX = 0;

	D2D1_RECT_F optionBox[3];

	CXBOXController Controller1 = CXBOXController(1);
	CXBOXController Controller2 = CXBOXController(2);

	bool gameOver;
	bool titleScreen;

	int deltaTimeNext;

	bool cpuControl_1p = false;

	bool cpuControl_2p = false;

	float GameOverCounter = 0;
	bool gameOverSound;
	int howToPlay ;
		
	PressDetect syatemSpace;
public:
	
	void SetCPU(bool _1p, bool _2p);

	bool GetCpu_1p()const
	{
		return cpuControl_1p;
	}

	bool GetCpu_2p()const
	{
		return cpuControl_2p;
	}

	void SetCpu_1p(bool _cpu)
	{
		cpuControl_1p = _cpu;
	}
	void SetCpu_2p(bool _cpu)
	{
		cpuControl_2p = _cpu;
	}

	void renderTitle(TCHAR szTitle[100], D2D1_COLOR_F& _gameBoardColor);
	void ballUpdate();

	int getDeltaTimeNext()const
	{
		return deltaTimeNext;
	}

	void setDeltaTimeNext(int _deltaTimeNext)
	{
		deltaTimeNext = _deltaTimeNext;
	}
	BOOL bRunning;		// Is the game running?
	HANDLE hGameThread; // Handle to the GameMain thread.
	BYTE keys[256];		// Current state of the keyboard.

	Game(void);
	virtual ~Game(void);

	void Startup(void);
	void Shutdown(void);
	GAMESTATE Input(void);
	GAMESTATE title(void);
	GAMESTATE Simulate(void);
	GAMESTATE Render(bool pause, TCHAR szTitle[32],TCHAR szPause[32], D2D1_COLOR_F, D2D1_COLOR_F, D2D1_COLOR_F);

	void InputPlayer();

	void InputEnemy();

	void SetHwnd(HWND _hWnd) {hWnd = _hWnd;}

	HRESULT LoadBitmapFromFile(LPCTSTR strFileName, ID2D1Bitmap** ppBitmap);

	//initial the pramater
	void initialPaddle();

	//redner the paddle
	void renderPaddle();

	void ballAndWallSim();
	void chiBulletSim();
	void addFrame(Paddle _paddle);

	void resetFrame(Paddle _paddle);


	void limitUpInput(Paddle& _paddle);

	void limitDownInput(Paddle& _paddle);

	void renderBackGround();
	void drawLife(Paddle& _paddle, D2D1_COLOR_F _gameBoardColor, D2D1_COLOR_F O_FILL_Color_Start, D2D1_COLOR_F O_FILL_Color_Stop);
	void renderEnviroment(D2D1_COLOR_F, D2D1_COLOR_F, D2D1_COLOR_F);

	void SetLineWidth(float _LineWidth){ LineWidth = _LineWidth; }
	float GetLineWidth()const
	{
		return LineWidth;
	}


	void loadSpriteCondtion(Paddle& _paddle);
	void ballAndRaddleSim();

	void renderAction(Paddle& _paddle);


	void changeStateDetect(Paddle& _paddle);

	void changeStatePre(Paddle& _paddle);

	void drawBullet();

	void bigChiSim(int _number, Paddle& _paddle, int countShoot, float speedDirection);
	void smallChiSim(int _number, Paddle& _paddle, int countShoot, float speedDirection);

	void drawBigBullet(int _number, Paddle& _paddle);
	void drawSmallBullet(int _number, Paddle& _paddle);

	void InputBigAtk(Paddle& _paddle, bool bPrevSpace, bool bCurrSpace);
	void InputSmallAtk(Paddle& _paddle, bool bPrevSpace, bool bCurrSpace);

	void InputBigButton(Paddle& _paddle, CXBOXController& _CXBOXController, char _press);
	void InputSmallButton(Paddle& _paddle, CXBOXController& _CXBOXController, char _press);

	bool smallAtkExe(Paddle& _paddle);
	bool bigAtkExe(Paddle& _paddle);
	bool superExe(Paddle& _paddle);

	void upExe(Paddle& _paddle);
	void downExe(Paddle& _paddle);

	void AIAction(Paddle& _paddle);

	void PaddleRecounter(Paddle& _paddle);
	void ballHurtPaddle(Paddle& _paddle);

	void drawChi(Paddle& _paddle);

	void chiAndRaddleSim(Paddle& _paddle);

	void SuperButton(Paddle& _paddle, CXBOXController& _CXBOXController, char _press);

	GAMESTATE gameReplay();
	

	void renderParticle(D2D1_RECT_F& temp,int _type);

	void renderReplay(Paddle _paddle);

	void orginalState(Paddle& _paddle);

	//voice function
	int GetFreeVoice();

	void audioSubmit(AudioData& _sound);	


	void systemInput();

	void renderPause(TCHAR szPause[100], D2D1_COLOR_F);

	void initialMusic();


	void InputMoveUp(Paddle& _paddle, CXBOXController& _CXBOXController, char _press);
	void InputMoveDown(Paddle& _paddle, CXBOXController& _CXBOXController, char _press);

};