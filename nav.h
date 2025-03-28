///////////////////////////////////////////////////////////////////////////////
// nav.h
// プレーヤーの移動を処理する関数のプロトタイプなど

// 迷宮の配列の大きさ
#define MAX_ROW	18
#define MAX_COL 22

// 金貨が落ちている部分は、この構造体で表現する
struct GoldLocations
{
	LPDIRECT3DRMFRAME lpGoldFrame;
	int nRow;
	int nCol;
	BOOL bFound;
};

// 関数のプロトタイプ
void SetupNavigator(void);
float TurnLeft(void);
float TurnRight(void);
float SwingLeft(void);
float SwingRight(void);
float fGetXPos(void);
float fGetZPos(void);
float fGetAngle(void);
void StepForward(void);
void StepBackward(void);
void RunForward(void);
void RunBackward(void);
void Move(float fRowDelta, float fColDelta);

BOOL bCheckForScore(int row, int column);
