// dsound.cpp
// DirectSoundの使用例

// Win32の不要な機能を排除
#define WIN32_LEAN_AND_MEAN

#include <windows.h>	// Windowsの標準ヘッダー
#include <mmsystem.h>	// マルチメディア機能と構造体
#include <direct.h>		// DirectXの定義
#include <dsound.h>		// DirectSoundのヘッダー

#include <stdio.h>		// ファイルI/O機能

// DirectSoundのポインタ
LPDIRECTSOUND pDirectSoundAPI = NULL;
LPDIRECTSOUNDBUFFER pGrunt = NULL;
LPDIRECTSOUNDBUFFER pScore = NULL;

///////////////////////////////////////////////////////////////////////////////
// WAVEファイルからDirectSoundバッファを作成します
///////////////////////////////////////////////////////////////////////////////
BOOL bSetupBufferFromWave(LPDIRECTSOUNDBUFFER& pDSBuffer, char *szWaveFile)
	{
	HMMIO	hmfr;
	MMCKINFO parent,child;
	WAVEFORMATEX wfmtx;

	// WAVEファイルをマルチメディアI/O関数で開きます
	hmfr = mmioOpen(szWaveFile,NULL, MMIO_READ | MMIO_ALLOCBUF);
	if(hmfr == NULL)
		{
		MessageBox(NULL,"Could not Open Wave File",NULL,MB_OK);
		return FALSE;
		}

	parent.ckid = (FOURCC)0;
	parent.cksize = 0;
	parent.fccType = (FOURCC)0;
	parent.dwDataOffset = 0;
	parent.dwFlags = 0;
	child = parent;

	parent.fccType = mmioFOURCC('W','A','V','E');
	if(mmioDescend(hmfr,&parent,NULL,MMIO_FINDRIFF))
		{
		mmioClose(hmfr,0);
		MessageBox(NULL,"Could not Descend into Wave File",NULL,MB_OK);
		return FALSE;
		}

	child.ckid = mmioFOURCC('f','m','t',' ');
	if(mmioDescend(hmfr,&child,&parent,0))
		{
		mmioClose(hmfr,0);
		MessageBox(NULL,"Could not Descend into format of Wave File",NULL,MB_OK);
		return FALSE;
		}

	// WAVのフォーマット情報を読み込みます
	if(mmioRead(hmfr, (char *)&wfmtx, sizeof(wfmtx)) != sizeof(wfmtx))
		{
		mmioClose(hmfr,0);
		MessageBox(NULL,"Error reading Wave Format",NULL,MB_OK);
		return FALSE;
		}

	// このファイルがWAVEファイルであることを確認します
	if(wfmtx.wFormatTag != WAVE_FORMAT_PCM)
		{
		mmioClose(hmfr,0);
		MessageBox(NULL,"Not a valid Wave Format",NULL,MB_OK);
		return FALSE;
		}
		
	if(mmioAscend(hmfr, &child, 0))
		{
		mmioClose(hmfr,0);
		MessageBox(NULL,"Unable to Ascend",NULL,MB_OK);
		return FALSE;
		}
		
	// チャンクを構造体に読み込みます
	child.ckid = mmioFOURCC('d','a','t','a');
	if(mmioDescend(hmfr,&child, &parent, MMIO_FINDCHUNK))
		{
		mmioClose(hmfr,0);
		MessageBox(NULL,"Wave file has no data",NULL,MB_OK);
		return FALSE;
		}

	// メモリを割り当て、データを読み込みます
	BYTE *pBuffer= new BYTE[child.cksize];
	if((DWORD)mmioRead(hmfr, (char *)pBuffer, child.cksize) != child.cksize)
		{
		mmioClose(hmfr,0);
		MessageBox(NULL,"Could not read Wave Data",NULL,MB_OK);
		return FALSE;
		}

	// WAVEファイルを閉じます
	mmioClose(hmfr,0);


	// DirectSoundバッファを作成します
	DSBUFFERDESC dsbdesc;
	PCMWAVEFORMAT pcmwf;
	
	memset(&pcmwf, 0, sizeof(PCMWAVEFORMAT));
	pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
	pcmwf.wf.nChannels	= wfmtx.nChannels;
	pcmwf.wf.nSamplesPerSec	= wfmtx.nSamplesPerSec;
	pcmwf.wf.nBlockAlign = wfmtx.nBlockAlign;
	pcmwf.wf.nAvgBytesPerSec = wfmtx.nAvgBytesPerSec;
	pcmwf.wBitsPerSample = wfmtx.wBitsPerSample;
	

	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_CTRLDEFAULT;
	dsbdesc.dwBufferBytes = child.cksize;			
	dsbdesc.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf;

	if(pDirectSoundAPI->CreateSoundBuffer(&dsbdesc, &pDSBuffer, NULL) != DS_OK)
		{
		MessageBox(NULL,"Could not Create Sound Buffer.",NULL,MB_OK);
		delete [] pBuffer;
		}


	// WAVファイルのデータをバッファに転送します
	LPVOID written1, written2;
	DWORD length1,length2;
	if(pDSBuffer->Lock(0, child.cksize, &written1, &length1, &written2, &length2, 0) == DSERR_BUFFERLOST)
		{
		pDSBuffer->Restore();
		pDSBuffer->Lock(0, child.cksize, &written1, &length1, &written2, &length2, 0);
		}

	CopyMemory(written1, pBuffer, length1);

	if(written2 != NULL)
		CopyMemory(written2, pBuffer+length1, length2);

	pDSBuffer->Unlock(written1, length1, written2, length2);

	// 作業用のバッファを解放します
	delete [] pBuffer;

	return TRUE;
	}



