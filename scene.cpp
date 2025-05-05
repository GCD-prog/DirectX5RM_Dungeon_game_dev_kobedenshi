///////////////////////////////////////////////////////////////////////////////
// Scene.cpp
// このファイルには、シーンを作成しレンダリングオプションをセットする機能が
// 含まれています
// Richard S. Wright Jr.
///////////////////////////////////////////////////////////////////////////////

// Win32の必要のない機能は省きます
#define WIN32_LEAN_AND_MEAN

#include <time.h>				// 乱数に使用

#include "dx.h"

LPDIRECTDRAW lpDirectDraw = NULL;
LPDIRECTDRAWCLIPPER lpDDClipper = NULL;
LPDIRECTDRAWSURFACE lpPrimary = NULL;
LPDIRECTDRAWSURFACE lpBackbuffer = NULL;
LPDIRECTDRAWSURFACE lpZbuffer = NULL;

// ddmain.cppで定義されているポインタ
LPDIRECT3DRM	lpDirect3DRM;		// Direct3D保持モードAPI
LPDIRECT3DRMFRAME lpD3DRMScene;	// Direct3Dの基準となるフレーム
LPDIRECT3DRMFRAME lpD3DRMCamera;	// Direct3Dのカメラフレーム
LPDIRECT3DRMDEVICE lpD3DRMDevice;	// Direct3Dデバイス

//Direct3D 関連
LPDIRECT3D lpDirect3D = NULL;
LPDIRECT3DDEVICE lpD3DDevice = NULL;
LPDIRECT3DRMVIEWPORT	lpD3DRMView = NULL;	// Direct3Dビューポート


// これらの関数は、このファイルの中でローカルに定義されていて、
// ダンジョンを構成するメッシュの作成に使われます
BOOL bGetFloorMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder);
BOOL bGetGoldFloorMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder);
BOOL bGetCeilingMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder);
BOOL bGetNSCorridorMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder);
BOOL bGetEWCorridorMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder);
BOOL bGetNWCornerMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder);
BOOL bGetSWCornerMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder);
BOOL bGetNECornerMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder);
BOOL bGetSECornerMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder);
BOOL bGetWWallMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder);
BOOL bGetEWallMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder);
BOOL bGetNWallMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder);
BOOL bGetSWallMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder);
BOOL bGetNDoorMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder);
BOOL bGetSDoorMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder);
BOOL bGetEDoorMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder);
BOOL bGetWDoorMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder);

// 金貨10枚の位置を示す配列
GoldLocations GoldFrames[10];

// ダンジョンのどこにどのメッシュを配置するかを表す配列 (nav.cppで定義)
extern int iDungeon[MAX_ROW][MAX_COL];

// メッシュを配置するマクロ:ここを修正するだけで、簡単に変換方法を変えられます
#define TRANSLATE() { if(FAILED(lpMeshBuilder->Translate( ((float)c * 6.0f) + 3.0f, 0.0f, (((17.0f - (float)r) * 6.0f) + 3.0f)))) return FALSE; }

// ローカルなテクスチャ
// 一度作成したら、使い終るまでとっておきます
LPDIRECT3DRMTEXTURE lpTxtFloor = NULL;			// テクスチャAPI
LPDIRECT3DRMTEXTURE lpTxtCeiling = NULL;			// テクスチャAPI
LPDIRECT3DRMTEXTURE lpTxtWood = NULL;			// テクスチャAPI
LPDIRECT3DRMTEXTURE lpTxtWall = NULL;			// テクスチャAPI
LPDIRECT3DRMTEXTURE lpTxtGold = NULL;			// テクスチャAPI
LPDIRECT3DRMFRAME lpDungeonFrame = NULL;			// ダンジョンを入れるフレーム

