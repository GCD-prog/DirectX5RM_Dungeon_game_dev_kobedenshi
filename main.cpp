
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

#define MAX_DRIVERS 5                   // 検出するドライバの最大数
static GUID DriverGUID[MAX_DRIVERS];     // ドライバのGUIDを格納する配列
static char DriverName[MAX_DRIVERS][50]; // ドライバ名を格納する配列
static int  NumDrivers = 0;             // 検出されたドライバの数 (初期値0)
static int  CurrDriver = -1;            // 現在選択されているドライバのインデックス (初期値-1)
static int  BPP = 16;                   // 画面の色深度 (16bitに設定)

int nScore = 0;			// 得点
int nPossible = 10;		// 可能な最高点

void FrameCnt(void)
{
		static int cnt;
		static DWORD Nowtime,Prevtime;
		static char text0[10];		// FPSカウント数
		static char text1[10];		// nScore 得点
		static char text2[10];		// nPossible 可能な最高点
		static char text_d3d_device[150] = "";
		static char text_dd_device[150] = "";

		HDC hdc;
		cnt++;
		Nowtime = timeGetTime();
		 if((Nowtime - Prevtime) >= 1000){
			Prevtime = Nowtime;
			wsprintf(text0,"%d fps", cnt);
			cnt = 0;
		}

		wsprintf(text1,"%d 得点", nScore);
		wsprintf(text2,"%d 最高点", nPossible);

		// デバイス名表示用の文字列を作成
		sprintf(text_d3d_device, "D3D Dev: %s", szDevice);
		sprintf(text_dd_device, "DD Dev : [%s]", szDDDeviceName);

		// 開発時のみ表示する
		lpBackbuffer->GetDC(&hdc);
		TextOut(hdc, 0, 0, text0, strlen(text0));		// FPSカウント数
		TextOut(hdc, 50, 0, text1, strlen(text1));		// nScore 得点
		TextOut(hdc, 100, 0, text2, strlen(text2));	// nPossible 可能な最高点

		TextOut(hdc, 0, 80, text_d3d_device, strlen(text_d3d_device));   // Direct3Dデバイス名
		TextOut(hdc, 0, 100, text_dd_device, strlen(text_dd_device));    // DirectDrawデバイス名

		lpBackbuffer->ReleaseDC(hdc);
}

void KeyControl(void)
{
	// 処理を書き込む

	// このプログラムはメモ書き、ここの位置では使わない。
	// プレーヤーの位置に合わせてカメラの位置を更新します
	// lpD3DRMCamera->AddRotation(D3DRMCOMBINE_REPLACE, 0.0f, 1.0f, 0.0f,fGetAngle());
	// lpD3DRMCamera->SetPosition(lpD3DRMScene, fGetXPos(), 1.5f, fGetZPos());
	// この座標位置の変数の値がとれていないから移動しないのでは？

	// 左旋回
	if( (GetAsyncKeyState(VK_LEFT)&0x8000) || (GetAsyncKeyState('Q')&0x8000) ){
			TurnLeft();
	}

	// 右旋回
	if( (GetAsyncKeyState(VK_RIGHT)&0x8000) || (GetAsyncKeyState('E')&0x8000) ){
			TurnRight();
	}

	// 前進
	if(GetAsyncKeyState('W')&0x8000){
		if(GetKeyState(VK_SHIFT) & 0x8000){		// Shiftキーで高速化
			RunForward();
		}else{
			StepForward();
		}
	}

	// 後退
	if(GetAsyncKeyState('S')&0x8000){
		if(GetKeyState(VK_SHIFT) & 0x8000){		// Shiftキーで高速化
			RunBackward();
		}else{
			StepBackward();
		}
	}

	// 左
	if(GetAsyncKeyState('A')&0x8000){
		if(GetKeyState(VK_SHIFT) & 0x8000){		// Shiftキーで高速化
			SwingLeft();
		}else{
			TurnLeft();
		}
	}

	// 右
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

	if ( lpPrimary->IsLost() == DDERR_SURFACELOST )		lpPrimary->Restore();

		RECT Scrrc={0, 0, 640, 480};   // 画面のサイズ

		// 秒間６０フレームを越えないようにする
		static DWORD nowTime, prevTime;
		nowTime = timeGetTime();
		if( (nowTime - prevTime) < 1000 / 60 ) return 0;
		prevTime = nowTime;

		// キー入力
		KeyControl();

		// Direct3DRM レンダリング処理
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

		// Flip関数
		lpPrimary->Flip(NULL, DDFLIP_WAIT);

		return TRUE;
}

//-------------------------------------------
// Name: BPPToDDBD()
// BPP(bits per pixel)をDirectDrawのビット深度フラグに変換
//-------------------------------------------
static DWORD BPPToDDBD(int bpp)
{

	switch(bpp) {
		case 1: return DDBD_1;
		case 2: return DDBD_2;
		case 4: return DDBD_4;
		case 8: return DDBD_8;
		case 16: return DDBD_16;
		case 24: return DDBD_24;
		case 32: return DDBD_32;
		default: return 0;
	}

}