///////////////////////////////////////////////////////////////////////////////
// DirectSoundの初期化
//  DirectSoundのポインタを取得し、2つのサウンドバッファを準備します
///////////////////////////////////////////////////////////////////////////////
BOOL bInitializeDirectSound(HWND hWnd)
	{
	// DirectSoundの作成に失敗したら、リターンするだけです (効果音なし)
	if(DirectSoundCreate(NULL,&pDirectSoundAPI,NULL) != DS_OK)
		return FALSE;

	pDirectSoundAPI->SetCooperativeLevel(hWnd, DSSCL_NORMAL);

	// うめき声のためのバッファを作成します
	if(!bSetupBufferFromWave(pGrunt,"grunt.wav"))
		{
		MessageBox(NULL,"Grunt failed",NULL,MB_OK);
		return FALSE;
		}

	// 得点を得たときの音のバッファを作成します
	if(!bSetupBufferFromWave(pScore,"score.wav"))
		{
		MessageBox(NULL,"Score failed",NULL,MB_OK);
		return FALSE;
		}

	return TRUE;
	}





///////////////////////////////////////////////////////////////////////////////
// DirectSoundの終了
//  2つのサウンドバッファとDirectSoundを解放します
///////////////////////////////////////////////////////////////////////////////
void ShutdownDirectSound(void)
	{
	if(pGrunt)
		{
		pGrunt->Release();
		pGrunt = NULL;
		}
		
	if(pScore)
		{
		pScore->Release();
		pScore = NULL;
		}

	if(pDirectSoundAPI)
		{
		pDirectSoundAPI->Release();
		pDirectSoundAPI = NULL;
		}
	}





///////////////////////////////////////////////////////////////////////////////
// うめき声を再生します
///////////////////////////////////////////////////////////////////////////////
void DDGrunt(void)
	{
	if(pGrunt)
		pGrunt->Play(0,0,0);
	}

///////////////////////////////////////////////////////////////////////////////
// 得点を得たときの音を再生します
///////////////////////////////////////////////////////////////////////////////
void DDScore(void)
	{
	if(pScore)
		pScore->Play(0,0,0);
	}