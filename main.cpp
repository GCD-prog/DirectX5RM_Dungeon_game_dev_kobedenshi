
/////////////////////////////////////////////////////////////////////////////////////////////////
// 一部引用部分(nav.h, nav.cpp, Scene.cpp, sound.h/cpp dsound.h/cpp ) 参考プログラム書籍のコード
// DirectX5ゲームプログラミング入門  出版社: インプレス (1998/3/1)
// ジェーソン コルブ (著), Jason Kolb (原名), アクロバイト (翻訳)
/////////////////////////////////////////////////////////////////////////////////////////////////

#include "dx.h"
#include <stdio.h>

#define INITGUID

#define WAIT 25

LPDIRECTDRAWSURFACE lpScreen;

/*
LPDIRECTDRAW lpDirectDraw = NULL;
LPDIRECTDRAWCLIPPER lpDDClipper = NULL;
LPDIRECTDRAWSURFACE lpPrimary = NULL;
LPDIRECTDRAWSURFACE lpBackbuffer = NULL;
LPDIRECTDRAWSURFACE lpZbuffer = NULL;

//Direct3D 関連
LPDIRECT3D lpDirect3D = NULL;
LPDIRECT3DDEVICE lpD3DDevice = NULL;

//Direct3D RM 関連
LPDIRECT3DRM lpDirect3DRM = NULL;
LPDIRECT3DRMDEVICE lpD3DRMDevice = NULL;
LPDIRECT3DRMVIEWPORT lpD3DRMView = NULL;
LPDIRECT3DRMFRAME lpD3DRMScene = NULL;
LPDIRECT3DRMFRAME lpD3DRMCamera = NULL;
*/

char szDevice[128], szDDDeviceName[128] = "default";

int nScore = 0;			// 得点
int nPossible = 10;		// 可能な最高点

void FrameCnt(void)
{
	     static int cnt;
		 static DWORD Nowtime,Prevtime;
		 static char text0[10];
		 static char Data[256];

		 HDC hdc;
		 cnt++;
		 Nowtime = timeGetTime();
		 if((Nowtime - Prevtime) >= 1000){
			 Prevtime = Nowtime;
			 wsprintf(text0,"%d fps", cnt);
			 cnt = 0;
		 }
	
		 sprintf(Data, "%s , [%s]", szDevice, szDDDeviceName);

		 // 開発時のみ表示する
		 lpBackbuffer->GetDC(&hdc);
		 TextOut(hdc, 0, 0, text0, strlen(text0));
		 TextOut(hdc, 200, 100, Data, strlen(Data));
		 lpBackbuffer->ReleaseDC(hdc);
}

void KeyControl(void)
{
	//処理を書き込む

	//このプログラムはメモ書き、ここの位置では使わない。
	// プレーヤーの位置に合わせてカメラの位置を更新します
	//lpD3DRMCamera->AddRotation(D3DRMCOMBINE_REPLACE, 0.0f, 1.0f, 0.0f,fGetAngle());
	//lpD3DRMCamera->SetPosition(lpD3DRMScene, fGetXPos(), 1.5f, fGetZPos());
	//この座標位置の変数の値がとれていないから移動しないのでは？

	//左旋回
	if( (GetAsyncKeyState(VK_LEFT)&0x8000) || (GetAsyncKeyState('Q')&0x8000) ){
			TurnLeft();
	}

	//右旋回
	if( (GetAsyncKeyState(VK_RIGHT)&0x8000) || (GetAsyncKeyState('E')&0x8000) ){
			TurnRight();
	}

	//前進
	if(GetAsyncKeyState('W')&0x8000){
		if(GetKeyState(VK_SHIFT) & 0x8000){		// Shiftキーで高速化
			RunForward();
		}else{
			StepForward();
		}
	}

	//後退
	if(GetAsyncKeyState('S')&0x8000){
		if(GetKeyState(VK_SHIFT) & 0x8000){		// Shiftキーで高速化
			RunBackward();
		}else{
			StepBackward();
		}
	}

	//左
	if(GetAsyncKeyState('A')&0x8000){
		if(GetKeyState(VK_SHIFT) & 0x8000){		// Shiftキーで高速化
			SwingLeft();
		}else{
			TurnLeft();
		}
	}

	//右
	if(GetAsyncKeyState('D')&0x8000){
		if(GetKeyState(VK_SHIFT) & 0x8000){		// Shiftキーで高速化
			SwingRight();
		}else{
			TurnRight();
		}
	}

	// プレーヤーの位置に合わせてカメラの位置を更新します ここに配置 lpD3DRMCamare
	lpD3DRMCamera->AddRotation(D3DRMCOMBINE_REPLACE, 0.0f, 1.0f, 0.0f,fGetAngle());
	lpD3DRMCamera->SetPosition(lpD3DRMScene, fGetXPos(), 1.5f, fGetZPos());

}