//-------------------------------------------
// Name: enumDeviceFunc()
// Desc: Direct3Dデバイスを列挙するためのコールバック関数
//       ハードウェアRGB > ハードウェアMono > HEL RGB > HEL Mono の順で優先
//       (三項演算子を if/else に書き換え)
//-------------------------------------------
static HRESULT WINAPI enumDeviceFunc(LPGUID lpGuid, LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC lpHWDesc, LPD3DDEVICEDESC lpHELDesc, LPVOID lpContext)
{

	LPD3DDEVICEDESC lpDesc = NULL;
	bool isHardware = false;
	bool isMono = false;
	int* lpCurrentDriverIndex = (int*)lpContext; // 現在選択中のドライバインデックス

	// デバイス記述子の選択 (ハードウェア優先)
	if (lpHWDesc && lpHWDesc->dcmColorModel != 0) {
		lpDesc = lpHWDesc;
		isHardware = true;
	} else if (lpHELDesc && lpHELDesc->dcmColorModel != 0) {
		lpDesc = lpHELDesc;
		isHardware = false;
	} else {
		// 有効なデバイス記述子がない場合はスキップ
		return D3DENUMRET_OK;
	}

	// 指定した色深度(BPP)に対応しているかチェック
	if (!(lpDesc->dwDeviceRenderBitDepth & BPPToDDBD(BPP))) {
		return D3DENUMRET_OK;
	}

	// ドライバ情報を配列に記録
	if (NumDrivers < MAX_DRIVERS) {
		memcpy(&DriverGUID[NumDrivers], lpGuid, sizeof(GUID));
		strncpy(DriverName[NumDrivers], lpDeviceDescription, 49);
		DriverName[NumDrivers][49] = '\0'; // NULL終端保証

		// 優先順位に基づいて CurrDriver を更新
		isMono = (lpDesc->dcmColorModel & D3DCOLOR_MONO);
		int currentPriority = 0; // 0: HEL Mono, 1: HEL RGB, 2: HW Mono, 3: HW RGB

		// currentPriority 計算
		if (isHardware) {
			// ハードウェアの場合
			if (isMono) {
				currentPriority = 2; // HW Mono
			} else {
				currentPriority = 3; // HW RGB
			}
		} else {
			// ソフトウェア (HEL) の場合
			if (isMono) {
				currentPriority = 0; // HEL Mono
			} else {
				currentPriority = 1; // HEL RGB
			}
		}

		int selectedPriority = -1; // 選択済みドライバの優先度 (-1 は未選択)
		if (*lpCurrentDriverIndex != -1) {
			// 既に選択されているドライバの優先度を DriverName から推測
			bool selectedIsHardware = false;
			bool selectedIsMono = false;

			// "(HW)" または "Hardware" が含まれていればハードウェアとみなす
			if (strstr(DriverName[*lpCurrentDriverIndex], "(HW)") || strstr(DriverName[*lpCurrentDriverIndex], "Hardware")) {
				selectedIsHardware = true;
			}
			// "Mono" が含まれていれば Mono とみなす
			if (strstr(DriverName[*lpCurrentDriverIndex], "Mono")) {
				selectedIsMono = true;
			}

			// selectedPriority 計算
			if (selectedIsHardware) {
				if (selectedIsMono) {
					selectedPriority = 2; // HW Mono
				} else {
					selectedPriority = 3; // HW RGB
				}
			} else {
				if (selectedIsMono) {
					selectedPriority = 0; // HEL Mono
				} else {
					selectedPriority = 1; // HEL RGB
				}
			}
		}

		// より優先度の高いドライバが見つかったら更新
		if (currentPriority > selectedPriority) {
			*lpCurrentDriverIndex = NumDrivers;
		}

		NumDrivers++; // 見つかったドライバ数を増やす
		if (NumDrivers == MAX_DRIVERS) {
			return D3DENUMRET_CANCEL; // 配列が一杯になったら終了
		}
	}

	return D3DENUMRET_OK;

}