///////////////////////////////////////////////////////////////////////////////
// レンダリングオプションの設定
//  背景を黒に、照明なしのフラットシェーディング
///////////////////////////////////////////////////////////////////////////////
BOOL SetRenderingOptions(void)
{
	// シーンの背景を黒に設定します
//	lpD3DRMScene->SetSceneBackground(D3DRGB(0,0,0));

	// シェーディングモードを照明なしのフラットに設定します
//	lpD3DRMDevice->SetQuality(D3DRMRENDER_UNLITFLAT);	// 初期設定

	lpD3DRMDevice->SetQuality(D3DRMLIGHT_ON | D3DRMFILL_SOLID | D3DRMSHADE_GOURAUD);	// 改造

	//アンビエント光源を配置
	LPDIRECT3DRMLIGHT lpD3DRMLightAmbient;
	
	lpDirect3DRM->CreateLightRGB(D3DRMLIGHT_AMBIENT, D3DVAL(5.0), D3DVAL(5.0), D3DVAL(5.0), &lpD3DRMLightAmbient);
	lpD3DRMScene->AddLight(lpD3DRMLightAmbient);
	lpD3DRMLightAmbient->Release();

	LPDIRECT3DRMFRAME lpD3DRMLightFrame;
	lpDirect3DRM->CreateFrame(lpD3DRMScene, &lpD3DRMLightFrame);
	
	//ポイント光源を配置
	LPDIRECT3DRMLIGHT lpD3DRMLightPoint;

	lpDirect3DRM->CreateLightRGB(D3DRMLIGHT_POINT, D3DVAL(0.9), D3DVAL(0.9), D3DVAL(0.9), &lpD3DRMLightPoint);
	
	lpD3DRMLightFrame->SetPosition(lpD3DRMScene, D3DVAL(10.0), D3DVAL(0.0), D3DVAL(0.0));
	lpD3DRMLightFrame->AddLight(lpD3DRMLightPoint);

	lpD3DRMLightPoint->Release();

	lpD3DRMLightFrame->Release();

	// 成功を示すTRUEを返します
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// 金貨のあるフレームをダンジョンの子フレームとして追加します
///////////////////////////////////////////////////////////////////////////////
void AddGoldFrames(LPDIRECT3DRMFRAME& lpDungeonFrame)
	{
	 // メッシュビルダインターフェイス
	LPDIRECT3DRMMESHBUILDER lpMeshBuilder = NULL;
	int r,c;

	// 乱数ジェネレータを初期化します
	srand( (unsigned)time( NULL ) );

	// 10個のフレームを作成します
	for(int i = 0; i < 10; i++)
		{
		// フレームを作成します
		if(FAILED(lpDirect3DRM->CreateFrame(lpD3DRMScene, &(GoldFrames[i].lpGoldFrame))))	return;

		// 床の上に金貨があるメッシュを取得します
		bGetGoldFloorMesh(lpMeshBuilder);

		// 金貨のランダムな位置を決定します
		do {
			r = int( ((float)rand() / (float)RAND_MAX) * (float)MAX_ROW);
			c = int( ((float)rand() / (float)RAND_MAX) * (float)MAX_COL);
			}
		while (iDungeon[r][c] == 0);

		// ダンジョンの適切な位置に移動します
		lpMeshBuilder->Translate(((float)c * 6.0f) +3.0f, 0.0f,  (((17.0f - (float)r) * 6.0f) + 3.0f));

		// 座標とフレームポインタを格納し、発見フラグをFALSEに設定します
		GoldFrames[i].nRow = r;
		GoldFrames[i].nCol = c;
		GoldFrames[i].lpGoldFrame->AddVisual(lpMeshBuilder);
		GoldFrames[i].bFound = FALSE;

		// フレームをダンジョンに追加します
		lpDungeonFrame->AddChild(GoldFrames[i].lpGoldFrame);


		// メッシュビルダを解放します
		lpMeshBuilder->Release();
		}
}

///////////////////////////////////////////////////////////////////////////////
// 指定された位置に金貨があるかを調べます。もし金貨があれば得点となり、
// フレームから金貨のフレームをダンジョンから取り除きます
///////////////////////////////////////////////////////////////////////////////
BOOL bCheckForScore(int row, int column)
{
	// 10個の金貨をそれぞれループで各位置をチェックします
	for(int i = 0; i < 10; i++)
		{
		if(GoldFrames[i].nRow == row && GoldFrames[i].nCol == column && GoldFrames[i].bFound == FALSE)
			{
			GoldFrames[i].bFound = TRUE;

			// シーンから金貨を削除します
			lpDungeonFrame->DeleteChild(GoldFrames[i].lpGoldFrame);
			GoldFrames[i].lpGoldFrame->Release();

			return TRUE;
			}
		}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// シーンを作成します
///////////////////////////////////////////////////////////////////////////////
BOOL BuildScene(void)
{
	// メッシュビルダインターフェイス
	LPDIRECT3DRMMESHBUILDER lpMeshBuilder = NULL;

	// 最初にレンダリングオプションを設定します
	if(!SetRenderingOptions())
		return FALSE;

	// フレームを作成します
	if(FAILED(lpDirect3DRM->CreateFrame(lpD3DRMScene, &lpDungeonFrame)))
		return FALSE;

	// テクスチャを用意します
	if(FAILED(lpDirect3DRM->LoadTexture("datafile\\Floor.bmp",&lpTxtFloor)))
		return FALSE;

	if(FAILED(lpDirect3DRM->LoadTexture("datafile\\Ceiling.bmp",&lpTxtCeiling)))
		return FALSE;

	if(FAILED(lpDirect3DRM->LoadTexture("datafile\\Wood.bmp",&lpTxtWood)))
		return FALSE;

	if(FAILED(lpDirect3DRM->LoadTexture("datafile\\Walls.bmp",&lpTxtWall)))
		return FALSE;

	if(FAILED(lpDirect3DRM->LoadTexture("datafile\\gold.bmp",&lpTxtGold)))
		return FALSE;


	// ダンジョン全体の各々の位置をループします。各々の矩形にはどのメッシュを
	// そこに配置すべきかを示す値が割り当てられています。この矩形を並べたもの
	// がダンジョンになります
	for(int r = 0; r < 18; r++)		
		for(int c = 0; c < 22; c++)		
			{
			switch(iDungeon[r][c])
				{
				case 0: 
					break;	// 何もしません（ダンジョンの外側）
							// プレーヤーがここに移動することもできません
				
				case 1:		// 南北の回廊
					if(!bGetNSCorridorMesh(lpMeshBuilder))
						return FALSE;
					break;

				case 2:		// 東西の回廊
					if(!bGetEWCorridorMesh(lpMeshBuilder))
						return FALSE;
					break;

				case 3:		// 北西の角
					if(!bGetNWCornerMesh(lpMeshBuilder))
						return FALSE;
					break;

				case 4:		// 南西の角
					if(!bGetSWCornerMesh(lpMeshBuilder))
						return FALSE;
					break;

				case 5:		// 北東の角
					if(!bGetNECornerMesh(lpMeshBuilder))
						return FALSE;
					break;

				case 6:		// 南東の角
					if(!bGetSECornerMesh(lpMeshBuilder))
						return FALSE;
					break;

				case 7:	// 特殊な場合 (床と天井だけ) で、十字路と部屋の
						// 中央にしかありません
					if(!bGetFloorMesh(lpMeshBuilder))
						return FALSE;

					// 頂点を適切な位置に移動します
					TRANSLATE();

					// メッシュをフレームに追加します
					if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
						return FALSE;

					// このメッシュビルダを解放し、新しいビルダを取得させます
					lpMeshBuilder->Release();
					if(!bGetCeilingMesh(lpMeshBuilder))
						return FALSE;

					// 頂点を適切な位置に移動します
					TRANSLATE();

					// メッシュをフレームに追加します
					if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
						return FALSE;
					
					// メッシュを解放しNULLをセットすることで、この床と天井が別の
					// 場所に配置されないようにします。
					lpMeshBuilder->Release();
					lpMeshBuilder = NULL;
					break;

				case 8:		// 西の壁
					if(!bGetWWallMesh(lpMeshBuilder))
						return FALSE;
					break;

				case 9:		// 東の壁
					if(!bGetEWallMesh(lpMeshBuilder))
						return FALSE;
					break;

				case 10:	// 北の壁
					if(!bGetNWallMesh(lpMeshBuilder))
						return FALSE;
					break;

				case 11:	// 南の壁
					if(!bGetSWallMesh(lpMeshBuilder))
						return FALSE;
					break;

				case 14:	// 北側のドア
					if(!bGetNDoorMesh(lpMeshBuilder))
						return FALSE;

					// 頂点を適切な位置に移動します
					TRANSLATE();

					// メッシュをフレームに追加します
					if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
						return FALSE;

					// このメッシュビルダを解放し、新しいビルダを取得させます
					lpMeshBuilder->Release();
					if(!bGetNSCorridorMesh(lpMeshBuilder))
						return FALSE;
	
					break;

				case 15:	// 南側のドア
					if(!bGetSDoorMesh(lpMeshBuilder))
						return FALSE;

					// 頂点を適切な位置に移動します
					TRANSLATE();

					// メッシュをフレームに追加します
					if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
						return FALSE;

					// このメッシュビルダを解放し、新しいビルダを取得させます
					lpMeshBuilder->Release();
					if(!bGetNSCorridorMesh(lpMeshBuilder))
						return FALSE;
	
					break;

				case 16:	// 東側のドア
					if(!bGetEDoorMesh(lpMeshBuilder))
						return FALSE;

					// 頂点を適切な位置に移動します
					TRANSLATE();

					// メッシュをフレームに追加します
					if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
						return FALSE;

					// このメッシュビルダを解放し、新しいビルダを取得させます
					lpMeshBuilder->Release();
					if(!bGetEWCorridorMesh(lpMeshBuilder))
						return FALSE;
	
					break;

				case 17:	// 南東の角、北にドア
					if(!bGetNDoorMesh(lpMeshBuilder))
						return FALSE;

					// 頂点を適切な位置に移動します
					TRANSLATE();

					// メッシュをフレームに追加します
					if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
						return FALSE;

					// このメッシュビルダを解放し、新しいビルダを取得させます
					lpMeshBuilder->Release();
					if(!bGetSECornerMesh(lpMeshBuilder))
						return FALSE;
	
					break;
	
				case 18:	// 北にドア、南に壁
					if(!bGetNDoorMesh(lpMeshBuilder))
						return FALSE;

					// 頂点を適切な位置に移動します
					TRANSLATE();

					// メッシュをフレームに追加します
					if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
						return FALSE;

					// このメッシュビルダを解放し、新しいビルダを取得させます
					lpMeshBuilder->Release();
					if(!bGetSWallMesh(lpMeshBuilder))
						return FALSE;
	
					break;

				case 20:	// 東にドア、西に壁
					if(!bGetEDoorMesh(lpMeshBuilder))
						return FALSE;

					// 頂点を適切な位置に移動します
					TRANSLATE();

					// メッシュをフレームに追加します
					if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
						return FALSE;

					// このメッシュビルダを解放し、新しいビルダを取得させます
					lpMeshBuilder->Release();
					if(!bGetWWallMesh(lpMeshBuilder))
						return FALSE;
	
					break;


				case 21:	// 南にドア、壁はなし
					if(!bGetSDoorMesh(lpMeshBuilder))
						return FALSE;
	
					break;


				case 22:	// 北にドア、壁はなし
					if(!bGetNDoorMesh(lpMeshBuilder))
						return FALSE;
	
					break;


				case 23:	// 西にドア、壁はなし
					if(!bGetWDoorMesh(lpMeshBuilder))
						return FALSE;
	
					break;


				case 24:	// 北と南にドア、壁はなし
					if(!bGetNDoorMesh(lpMeshBuilder))
						return FALSE;

					// 頂点を適切な位置に移動します
					TRANSLATE();

					// メッシュをフレームに追加します
					if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
						return FALSE;

					// このメッシュビルダを解放し、新しいビルダを取得させます
					lpMeshBuilder->Release();
					if(!bGetSDoorMesh(lpMeshBuilder))
						return FALSE;
	
					break;


				case 25:	// 入り口 (行き止まり)
					if(!bGetNSCorridorMesh(lpMeshBuilder))
						return FALSE;

					// 頂点を適切な位置に移動します
					TRANSLATE();

					// メッシュをフレームに追加します
					if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
						return FALSE;

					// このメッシュビルダを解放し、新しいビルダを取得させます
					lpMeshBuilder->Release();
					if(!bGetSWallMesh(lpMeshBuilder))
						return FALSE;
			
					break;


				default:	// 何もしない (安全のため、上のcaseに
							// 当てはまらないものをここで無視します)
					lpMeshBuilder = NULL;
				}

			
			// メッシュが作成されていたら、適切な位置に移動し、フレームに
			// 追加します。そして、天井と床を加えます
			if(lpMeshBuilder != NULL)
				{
				// 頂点を適切な位置に移動します
				TRANSLATE();
	
				// メッシュをフレームに追加します
				if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
					return FALSE;
				
				lpMeshBuilder->Release();

				// 床
				if(!bGetFloorMesh(lpMeshBuilder))
					return FALSE;

				// 頂点を適切な位置に移動します
				TRANSLATE();
	
				// メッシュをフレームに追加します
				if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
					return FALSE;
				
				lpMeshBuilder->Release();

				// 天井
				if(!bGetCeilingMesh(lpMeshBuilder))
					return FALSE;

				// 頂点を適切な位置に移動します
				TRANSLATE();
	
				// メッシュをフレームに追加します
				if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
					return FALSE;
				
				lpMeshBuilder->Release();
				}

			lpMeshBuilder = NULL;
			}


	// 金貨を含むフレームをダンジョンに追加します
	AddGoldFrames(lpDungeonFrame);

	// シーン内でフレームの位置を設定します
	if(FAILED(lpD3DRMCamera->SetPosition(lpD3DRMScene, fGetXPos(), 1.5f, fGetZPos())))
		return FALSE;

	// 方向を設定します
	if(FAILED(lpD3DRMCamera->SetOrientation(lpD3DRMScene, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f)))
		return FALSE;

	// 使い終わったインターフェイスを解放します
	lpTxtFloor->Release();
	lpTxtCeiling->Release();
	lpTxtWall->Release();
	lpTxtGold->Release();

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// 金貨のある床のメッシュを作成します
///////////////////////////////////////////////////////////////////////////////
BOOL bGetGoldFloorMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
	{
	D3DVECTOR Vertices[] = { -3.0f, 0.1f, 3.0f,  3.0f, 0.1f, 3.0f,	
							  3.0f, 0.1f, -3.0f,  -3.0f, 0.1f, -3.0f };

	int	Faces[] = {4, 0, 1, 2, 3, 0};			

	// メッシュビルダを作成します
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder)))	return FALSE;

	// すべての面の色を白に設定します
	// これがテクスチャと組み合わされます
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));
	
	// 配列からメッシュを作成します
	lpMeshBuilder->AddFaces( 4, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// テクスチャを設定します
	lpMeshBuilder->SetTexture(lpTxtGold);

	// 床のテクスチャの座標を設定します
	lpMeshBuilder->SetTextureCoordinates(0,0.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(1,0.f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(2,1.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(3,1.0f,0.0f);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// 床のメッシュを作成します
BOOL bGetFloorMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { -3.0f, 0.0f, 3.0f,  3.0f, 0.0f, 3.0f,	
							  3.0f, 0.0f, -3.0f,  -3.0f, 0.0f, -3.0f };

	int	Faces[] = {4, 0, 1, 2, 3, 0};			

	// メッシュビルダを作成します
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder)))	return FALSE;

	// すべての面の色を白に設定します
	// これがテクスチャと組み合わされます
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));
	
	// 配列からメッシュを作成します
	lpMeshBuilder->AddFaces( 4, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// テクスチャを設定します
	lpMeshBuilder->SetTexture(lpTxtFloor);

	// 床のテクスチャの座標を設定します
	lpMeshBuilder->SetTextureCoordinates(0,0.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(1,0.f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(2,1.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(3,1.0f,0.0f);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// 天井のメッシュを作成します
BOOL bGetCeilingMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { -3.0f, 3.0f, 3.0f,  -3.0f, 3.0f, -3.0f,	
							  3.0f, 3.0f, -3.0f, 3.0f, 3.0f, 3.0f };

	int	Faces[] = {4, 0, 1, 2, 3, 0};

	// メッシュビルダを作成します
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder)))
		return FALSE;

	// すべての面の色を白に設定します
	// これがテクスチャと組み合わされます
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));
	
	// 配列からメッシュを作成します
	lpMeshBuilder->AddFaces( 4, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// テクスチャを設定します
	lpMeshBuilder->SetTexture(lpTxtCeiling);

	// 天井のテクスチャを作成します
	lpMeshBuilder->SetTextureCoordinates(0,0.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(1,0.f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(2,1.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(3,1.0f,0.0f);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// 南北の回廊のメッシュを作成します
BOOL bGetNSCorridorMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { -3.0f, 0.0f, -3.0f, -3.0f, 3.0f, -3.0f,	// 西側の壁
							 -3.0f, 3.0f, 3.0f, -3.0f, 0.0f,  3.0f,	

							 3.0f, 0.0f,  3.0f, 3.0f, 3.0f,  3.0f,	// 東側の壁
							 3.0f, 3.0f, -3.0f, 3.0f, 0.0f, -3.0f,	};

	int Faces[] = 	{4, 0, 1, 2, 3,	// 西の内側
		  	4, 4, 5, 6, 7,	// 東の内側
			0};		// 終わりを示します

	// メッシュビルダを作成します
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder))) return FALSE;

	// すべての壁面の色を白に設定します
	// これがテクスチャと組み合わされます
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));
	
	// 配列からメッシュを作成します
	lpMeshBuilder->AddFaces( 8, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// テクスチャを設定します
	lpMeshBuilder->SetTexture(lpTxtWall);
	
	// テクスチャ座標を設定します
	// 各面には4つの頂点があり、反時計方向に配置されます
	// 単純なループはテクスチャ座標をすべて同じ値に設定します
	// 次の座標は壁面のパネルの座標です
	for(int i = 0; i < 8; i+= 4){
		lpMeshBuilder->SetTextureCoordinates(i,0.0f,0.0f);
		lpMeshBuilder->SetTextureCoordinates(i+1,1.0f,0.0f);
		lpMeshBuilder->SetTextureCoordinates(i+2,1.0f,1.0f);
		lpMeshBuilder->SetTextureCoordinates(i+3,0.0f,1.0f);
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// 東西の回廊のメッシュを作成します
BOOL bGetEWCorridorMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { -3.0f, 0.0f, 3.0f,	-3.0f, 3.0f, 3.0f,		// 北の壁
							 3.0f, 3.0f, 3.0f,	 3.0f, 0.0f,  3.0f,	

							 3.0f, 0.0f,  -3.0f,  3.0f, 3.0f,  -3.0f,	// 南の壁
							-3.0f, 3.0f, -3.0f,	-3.0f, 0.0f, -3.0f,	};

	int Faces[] = {4, 0, 1, 2, 3,	// 内側の左 
	 			   4, 4, 5, 6, 7,	// 内側の右
				   0};				// 終わりを示します


	// メッシュビルダを作成します
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder)))	return FALSE;

	// すべての壁面の色を白に設定します
	// これがテクスチャと組み合わされます
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));

	// 配列からメッシュを作成します
	lpMeshBuilder->AddFaces( 8, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// テクスチャを設定します
	lpMeshBuilder->SetTexture(lpTxtWall);

	// テクスチャ座標を設定します
	// 各面には4つの頂点があり、反時計方向に配置されます
	// 単純なループはテクスチャ座標をすべて同じ値に設定します
	for(int i = 0; i < 8; i+= 4){
		lpMeshBuilder->SetTextureCoordinates(i,0.0f,0.0f);
		lpMeshBuilder->SetTextureCoordinates(i+1,1.0f,0.0f);
		lpMeshBuilder->SetTextureCoordinates(i+2,1.0f,1.0f);
		lpMeshBuilder->SetTextureCoordinates(i+3,0.0f,1.0f);
	}

	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// 北西の角のメッシュを作成します
BOOL bGetNWCornerMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
	{
	D3DVECTOR Vertices[] = { -3.0f, 0.0f, 3.0f,	-3.0f, 3.0f, 3.0f,		// 北の壁
							 3.0f, 3.0f, 3.0f,	 3.0f, 0.0f,  3.0f,	

							-3.0f, 0.0f,  -3.0f,  -3.0f, 3.0f, -3.0f,	// 西の壁
							-3.0f, 3.0f,  3.0f,	-3.0f, 0.0f, 3.0f,	};

	int Faces[] = {4, 0, 1, 2, 3,	// 内側の左
	 			   4, 4, 5, 6, 7,	// 内側の右
				   0};				// 終わりを示します


	// メッシュビルダを作成します
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder)))	return FALSE;

	// すべての壁面の色を白に設定します
	// これがテクスチャと組み合わされます
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));

	// 配列からメッシュを作成します
	lpMeshBuilder->AddFaces( 8, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// テクスチャを設定します
	lpMeshBuilder->SetTexture(lpTxtWall);

	// テクスチャ座標を設定します
	// 各面には4つの頂点があり、反時計方向に配置されます
	// 単純なループはテクスチャ座標をすべて同じ値に設定します
	for(int i = 0; i < 8; i+= 4){
		lpMeshBuilder->SetTextureCoordinates(i,0.0f,0.0f);
		lpMeshBuilder->SetTextureCoordinates(i+1,1.0f,0.0f);
		lpMeshBuilder->SetTextureCoordinates(i+2,1.0f,1.0f);
		lpMeshBuilder->SetTextureCoordinates(i+3,0.0f,1.0f);
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// 南西の角のメッシュを作成します
BOOL bGetSWCornerMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { 3.0f, 0.0f, -3.0f,	3.0f, 3.0f, -3.0f,		// 南の壁
							-3.0f, 3.0f, -3.0f,	-3.0f, 0.0f, -3.0f,	

							-3.0f, 0.0f,  -3.0f,  -3.0f, 3.0f, -3.0f,	// 西の壁
							-3.0f, 3.0f,  3.0f,	-3.0f, 0.0f, 3.0f,	};

	int Faces[] = {4, 0, 1, 2, 3,	// 内側の左
	 			   4, 4, 5, 6, 7,	// 内側の右
				   0};				// 終わりを示します

	// メッシュビルダを作成します
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder)))
		return FALSE;

	// すべての壁面の色を白に設定します
	// これがテクスチャと組み合わされます
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));

	// 配列からメッシュを作成します
	lpMeshBuilder->AddFaces( 8, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// テクスチャを設定します
	lpMeshBuilder->SetTexture(lpTxtWall);

	// テクスチャ座標を設定します
	// 各面には4つの頂点があり、反時計方向に配置されます
	// 単純なループはテクスチャ座標をすべて同じ値に設定します
	for(int i = 0; i < 8; i+= 4){
		lpMeshBuilder->SetTextureCoordinates(i,0.0f,0.0f);
		lpMeshBuilder->SetTextureCoordinates(i+1,1.0f,0.0f);
		lpMeshBuilder->SetTextureCoordinates(i+2,1.0f,1.0f);
		lpMeshBuilder->SetTextureCoordinates(i+3,0.0f,1.0f);
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// 北東の角のメッシュを作成します
BOOL bGetNECornerMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { -3.0f, 0.0f, 3.0f,	-3.0f, 3.0f, 3.0f,		// 北の壁
							 3.0f, 3.0f, 3.0f,	 3.0f, 0.0f,  3.0f,	

							 3.0f, 0.0f,  3.0f,  3.0f, 3.0f,  3.0f,		// 東の壁
							 3.0f, 3.0f, -3.0f,	3.0f, 0.0f, -3.0f,	};

	int Faces[] = {4, 0, 1, 2, 3,	// 内側の左
	 			   4, 4, 5, 6, 7,	// 内側の右
				   0};				// 終わりを示します


	// メッシュビルダを作成します
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder)))	return FALSE;

	// すべての壁面の色を白に設定します
	// これがテクスチャと組み合わされます
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));

	// 配列からメッシュを作成します
	lpMeshBuilder->AddFaces(	8, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// テクスチャを設定します
	lpMeshBuilder->SetTexture(lpTxtWall);

	// テクスチャ座標を設定します
	// 各面には4つの頂点があり、反時計方向に配置されます
	// 単純なループはテクスチャ座標をすべて同じ値に設定します
	for(int i = 0; i < 8; i+= 4){
		lpMeshBuilder->SetTextureCoordinates(i,0.0f,0.0f);
		lpMeshBuilder->SetTextureCoordinates(i+1,1.0f,0.0f);
		lpMeshBuilder->SetTextureCoordinates(i+2,1.0f,1.0f);
		lpMeshBuilder->SetTextureCoordinates(i+3,0.0f,1.0f);
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// 南東の角のメッシュを作成します
BOOL bGetSECornerMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = {	3.0f, 0.0f, -3.0f,	3.0f, 3.0f, -3.0f,		// 南の壁
								-3.0f, 3.0f, -3.0f,	-3.0f, 0.0f, -3.0f,	

								3.0f, 0.0f,  3.0f,  3.0f, 3.0f,  3.0f,		// 西の壁
								3.0f, 3.0f, -3.0f,	3.0f, 0.0f, -3.0f,	};

	int Faces[] = {4, 0, 1, 2, 3,	// 内側の左
	 			   4, 4, 5, 6, 7,	// 内側の右
				   0};				// 終わりを示します


	// メッシュビルダを作成します
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder)))
		return FALSE;

	// すべての壁面の色を白に設定します
	// これがテクスチャと組み合わされます
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));

	// 配列からメッシュを作成します
	lpMeshBuilder->AddFaces( 8, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// テクスチャを設定します
	lpMeshBuilder->SetTexture(lpTxtWall);

	// テクスチャ座標を設定します
	// 各面には4つの頂点があり、反時計方向に配置されます
	// 単純なループはテクスチャ座標をすべて同じ値に設定します
	for(int i = 0; i < 8; i+= 4){
		lpMeshBuilder->SetTextureCoordinates(i,0.0f,0.0f);
		lpMeshBuilder->SetTextureCoordinates(i+1,1.0f,0.0f);
		lpMeshBuilder->SetTextureCoordinates(i+2,1.0f,1.0f);
		lpMeshBuilder->SetTextureCoordinates(i+3,0.0f,1.0f);
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// 西側の壁のメッシュを作成します
BOOL bGetWWallMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { -3.0f, 0.0f,  -3.0f,  -3.0f, 3.0f, -3.0f,	// 西の壁
								-3.0f, 3.0f,  3.0f,	-3.0f, 0.0f, 3.0f  };

	int	Faces[] = {4, 0, 1, 2, 3, 0};

	// メッシュビルダを作成します
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder)))
		return FALSE;

	// すべての壁面の色を白に設定します
	// これがテクスチャと組み合わされます
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));
	
	// 配列からメッシュを作成します
	lpMeshBuilder->AddFaces( 4, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// テクスチャを設定します
	lpMeshBuilder->SetTexture(lpTxtWall);

	// テクスチャ座標を設定します
	lpMeshBuilder->SetTextureCoordinates(0,0.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(1,0.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(2,1.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(3,1.0f,0.0f);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// 東の壁のメッシュを作成します
BOOL bGetEWallMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { 3.0f, 0.0f,  3.0f,  3.0f, 3.0f,  3.0f,	// 東の壁
								3.0f, 3.0f, -3.0f,	3.0f, 0.0f, -3.0f};

	int	Faces[] = {4, 0, 1, 2, 3, 0};

	// メッシュビルダを作成します
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder)))
		return FALSE;

	// すべての壁面の色を白に設定します
	// これがテクスチャと組み合わされます
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));
	
	// 配列からメッシュを作成します
	lpMeshBuilder->AddFaces( 4, Vertices, 
							0, NULL, 
							(unsigned long *)Faces, NULL);

	// テクスチャを設定します
	lpMeshBuilder->SetTexture(lpTxtWall);

	// テクスチャ座標を設定します
	lpMeshBuilder->SetTextureCoordinates(0,0.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(1,0.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(2,1.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(3,1.0f,0.0f);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// 北の壁のメッシュを作成します
BOOL bGetNWallMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
	{
	D3DVECTOR Vertices[] = { -3.0f, 0.0f, 3.0f,	-3.0f, 3.0f, 3.0f,		// 北の壁
								3.0f, 3.0f, 3.0f,	 3.0f, 0.0f,  3.0f};

	int	Faces[] = {4, 0, 1, 2, 3, 0};

	// メッシュビルダを作成します
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder))) return FALSE;

	// すべての壁面の色を白に設定します
	// これがテクスチャと組み合わされます
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));
	
	// 配列からメッシュを作成します
	lpMeshBuilder->AddFaces( 4, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// テクスチャを設定します
	lpMeshBuilder->SetTexture(lpTxtWall);

	// テクスチャ座標を設定します
	lpMeshBuilder->SetTextureCoordinates(0,0.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(1,0.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(2,1.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(3,1.0f,0.0f);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// 南の壁のメッシュを作成します
BOOL bGetSWallMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { 3.0f, 0.0f, -3.0f,	3.0f, 3.0f, -3.0f,		// 南の壁
							-3.0f, 3.0f, -3.0f,	-3.0f, 0.0f, -3.0f};

	int	Faces[] = {4, 0, 1, 2, 3, 0};

	// メッシュビルダを作成します
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder))) return FALSE;

	// すべての壁面の色を白に設定します
	// これがテクスチャと組み合わされます
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));
	
	// 配列からメッシュを作成します
	lpMeshBuilder->AddFaces( 4, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// テクスチャを設定します
	lpMeshBuilder->SetTexture(lpTxtWall);

	// テクスチャ座標を設定します
	lpMeshBuilder->SetTextureCoordinates(0,0.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(1,0.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(2,1.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(3,1.0f,0.0f);

	return TRUE;
}



///////////////////////////////////////////////////////////////////////////////
// 北側のドアのメッシュを作成します
BOOL bGetNDoorMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { -3.0f, 0.0f, 3.0f,	-3.0f, 3.0f, 3.0f,		// 北のドア
								3.0f, 3.0f, 3.0f,	 3.0f, 0.0f,  3.0f};

	int	Faces[] = {4, 0, 1, 2, 3, 0};

	// メッシュビルダを作成します
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder)))
		return FALSE;

	// すべての壁面の色を白に設定します
	// これがテクスチャと組み合わされます
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));
	
	// 配列からメッシュを作成します
	lpMeshBuilder->AddFaces( 4, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// テクスチャを設定します
	lpMeshBuilder->SetTexture(lpTxtWood);

	// テクスチャ座標を設定します
	lpMeshBuilder->SetTextureCoordinates(0,0.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(1,1.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(2,1.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(3,0.0f,1.0f);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// 南側のドアのメッシュを作成します
BOOL bGetSDoorMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { 3.0f, 0.0f, -3.0f,	3.0f, 3.0f, -3.0f,		// 南のドア
							-3.0f, 3.0f, -3.0f,	-3.0f, 0.0f, -3.0f};

	int	Faces[] = {4, 0, 1, 2, 3, 0};

	// メッシュビルダを作成します
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder))) return FALSE;

	// すべての壁面の色を白に設定します
	// これがテクスチャと組み合わされます
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));
	
	// 配列からメッシュを作成します
	lpMeshBuilder->AddFaces( 4, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// テクスチャを設定します
	lpMeshBuilder->SetTexture(lpTxtWood);

	// テクスチャ座標を設定します
  	lpMeshBuilder->SetTextureCoordinates(0,0.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(1,1.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(2,1.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(3,0.0f,1.0f);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// 東側のドアのメッシュを作成します
BOOL bGetEDoorMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { 3.0f, 0.0f,  3.0f,  3.0f, 3.0f,  3.0f,	// 東のドア
								3.0f, 3.0f, -3.0f,	3.0f, 0.0f, -3.0f};

	int	Faces[] = {4, 0, 1, 2, 3, 0};

	// メッシュビルダを作成します
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder))) return FALSE;

	// すべての壁面の色を白に設定します
	// これがテクスチャと組み合わされます
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));
	
	// 配列からメッシュを作成します
	lpMeshBuilder->AddFaces( 4, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// テクスチャを設定します
	lpMeshBuilder->SetTexture(lpTxtWood);

	// テクスチャ座標を設定します
  	lpMeshBuilder->SetTextureCoordinates(0,0.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(1,1.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(2,1.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(3,0.0f,1.0f);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// 西側のドアのメッシュを作成します
BOOL bGetWDoorMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { -3.0f, 0.0f,  -3.0f,  -3.0f, 3.0f, -3.0f,	// 西のドア
							-3.0f, 3.0f,  3.0f,	-3.0f, 0.0f, 3.0f  };


	int	Faces[] = {4, 0, 1, 2, 3, 0};

	// メッシュビルダを作成します
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder))) return FALSE;

	// すべての壁面の色を白に設定します
	// これがテクスチャと組み合わされます
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));
	
	// 配列からメッシュを作成します
	lpMeshBuilder->AddFaces( 4, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// テクスチャを設定します
	lpMeshBuilder->SetTexture(lpTxtWood);

	// テクスチャ座標を設定します
	lpMeshBuilder->SetTextureCoordinates(0,0.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(1,1.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(2,1.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(3,0.0f,1.0f);

	return TRUE;
}

//エラーハンドラとCOMオブジェクトの解放
void ReleaseAll(void)
{

/*
	if(lpMeshBuilder != NULL)
		lpMeshBuilder->Release();

	if(lpTxtWood != NULL)
		lpTxtWood->Release();
*/

	if(lpD3DRMCamera != NULL)
		lpD3DRMCamera->Release();
	if(lpD3DRMScene != NULL)
		lpD3DRMScene->Release();
	if(lpD3DRMView != NULL)
		lpD3DRMView->Release();
	if(lpD3DRMDevice != NULL)
		lpD3DRMDevice->Release();
	if(lpDirect3DRM != NULL)
		lpDirect3DRM->Release();

	if(lpDirect3D != NULL)
		lpDirect3D->Release();
	if(lpD3DDevice != NULL)
		lpD3DDevice->Release();

	if(lpZbuffer != NULL)
		lpZbuffer->Release();
	if(lpPrimary != NULL)
		lpPrimary->Release();
//	if(lpBackbuffer != NULL)
//		lpBackbuffer->Release();
	if(lpDDClipper != NULL)
		lpDDClipper->Release();
	if(lpDirectDraw != NULL)
		lpDirectDraw->Release();
}