BOOL RenderFrame(void)
{

	char lpt[20];  // midi リピート
	mciSendString("status bgm mode", lpt, sizeof(lpt), NULL);
	if(strcmp(lpt, "stopped") == 0){
		mciSendString("play bgm from 0", NULL, 0, NULL);
	}

	if ( lpPrimary->IsLost() == DDERR_SURFACELOST )		lpPrimary->Restore();

		RECT Scrrc={0, 0, 640, 480};   //画面のサイズ

		//秒間６０フレームを越えないようにする
		static DWORD nowTime, prevTime;
		nowTime = timeGetTime();
		if( (nowTime - prevTime) < 1000 / 60 ) return 0;
		prevTime = nowTime;

		//キー入力
		KeyControl();

		//Direct3DRM レンダリング処理
		lpD3DRMScene->Move(D3DVAL(1.0)); 
		lpD3DRMView->Clear();

		// 現在のシーンをバックバッファで再描画します。これは、
		// 現在のサーフェイスがGDIサーフェイスでない場合の処理です
		// この場合、こうしてシーンを再描画すると、スコアを表示する前に
		// シーンが1フレームだけ戻るのを防げます
		lpD3DRMView->Render(lpD3DRMScene);  // シーンのレンダリング
		lpD3DRMDevice->Update();              // デバイス（ウィンドウ）の更新

		//FPS値計測
		FrameCnt();

		// Flip関数は使わない 画面がちらつく
		lpPrimary->Flip(NULL, DDFLIP_WAIT);

		return TRUE;
}

//-------------------------------------------
// ドライバ検索処理関数
//-------------------------------------------
GUID* D3D_GuidSearch(HWND hwnd)
{
	HRESULT d3dret;   //ダイレクト３Ｄ系関数の戻り値を格納する変数
	GUID*   Guid;
	LPDIRECT3D          lpD3D;
	LPDIRECTDRAW        lpDD;
	D3DFINDDEVICESEARCH S_DATA;
	D3DFINDDEVICERESULT R_DATA;
	char str[100];

	//GUID取得の準備
	memset(&S_DATA, 0, sizeof S_DATA);
	S_DATA.dwSize = sizeof S_DATA;
	S_DATA.dwFlags = D3DFDS_COLORMODEL;
	S_DATA.dcmColorModel = D3DCOLOR_RGB;
	memset(&R_DATA, 0, sizeof R_DATA);
	R_DATA.dwSize = sizeof R_DATA;

	//DIRECTDRAWの生成
	d3dret = DirectDrawCreate(NULL, &lpDD, NULL);
	if (d3dret != DD_OK) {
		MessageBox( hwnd, "ダイレクトドローオブジェクトの生成に失敗しました", "初期化", MB_OK);
		lpDD->Release();
		return NULL;
	}

	//DIRECTD3Dの生成
	d3dret = lpDD->QueryInterface(IID_IDirect3D, (void**)&lpD3D);
	if (d3dret != D3D_OK) {
		MessageBox( hwnd, "ダイレクト３Ｄオブジェクトの生成に失敗しました", "初期化", MB_OK);
		lpDD->Release();
		lpD3D->Release();
		return NULL;
	}
	//デバイスの列挙
	d3dret = lpD3D->FindDevice(&S_DATA, &R_DATA);
	if (d3dret != D3D_OK) {
		MessageBox( hwnd, "デバイスの列挙に失敗しました", "初期化", MB_OK);
		lpDD->Release();
		lpD3D->Release();
		return NULL;
	}

	//ガイドの取得
	Guid = &R_DATA.guid;
	//不要になったオブジェクトのリリース
	lpDD->Release();
	lpD3D->Release();
	//OutputDebugString(str);
	wsprintf(str, "%x", *Guid);
	return (Guid);
}