//-------------------------------------------
// Name: EnumDrivers()
// Desc: 使用可能なDirect3Dドライバを列挙し、最適なものを選択する
//-------------------------------------------
static BOOL EnumDrivers()
{

	LPDIRECTDRAW lpDD_enum = NULL; // 列挙用の一時的なDirectDrawオブジェクト
	LPDIRECT3D   lpD3D_enum = NULL; // 列挙用の一時的なDirect3Dオブジェクト
	HRESULT hr;

	// 一時的なDirectDrawオブジェクトを作成
	hr = DirectDrawCreate(NULL, &lpDD_enum, NULL);
	if (FAILED(hr)) {
		MessageBox(NULL, "ドライバ列挙用 DirectDraw 作成失敗", "初期化エラー", MB_OK);
		return FALSE;
	}

	// 一時的なDirect3Dインターフェースを取得
	hr = lpDD_enum->QueryInterface(IID_IDirect3D, (void**)&lpD3D_enum);
	if (FAILED(hr)) {
		MessageBox(NULL, "ドライバ列挙用 Direct3D 取得失敗", "初期化エラー", MB_OK);
		lpDD_enum->Release();
		return FALSE;
	}

	// ドライバを列挙して最適なものを選択 (結果はグローバル変数 CurrDriver に格納)
	NumDrivers = 0; // ドライバ数をリセット
	CurrDriver = -1; // 選択中ドライバをリセット
	hr = lpD3D_enum->EnumDevices(enumDeviceFunc, &CurrDriver);
	if (FAILED(hr)) {
		MessageBox(NULL, "Direct3D デバイス列挙失敗", "初期化エラー", MB_OK);
	}

	// 一時オブジェクトを解放
	lpD3D_enum->Release();
	lpDD_enum->Release();

	// ドライバが見つからなかったか、選択されなかった場合
	if (NumDrivers == 0 || CurrDriver == -1) {
		MessageBox(NULL, "使用可能なDirect3Dドライバが見つかりません。", "初期化エラー", MB_OK);
		return FALSE;
	}

	// デバッグ出力に使用するドライバ情報を表示
	char log[100];
	sprintf(log, "選択されたドライバ: %d - %s\n", CurrDriver, DriverName[CurrDriver]);
	OutputDebugString(log);

	return TRUE;

}

LRESULT APIENTRY WndFunc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch(msg){

	case WM_CREATE:

		break;

	case WM_KEYDOWN:

		// Escキーでプログラムを終了します
		if(wParam == VK_ESCAPE){

			// dsound.cpp
			ShutdownDirectSound();
			// sound.cpp
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

			// dsound.cpp
			ShutdownDirectSound();

			// sound.cpp midi_file
			StopBackgroundMusic();

			lpScreen->Release();

			ReleaseAll(); //各オブジェクトをReleaseAll()で解放する

			PostQuitMessage(0);

		break;

	// sound.cpp midi_file
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


		// --- DirectDrawオブジェクトの作成 (画面モード設定等のために先に作成) ---
		// デフォルトのDirectDrawドライバを使用 (NULL を指定)
		if(FAILED(DirectDrawCreate(NULL, &lpDirectDraw, NULL))) {
			MessageBox(hwnd, "DirectDrawCreate Fail", "Error", MB_OK);
			ReleaseAll(); return FALSE;
		}
		// とりあえずDirectDrawドライバ名をセット (EnumDriversの結果とは別)
		lstrcpy(szDDDeviceName, "プライマリ ディスプレイ ドライバ");

		// --- 強調レベルと画面モード設定 ---
		if (FAILED(lpDirectDraw->SetCooperativeLevel(hwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT))) {
			MessageBox(hwnd,"SetCooperativeLevel Fail","Error",MB_OK); ReleaseAll(); return FALSE;
		}
		// ディスプレイモード変更 (BPP変数を使用)
		if (FAILED(lpDirectDraw->SetDisplayMode(640, 480, BPP))) {
			char err[100]; sprintf(err, "%dビット ディスプレイモード設定失敗", BPP);
			MessageBox(hwnd, err, "初期化エラー", MB_OK); ReleaseAll(); return FALSE;
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

		// --- Direct3Dデバイスの列挙と選択 ---
		if (!EnumDrivers()) {
			// エラーメッセージは EnumDrivers 内で表示される
			ReleaseAll();
			return FALSE;
		}

		// --- Direct3DRMデバイス作成 ---
		// EnumDriversで選択されたGUID (DriverGUID[CurrDriver]) を使用する
		HRESULT ddret = lpDirect3DRM->CreateDeviceFromSurface(
							&DriverGUID[CurrDriver], // 選択されたドライバのGUID
							lpDirectDraw,           // 作成済みのDirectDrawオブジェクト
							lpBackbuffer,
							&lpD3DRMDevice );

		if (FAILED(ddret)) {
			char err[200];
			sprintf(err, "選択されたドライバ(%s)でのRMデバイス作成に失敗しました。", DriverName[CurrDriver]);
			MessageBox(hwnd, err, "初期化エラー", MB_OK);
			ReleaseAll(); return FALSE;
		}
		// 選択されたDirect3Dデバイス名を szDevice にコピー
		strncpy(szDevice, DriverName[CurrDriver], sizeof(szDevice) - 1);
		szDevice[sizeof(szDevice) - 1] = '\0';


		// --- デバイス品質設定 ---
		lpD3DRMDevice->SetQuality(D3DRMFILL_SOLID | D3DRMSHADE_GOURAUD | D3DRMLIGHT_ON);

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

		// nav.cpp ナビゲーションシステムを初期化します (座標処理 これがないと移動できない)
		SetupNavigator();		

		// scene.cpp 本プログラムの3Dオブジェクトの呼び出し, シーンを作成します
		BuildScene();

		// sound.cpp シーンが描画された後、BGM(midi)を開始します
		StartBackgroundMusic(hwnd);

		while(1){

			if(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
				{
					if(!GetMessage(&msg, NULL, 0, 0))
						return msg.wParam;
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}else{

							// ゲームループ
							RenderFrame();

					}
		}
		return msg.wParam;

}
