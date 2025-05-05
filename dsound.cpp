// dsound.cpp
// DirectSound�̎g�p��

// Win32�̕s�v�ȋ@�\��r��
#define WIN32_LEAN_AND_MEAN

#include <windows.h>	// Windows�̕W���w�b�_�[
#include <mmsystem.h>	// �}���`���f�B�A�@�\�ƍ\����
#include <direct.h>		// DirectX�̒�`
#include <dsound.h>		// DirectSound�̃w�b�_�[

#include <stdio.h>		// �t�@�C��I/O�@�\

// DirectSound�̃|�C���^
LPDIRECTSOUND pDirectSoundAPI = NULL;
LPDIRECTSOUNDBUFFER pGrunt = NULL;
LPDIRECTSOUNDBUFFER pScore = NULL;

///////////////////////////////////////////////////////////////////////////////
// WAVE�t�@�C������DirectSound�o�b�t�@���쐬���܂�
///////////////////////////////////////////////////////////////////////////////
BOOL bSetupBufferFromWave(LPDIRECTSOUNDBUFFER& pDSBuffer, char *szWaveFile)
	{
	HMMIO	hmfr;
	MMCKINFO parent,child;
	WAVEFORMATEX wfmtx;

	// WAVE�t�@�C�����}���`���f�B�AI/O�֐��ŊJ���܂�
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

	// WAV�̃t�H�[�}�b�g����ǂݍ��݂܂�
	if(mmioRead(hmfr, (char *)&wfmtx, sizeof(wfmtx)) != sizeof(wfmtx))
		{
		mmioClose(hmfr,0);
		MessageBox(NULL,"Error reading Wave Format",NULL,MB_OK);
		return FALSE;
		}

	// ���̃t�@�C����WAVE�t�@�C���ł��邱�Ƃ��m�F���܂�
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
		
	// �`�����N���\���̂ɓǂݍ��݂܂�
	child.ckid = mmioFOURCC('d','a','t','a');
	if(mmioDescend(hmfr,&child, &parent, MMIO_FINDCHUNK))
		{
		mmioClose(hmfr,0);
		MessageBox(NULL,"Wave file has no data",NULL,MB_OK);
		return FALSE;
		}

	// �����������蓖�āA�f�[�^��ǂݍ��݂܂�
	BYTE *pBuffer= new BYTE[child.cksize];
	if((DWORD)mmioRead(hmfr, (char *)pBuffer, child.cksize) != child.cksize)
		{
		mmioClose(hmfr,0);
		MessageBox(NULL,"Could not read Wave Data",NULL,MB_OK);
		return FALSE;
		}

	// WAVE�t�@�C������܂�
	mmioClose(hmfr,0);


	// DirectSound�o�b�t�@���쐬���܂�
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
	dsbdesc.dwFlags = DSBCAPS_CTRLDEFAULT;		// DirectX7�ł͎g��Ȃ� DirectX6�ȉ��̂Ƃ��ɋL�q�B
	dsbdesc.dwBufferBytes = child.cksize;			
	dsbdesc.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf;

	if(pDirectSoundAPI->CreateSoundBuffer(&dsbdesc, &pDSBuffer, NULL) != DS_OK)
		{
		MessageBox(NULL,"Could not Create Sound Buffer.",NULL,MB_OK);
		delete [] pBuffer;
		}


	// WAV�t�@�C���̃f�[�^���o�b�t�@�ɓ]�����܂�
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

	// ��Ɨp�̃o�b�t�@��������܂�
	delete [] pBuffer;

	return TRUE;
	}



///////////////////////////////////////////////////////////////////////////////
// DirectSound�̏�����
//  DirectSound�̃|�C���^���擾���A2�̃T�E���h�o�b�t�@���������܂�
///////////////////////////////////////////////////////////////////////////////
BOOL bInitializeDirectSound(HWND hWnd)
	{
	// DirectSound�̍쐬�Ɏ��s������A���^�[�����邾���ł� (���ʉ��Ȃ�)
	if(DirectSoundCreate(NULL,&pDirectSoundAPI,NULL) != DS_OK)
		return FALSE;

	pDirectSoundAPI->SetCooperativeLevel(hWnd, DSSCL_NORMAL);

	// ���߂����̂��߂̃o�b�t�@���쐬���܂�
	if(!bSetupBufferFromWave(pGrunt,"grunt.wav"))
		{
		MessageBox(NULL,"Grunt failed",NULL,MB_OK);
		return FALSE;
		}

	// ���_�𓾂��Ƃ��̉��̃o�b�t�@���쐬���܂�
	if(!bSetupBufferFromWave(pScore,"score.wav"))
		{
		MessageBox(NULL,"Score failed",NULL,MB_OK);
		return FALSE;
		}

	return TRUE;
	}





///////////////////////////////////////////////////////////////////////////////
// DirectSound�̏I��
//  2�̃T�E���h�o�b�t�@��DirectSound��������܂�
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
// ���߂������Đ����܂�
///////////////////////////////////////////////////////////////////////////////
void DDGrunt(void)
	{
	if(pGrunt)
		pGrunt->Play(0,0,0);
	}

///////////////////////////////////////////////////////////////////////////////
// ���_�𓾂��Ƃ��̉����Đ����܂�
///////////////////////////////////////////////////////////////////////////////
void DDScore(void)
	{
	if(pScore)
		pScore->Play(0,0,0);
	}