/*-------------------------------------------
// DirectDraw デバイスの列挙と選定
--------------------------------------------*/
BOOL CALLBACK DDEnumCallback(GUID FAR* lpGUID, LPSTR lpDriverDesc, LPSTR lpDriverName, LPVOID lpContext)
{
	LPDIRECTDRAW lpDD;
	DDCAPS DriverCaps, HELCaps;

	// DirectDraw オブジェクトを試験的に生成する
	if(DirectDrawCreate(lpGUID, &lpDD, NULL) != DD_OK) {
		*(LPDIRECTDRAW*)lpContext = NULL;
		return DDENUMRET_OK;
	}

	// DirectDrawの能力を取得
	ZeroMemory(&DriverCaps, sizeof(DDCAPS));
	DriverCaps.dwSize = sizeof(DDCAPS);
	ZeroMemory(&HELCaps, sizeof(DDCAPS));
	HELCaps.dwSize = sizeof(DDCAPS);

	if(lpDD->GetCaps(&DriverCaps, &HELCaps) == DD_OK) {
	// ハードウェア3D支援が期待できる場合で．
	// なおかつテクスチャが使用できる場合それを使う
		if ((DriverCaps.dwCaps & DDCAPS_3D) && (DriverCaps.ddsCaps.dwCaps & DDSCAPS_TEXTURE)) {
			*(LPDIRECTDRAW*)lpContext = lpDD;
			lstrcpy(szDDDeviceName, lpDriverDesc);
			return DDENUMRET_CANCEL;
		}
	}

	// 他のドライバを試す
	*(LPDIRECTDRAW*)lpContext = NULL;
	lpDD->Release();

	return DDENUMRET_OK;
}

LRESULT APIENTRY WndFunc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch(msg){

	case WM_CREATE:

		break;

	case WM_KEYDOWN:

		// Escキーでプログラムを終了します
		if(wParam == VK_ESCAPE){

			ShutdownDirectSound();
			StopBackgroundMusic();

			//画面モードを元に戻す
			lpDirectDraw->SetCooperativeLevel(hwnd, DDSCL_NORMAL);
			lpDirectDraw->RestoreDisplayMode();

			lpScreen->Release();

			ReleaseAll(); //各オブジェクトをReleaseAll()で解放する

			PostQuitMessage(0);

		}

		break;

	case WM_DESTROY:

			ShutdownDirectSound();
			StopBackgroundMusic();

			lpScreen->Release();

			ReleaseAll(); //各オブジェクトをReleaseAll()で解放する

			PostQuitMessage(0);

		break;

	// BGMが停止すると、そのことが通知されます
	// "play"コマンドを再実行します
	case MM_MCINOTIFY:
			if(wParam == MCI_NOTIFY_SUCCESSFUL)
				ReplayBackgroundMusic(hwnd);

		break;

	default:
		return (DefWindowProc(hwnd, msg, wParam, lParam));
	}
	return 0;
}

