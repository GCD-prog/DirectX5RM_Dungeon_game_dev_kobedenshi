#include <windows.h>

#include <ddraw.h>
#include <d3drm.h>
#include <d3d.h>
//#include <d3drmwin.h>
#include <mmsystem.h>	//マルチメディア系の制御（timeGetTime()で必要）

#include <math.h>

#include "nav.h"		// プレーヤーの移動のためのルーチン

#include "sound.h"		// サウンド機能(MIDI)
#include "dsound.h"		// DirectSoundを使う関数(効果音)

#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "d3drm.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")

#pragma comment(lib, "dsound.lib")

extern LPDIRECTDRAW lpDirectDraw;
extern LPDIRECTDRAWCLIPPER lpDDClipper;
extern LPDIRECTDRAWSURFACE lpPrimary;
extern LPDIRECTDRAWSURFACE lpBackbuffer;
extern LPDIRECTDRAWSURFACE lpZbuffer;

extern LPDIRECT3D lpDirect3D;
extern LPDIRECT3DDEVICE lpD3DDevice;

extern LPDIRECT3DRM lpDirect3DRM;
extern LPDIRECT3DRMDEVICE lpD3DRMDevice;
extern LPDIRECT3DRMVIEWPORT lpD3DRMView;
extern LPDIRECT3DRMFRAME lpD3DRMScene;
extern LPDIRECT3DRMFRAME lpD3DRMCamera;


// Desc: BPP(bits per pixel)をDirectDrawのビット深度フラグに変換
static DWORD BPPToDDBD(int bpp);

// ドライバ検索処理関数
static HRESULT WINAPI enumDeviceFunc(LPGUID lpGuid, LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC lpHWDesc, LPD3DDEVICEDESC lpHELDesc, LPVOID lpContext);

// DirectDraw デバイスの列挙と選定
static BOOL EnumDrivers();

// メッシュなどを作りシーンを作成します
BOOL BuildScene(void);

void ReleaseAll(void);