void LoadBMP(LPDIRECTDRAWSURFACE lpSurface, char *fname)
{
	     HBITMAP hBmp = NULL;
		 BITMAP bm;
		 HDC hdc, hMemdc;

		 hBmp = (HBITMAP)LoadImage(GetModuleHandle(NULL), fname, IMAGE_BITMAP, 0, 0,
			      LR_CREATEDIBSECTION | LR_LOADFROMFILE);
		 GetObject(hBmp, sizeof(bm), &bm);

		 hMemdc = CreateCompatibleDC(NULL);
		 SelectObject(hMemdc, hBmp);

		 lpSurface->GetDC(&hdc);
		 BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hMemdc, 0, 0, SRCCOPY);
		 lpSurface->ReleaseDC(hdc);

		 DeleteDC(hMemdc);
		 DeleteObject(hBmp);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpszCmdParam, int nCmdShow)
{
		lpDirectDraw = NULL;
		lpDDClipper = NULL;
		lpPrimary = NULL;
		lpBackbuffer = NULL;
		lpZbuffer = NULL;
		lpDirect3DRM = NULL;
		lpD3DRMDevice = NULL;
		lpD3DRMView = NULL;
		lpD3DRMScene = NULL;

		MSG msg;
		HWND hwnd;

		DDSURFACEDESC Dds;
		DDSCAPS Ddscaps;

		WNDCLASS wc;
		char szAppName[] = "Generic Game SDK Window";
		
		wc.style = CS_DBLCLKS;
		wc.lpfnWndProc = WndFunc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInst;
		wc.hIcon = LoadIcon(NULL,IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = szAppName;

		RegisterClass(&wc);

		hwnd=CreateWindowEx(
							WS_EX_TOPMOST,
							szAppName,
							"Direct X",
							WS_VISIBLE | WS_POPUP,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							640, 480,
							NULL, NULL, hInst,
							NULL);

		if(!hwnd) return FALSE;

		ShowWindow(hwnd, nCmdShow);
		UpdateWindow(hwnd);
		SetFocus(hwnd);

		ShowCursor(FALSE); //カーソルを隠す

		//Direct3DRMの構築
		Direct3DRMCreate(&lpDirect3DRM);

		//DirectDrawClipperの構築
		DirectDrawCreateClipper(0, &lpDDClipper, NULL);

		// DirectDrawドライバを列挙する
		DirectDrawEnumerate(DDEnumCallback, &lpDirectDraw);

		// 列挙によってDirectDrawドライバを決める
		// 決定しなかった場合、現在アクティブなドライバを使う
		if(!lpDirectDraw){
			lstrcpy(szDDDeviceName, "Active Driver");
			DirectDrawCreate(NULL, &lpDirectDraw, NULL);
		}

		lpDirectDraw->SetCooperativeLevel(hwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT);

		//ディスプレイモード変更
		lpDirectDraw->SetDisplayMode(640, 480, 16);

		//基本サーフェスとフロントバッファの生成 (１つを作成)
		ZeroMemory(&Dds, sizeof(DDSURFACEDESC));
		Dds.dwSize = sizeof(DDSURFACEDESC);
		Dds.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		Dds.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_VIDEOMEMORY | DDSCAPS_3DDEVICE;
		Dds.dwBackBufferCount = 1;

		lpDirectDraw->CreateSurface(&Dds, &lpPrimary, NULL);

		//バックバッファのポインタ取得
		Ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
		lpPrimary->GetAttachedSurface(&Ddscaps, &lpBackbuffer);

		// Z-Buffer作成
		//基本サーフェスとバッファ１つを作成
		ZeroMemory(&Dds, sizeof(DDSURFACEDESC));
		Dds.dwSize = sizeof(DDSURFACEDESC);
		Dds.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_ZBUFFERBITDEPTH;
		Dds.dwHeight = 640;
		Dds.dwWidth = 480;
		Dds.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY;
		Dds.dwZBufferBitDepth = 16;

		lpDirectDraw->CreateSurface(&Dds, &lpZbuffer, NULL);

		lpBackbuffer->AddAttachedSurface(lpZbuffer);

/*
		//DirectDrawでのアクセスもできるように、Clipperをつける
		struct _MYRGNDATA {
			RGNDATAHEADER rdh;
			RECT rc;
		}rgndata;
	
		rgndata.rdh.dwSize = sizeof(RGNDATAHEADER);
		rgndata.rdh.iType = RDH_RECTANGLES;
		rgndata.rdh.nCount = 1;
		rgndata.rdh.nRgnSize = sizeof(RECT)*1;
		rgndata.rdh.rcBound.left = 0;
		rgndata.rdh.rcBound.right = 640;
		rgndata.rdh.rcBound.top = 0;
		rgndata.rdh.rcBound.bottom = 480;

		rgndata.rc.top = 0;
		rgndata.rc.bottom = 480;
		rgndata.rc.left = 0;
		rgndata.rc.right = 640;

		lpDirectDraw->CreateClipper(0, &lpDDClipper, NULL);
		lpDDClipper->SetClipList((LPRGNDATA)&rgndata, NULL);
		lpBackbuffer->SetClipper(lpDDClipper);
*/

		DDCOLORKEY          ddck;

		//背景サーフェスを作成
		Dds.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		Dds.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		Dds.dwWidth = 640;
		Dds.dwHeight = 480;
		lpDirectDraw->CreateSurface(&Dds, &lpScreen, NULL);

		//カラーキーの指定 白:RGB(255, 255, 255)　黒:RGB(0, 0, 0)
		ddck.dwColorSpaceLowValue=RGB(0, 0, 0);
		ddck.dwColorSpaceHighValue=RGB(0, 0, 0);
		lpScreen->SetColorKey(DDCKEY_SRCBLT, &ddck);

		//各サーフェスに画像を読み込む
		LoadBMP(lpScreen, "datafile\\back.BMP");  //背景

		DirectDrawCreateClipper(0, &lpDDClipper, NULL);

//Direct3DRMの初期化 ここから

//デバイスの生成 ＨＥＬ実行 RAMP 低画質 速度遅い
//		Direct3DRMCreate(&lpDirect3DRM);
//		lpDirect3DRM->CreateDeviceFromSurface(NULL, lpDirectDraw, lpBackbuffer, &lpD3DRMDevice);
//

//デバイスの生成 ＨＡＬで実行する

		HRESULT ddret;   //ダイレクト３Ｄ系関数の戻り値を格納する変数
		GUID*   Guid;

		Direct3DRMCreate(&lpDirect3DRM);

		Guid = D3D_GuidSearch(hwnd);
		// HAL
		ddret = lpDirect3DRM->CreateDeviceFromSurface(Guid, (IDirectDraw*)lpDirectDraw, lpBackbuffer, &lpD3DRMDevice);
			strcpy(szDevice,"D3D HAL");
		if (ddret != D3DRM_OK) {
			MessageBox( hwnd, "デバイスの生成に失敗、ＨＡＬでの実行は不可能です", "", MB_OK);
			//HALでの実行が不可能な時、HELでの実行を行う
			ddret = lpDirect3DRM->CreateDeviceFromSurface(Guid, (IDirectDraw*)lpDirectDraw, lpBackbuffer, &lpD3DRMDevice);
			if (ddret != D3DRM_OK) {
				strcpy(szDevice,"HEL");
				MessageBox( hwnd, "ＨＥＬでの、デバイスの生成に失敗、Direct3Dの使用は不可能です", "", MB_OK);
			}

			if(ddret != D3DRM_OK){
				//MMX
				ddret = lpDirect3DRM->CreateDeviceFromSurface(Guid, (IDirectDraw*)lpDirectDraw, lpBackbuffer, &lpD3DRMDevice);
				strcpy(szDevice,"D3D MMX Emulation");
			}

			if(ddret != D3DRM_OK){
				//RGB
				ddret = lpDirect3DRM->CreateDeviceFromSurface(Guid, (IDirectDraw*)lpDirectDraw, lpBackbuffer, &lpD3DRMDevice);
				strcpy(szDevice,"D3D RGB Emulation");
			}
		}
//

		lpD3DRMDevice->SetQuality(D3DRMLIGHT_ON | D3DRMFILL_SOLID | D3DRMSHADE_GOURAUD);

		lpDirect3DRM->CreateFrame(NULL, &lpD3DRMScene);

		//カメラを作成
								// 親フレーム ,	子フレーム
		lpDirect3DRM->CreateFrame(lpD3DRMScene, &lpD3DRMCamera);

		lpD3DRMCamera->SetPosition(lpD3DRMScene, D3DVAL(0.0), D3DVAL(0.0), D3DVAL(0.0));

		//デバイスとカメラからDirect3DRMViewPortを作成
		lpDirect3DRM->CreateViewport(lpD3DRMDevice, lpD3DRMCamera, 0, 0, 640, 480, &lpD3DRMView);

		// 背景の奥行を、プレーヤーがダンジョン内で見ることができる最大距離に設定します。
		// こうすると、ダンジョン全体の視野を邪魔することなく、遠くのクリッピングプレーンをできるだけ近くに設定できます
		lpD3DRMView->SetBack(D3DVAL(70.f));

		// フロントクリッピングプレーンを非常に近くに設定します
		lpD3DRMView->SetFront(0.5f);

		// ナビゲーションシステムを初期化します
		SetupNavigator();		

		// シーンを作成します
		BuildScene();

		// シーンが描画された後、BGMを開始します
		StartBackgroundMusic(hwnd);

		while(1){

			if(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
				{
					if(!GetMessage(&msg, NULL, 0, 0))
						return msg.wParam;
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}else{

							RenderFrame();

					}
		}
		return msg.wParam;

}
