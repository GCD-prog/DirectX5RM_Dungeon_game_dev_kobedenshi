///////////////////////////////////////////////////////////////////////////////
// Scene.cpp
// ���̃t�@�C���ɂ́A�V�[�����쐬�������_�����O�I�v�V�������Z�b�g����@�\��
// �܂܂�Ă��܂�
// Richard S. Wright Jr.
///////////////////////////////////////////////////////////////////////////////

// Win32�̕K�v�̂Ȃ��@�\�͏Ȃ��܂�
#define WIN32_LEAN_AND_MEAN

#include <time.h>				// �����Ɏg�p

#include "dx.h"

LPDIRECTDRAW lpDirectDraw = NULL;
LPDIRECTDRAWCLIPPER lpDDClipper = NULL;
LPDIRECTDRAWSURFACE lpPrimary = NULL;
LPDIRECTDRAWSURFACE lpBackbuffer = NULL;
LPDIRECTDRAWSURFACE lpZbuffer = NULL;

// ddmain.cpp�Œ�`����Ă���|�C���^
LPDIRECT3DRM	lpDirect3DRM;		// Direct3D�ێ����[�hAPI
LPDIRECT3DRMFRAME lpD3DRMScene;	// Direct3D�̊�ƂȂ�t���[��
LPDIRECT3DRMFRAME lpD3DRMCamera;	// Direct3D�̃J�����t���[��
LPDIRECT3DRMDEVICE lpD3DRMDevice;	// Direct3D�f�o�C�X

//Direct3D �֘A
LPDIRECT3D lpDirect3D = NULL;
LPDIRECT3DDEVICE lpD3DDevice = NULL;
LPDIRECT3DRMVIEWPORT	lpD3DRMView = NULL;	// Direct3D�r���[�|�[�g


// �����̊֐��́A���̃t�@�C���̒��Ń��[�J���ɒ�`����Ă��āA
// �_���W�������\�����郁�b�V���̍쐬�Ɏg���܂�
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

// ����10���̈ʒu�������z��
GoldLocations GoldFrames[10];

// �_���W�����̂ǂ��ɂǂ̃��b�V����z�u���邩��\���z�� (nav.cpp�Œ�`)
extern int iDungeon[MAX_ROW][MAX_COL];

// ���b�V����z�u����}�N��:�������C�����邾���ŁA�ȒP�ɕϊ����@��ς����܂�
#define TRANSLATE() { if(FAILED(lpMeshBuilder->Translate( ((float)c * 6.0f) + 3.0f, 0.0f, (((17.0f - (float)r) * 6.0f) + 3.0f)))) return FALSE; }

// ���[�J���ȃe�N�X�`��
// ��x�쐬������A�g���I��܂łƂ��Ă����܂�
LPDIRECT3DRMTEXTURE lpTxtFloor = NULL;			// �e�N�X�`��API
LPDIRECT3DRMTEXTURE lpTxtCeiling = NULL;			// �e�N�X�`��API
LPDIRECT3DRMTEXTURE lpTxtWood = NULL;			// �e�N�X�`��API
LPDIRECT3DRMTEXTURE lpTxtWall = NULL;			// �e�N�X�`��API
LPDIRECT3DRMTEXTURE lpTxtGold = NULL;			// �e�N�X�`��API
LPDIRECT3DRMFRAME lpDungeonFrame = NULL;			// �_���W����������t���[��

///////////////////////////////////////////////////////////////////////////////
// �����_�����O�I�v�V�����̐ݒ�
//  �w�i�����ɁA�Ɩ��Ȃ��̃t���b�g�V�F�[�f�B���O
///////////////////////////////////////////////////////////////////////////////
BOOL SetRenderingOptions(void)
{
	// �V�[���̔w�i�����ɐݒ肵�܂�
//	lpD3DRMScene->SetSceneBackground(D3DRGB(0,0,0));

	// �V�F�[�f�B���O���[�h���Ɩ��Ȃ��̃t���b�g�ɐݒ肵�܂�
//	lpD3DRMDevice->SetQuality(D3DRMRENDER_UNLITFLAT);	// �����ݒ�

	lpD3DRMDevice->SetQuality(D3DRMLIGHT_ON | D3DRMFILL_SOLID | D3DRMSHADE_GOURAUD);	// ����

	//�A���r�G���g������z�u
	LPDIRECT3DRMLIGHT lpD3DRMLightAmbient;
	
	lpDirect3DRM->CreateLightRGB(D3DRMLIGHT_AMBIENT, D3DVAL(5.0), D3DVAL(5.0), D3DVAL(5.0), &lpD3DRMLightAmbient);
	lpD3DRMScene->AddLight(lpD3DRMLightAmbient);
	lpD3DRMLightAmbient->Release();

	LPDIRECT3DRMFRAME lpD3DRMLightFrame;
	lpDirect3DRM->CreateFrame(lpD3DRMScene, &lpD3DRMLightFrame);
	
	//�|�C���g������z�u
	LPDIRECT3DRMLIGHT lpD3DRMLightPoint;

	lpDirect3DRM->CreateLightRGB(D3DRMLIGHT_POINT, D3DVAL(0.9), D3DVAL(0.9), D3DVAL(0.9), &lpD3DRMLightPoint);
	
	lpD3DRMLightFrame->SetPosition(lpD3DRMScene, D3DVAL(10.0), D3DVAL(0.0), D3DVAL(0.0));
	lpD3DRMLightFrame->AddLight(lpD3DRMLightPoint);

	lpD3DRMLightPoint->Release();

	lpD3DRMLightFrame->Release();

	// ����������TRUE��Ԃ��܂�
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// ���݂̂���t���[�����_���W�����̎q�t���[���Ƃ��Ēǉ����܂�
///////////////////////////////////////////////////////////////////////////////
void AddGoldFrames(LPDIRECT3DRMFRAME& lpDungeonFrame)
	{
	 // ���b�V���r���_�C���^�[�t�F�C�X
	LPDIRECT3DRMMESHBUILDER lpMeshBuilder = NULL;
	int r,c;

	// �����W�F�l���[�^�����������܂�
	srand( (unsigned)time( NULL ) );

	// 10�̃t���[�����쐬���܂�
	for(int i = 0; i < 10; i++)
		{
		// �t���[�����쐬���܂�
		if(FAILED(lpDirect3DRM->CreateFrame(lpD3DRMScene, &(GoldFrames[i].lpGoldFrame))))	return;

		// ���̏�ɋ��݂����郁�b�V�����擾���܂�
		bGetGoldFloorMesh(lpMeshBuilder);

		// ���݂̃����_���Ȉʒu�����肵�܂�
		do {
			r = int( ((float)rand() / (float)RAND_MAX) * (float)MAX_ROW);
			c = int( ((float)rand() / (float)RAND_MAX) * (float)MAX_COL);
			}
		while (iDungeon[r][c] == 0);

		// �_���W�����̓K�؂Ȉʒu�Ɉړ����܂�
		lpMeshBuilder->Translate(((float)c * 6.0f) +3.0f, 0.0f,  (((17.0f - (float)r) * 6.0f) + 3.0f));

		// ���W�ƃt���[���|�C���^���i�[���A�����t���O��FALSE�ɐݒ肵�܂�
		GoldFrames[i].nRow = r;
		GoldFrames[i].nCol = c;
		GoldFrames[i].lpGoldFrame->AddVisual(lpMeshBuilder);
		GoldFrames[i].bFound = FALSE;

		// �t���[�����_���W�����ɒǉ����܂�
		lpDungeonFrame->AddChild(GoldFrames[i].lpGoldFrame);


		// ���b�V���r���_��������܂�
		lpMeshBuilder->Release();
		}
}

///////////////////////////////////////////////////////////////////////////////
// �w�肳�ꂽ�ʒu�ɋ��݂����邩�𒲂ׂ܂��B�������݂�����Γ��_�ƂȂ�A
// �t���[��������݂̃t���[�����_���W���������菜���܂�
///////////////////////////////////////////////////////////////////////////////
BOOL bCheckForScore(int row, int column)
{
	// 10�̋��݂����ꂼ�ꃋ�[�v�Ŋe�ʒu���`�F�b�N���܂�
	for(int i = 0; i < 10; i++)
		{
		if(GoldFrames[i].nRow == row && GoldFrames[i].nCol == column && GoldFrames[i].bFound == FALSE)
			{
			GoldFrames[i].bFound = TRUE;

			// �V�[��������݂��폜���܂�
			lpDungeonFrame->DeleteChild(GoldFrames[i].lpGoldFrame);
			GoldFrames[i].lpGoldFrame->Release();

			return TRUE;
			}
		}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// �V�[�����쐬���܂�
///////////////////////////////////////////////////////////////////////////////
BOOL BuildScene(void)
{
	// ���b�V���r���_�C���^�[�t�F�C�X
	LPDIRECT3DRMMESHBUILDER lpMeshBuilder = NULL;

	// �ŏ��Ƀ����_�����O�I�v�V������ݒ肵�܂�
	if(!SetRenderingOptions())
		return FALSE;

	// �t���[�����쐬���܂�
	if(FAILED(lpDirect3DRM->CreateFrame(lpD3DRMScene, &lpDungeonFrame)))
		return FALSE;

	// �e�N�X�`����p�ӂ��܂�
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


	// �_���W�����S�̂̊e�X�̈ʒu�����[�v���܂��B�e�X�̋�`�ɂ͂ǂ̃��b�V����
	// �����ɔz�u���ׂ����������l�����蓖�Ă��Ă��܂��B���̋�`����ׂ�����
	// ���_���W�����ɂȂ�܂�
	for(int r = 0; r < 18; r++)		
		for(int c = 0; c < 22; c++)		
			{
			switch(iDungeon[r][c])
				{
				case 0: 
					break;	// �������܂���i�_���W�����̊O���j
							// �v���[���[�������Ɉړ����邱�Ƃ��ł��܂���
				
				case 1:		// ��k�̉�L
					if(!bGetNSCorridorMesh(lpMeshBuilder))
						return FALSE;
					break;

				case 2:		// �����̉�L
					if(!bGetEWCorridorMesh(lpMeshBuilder))
						return FALSE;
					break;

				case 3:		// �k���̊p
					if(!bGetNWCornerMesh(lpMeshBuilder))
						return FALSE;
					break;

				case 4:		// �쐼�̊p
					if(!bGetSWCornerMesh(lpMeshBuilder))
						return FALSE;
					break;

				case 5:		// �k���̊p
					if(!bGetNECornerMesh(lpMeshBuilder))
						return FALSE;
					break;

				case 6:		// �쓌�̊p
					if(!bGetSECornerMesh(lpMeshBuilder))
						return FALSE;
					break;

				case 7:	// ����ȏꍇ (���ƓV�䂾��) �ŁA�\���H�ƕ�����
						// �����ɂ�������܂���
					if(!bGetFloorMesh(lpMeshBuilder))
						return FALSE;

					// ���_��K�؂Ȉʒu�Ɉړ����܂�
					TRANSLATE();

					// ���b�V�����t���[���ɒǉ����܂�
					if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
						return FALSE;

					// ���̃��b�V���r���_��������A�V�����r���_���擾�����܂�
					lpMeshBuilder->Release();
					if(!bGetCeilingMesh(lpMeshBuilder))
						return FALSE;

					// ���_��K�؂Ȉʒu�Ɉړ����܂�
					TRANSLATE();

					// ���b�V�����t���[���ɒǉ����܂�
					if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
						return FALSE;
					
					// ���b�V���������NULL���Z�b�g���邱�ƂŁA���̏��ƓV�䂪�ʂ�
					// �ꏊ�ɔz�u����Ȃ��悤�ɂ��܂��B
					lpMeshBuilder->Release();
					lpMeshBuilder = NULL;
					break;

				case 8:		// ���̕�
					if(!bGetWWallMesh(lpMeshBuilder))
						return FALSE;
					break;

				case 9:		// ���̕�
					if(!bGetEWallMesh(lpMeshBuilder))
						return FALSE;
					break;

				case 10:	// �k�̕�
					if(!bGetNWallMesh(lpMeshBuilder))
						return FALSE;
					break;

				case 11:	// ��̕�
					if(!bGetSWallMesh(lpMeshBuilder))
						return FALSE;
					break;

				case 14:	// �k���̃h�A
					if(!bGetNDoorMesh(lpMeshBuilder))
						return FALSE;

					// ���_��K�؂Ȉʒu�Ɉړ����܂�
					TRANSLATE();

					// ���b�V�����t���[���ɒǉ����܂�
					if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
						return FALSE;

					// ���̃��b�V���r���_��������A�V�����r���_���擾�����܂�
					lpMeshBuilder->Release();
					if(!bGetNSCorridorMesh(lpMeshBuilder))
						return FALSE;
	
					break;

				case 15:	// �쑤�̃h�A
					if(!bGetSDoorMesh(lpMeshBuilder))
						return FALSE;

					// ���_��K�؂Ȉʒu�Ɉړ����܂�
					TRANSLATE();

					// ���b�V�����t���[���ɒǉ����܂�
					if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
						return FALSE;

					// ���̃��b�V���r���_��������A�V�����r���_���擾�����܂�
					lpMeshBuilder->Release();
					if(!bGetNSCorridorMesh(lpMeshBuilder))
						return FALSE;
	
					break;

				case 16:	// �����̃h�A
					if(!bGetEDoorMesh(lpMeshBuilder))
						return FALSE;

					// ���_��K�؂Ȉʒu�Ɉړ����܂�
					TRANSLATE();

					// ���b�V�����t���[���ɒǉ����܂�
					if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
						return FALSE;

					// ���̃��b�V���r���_��������A�V�����r���_���擾�����܂�
					lpMeshBuilder->Release();
					if(!bGetEWCorridorMesh(lpMeshBuilder))
						return FALSE;
	
					break;

				case 17:	// �쓌�̊p�A�k�Ƀh�A
					if(!bGetNDoorMesh(lpMeshBuilder))
						return FALSE;

					// ���_��K�؂Ȉʒu�Ɉړ����܂�
					TRANSLATE();

					// ���b�V�����t���[���ɒǉ����܂�
					if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
						return FALSE;

					// ���̃��b�V���r���_��������A�V�����r���_���擾�����܂�
					lpMeshBuilder->Release();
					if(!bGetSECornerMesh(lpMeshBuilder))
						return FALSE;
	
					break;
	
				case 18:	// �k�Ƀh�A�A��ɕ�
					if(!bGetNDoorMesh(lpMeshBuilder))
						return FALSE;

					// ���_��K�؂Ȉʒu�Ɉړ����܂�
					TRANSLATE();

					// ���b�V�����t���[���ɒǉ����܂�
					if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
						return FALSE;

					// ���̃��b�V���r���_��������A�V�����r���_���擾�����܂�
					lpMeshBuilder->Release();
					if(!bGetSWallMesh(lpMeshBuilder))
						return FALSE;
	
					break;

				case 20:	// ���Ƀh�A�A���ɕ�
					if(!bGetEDoorMesh(lpMeshBuilder))
						return FALSE;

					// ���_��K�؂Ȉʒu�Ɉړ����܂�
					TRANSLATE();

					// ���b�V�����t���[���ɒǉ����܂�
					if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
						return FALSE;

					// ���̃��b�V���r���_��������A�V�����r���_���擾�����܂�
					lpMeshBuilder->Release();
					if(!bGetWWallMesh(lpMeshBuilder))
						return FALSE;
	
					break;


				case 21:	// ��Ƀh�A�A�ǂ͂Ȃ�
					if(!bGetSDoorMesh(lpMeshBuilder))
						return FALSE;
	
					break;


				case 22:	// �k�Ƀh�A�A�ǂ͂Ȃ�
					if(!bGetNDoorMesh(lpMeshBuilder))
						return FALSE;
	
					break;


				case 23:	// ���Ƀh�A�A�ǂ͂Ȃ�
					if(!bGetWDoorMesh(lpMeshBuilder))
						return FALSE;
	
					break;


				case 24:	// �k�Ɠ�Ƀh�A�A�ǂ͂Ȃ�
					if(!bGetNDoorMesh(lpMeshBuilder))
						return FALSE;

					// ���_��K�؂Ȉʒu�Ɉړ����܂�
					TRANSLATE();

					// ���b�V�����t���[���ɒǉ����܂�
					if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
						return FALSE;

					// ���̃��b�V���r���_��������A�V�����r���_���擾�����܂�
					lpMeshBuilder->Release();
					if(!bGetSDoorMesh(lpMeshBuilder))
						return FALSE;
	
					break;


				case 25:	// ����� (�s���~�܂�)
					if(!bGetNSCorridorMesh(lpMeshBuilder))
						return FALSE;

					// ���_��K�؂Ȉʒu�Ɉړ����܂�
					TRANSLATE();

					// ���b�V�����t���[���ɒǉ����܂�
					if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
						return FALSE;

					// ���̃��b�V���r���_��������A�V�����r���_���擾�����܂�
					lpMeshBuilder->Release();
					if(!bGetSWallMesh(lpMeshBuilder))
						return FALSE;
			
					break;


				default:	// �������Ȃ� (���S�̂��߁A���case��
							// ���Ă͂܂�Ȃ����̂������Ŗ������܂�)
					lpMeshBuilder = NULL;
				}

			
			// ���b�V�����쐬����Ă�����A�K�؂Ȉʒu�Ɉړ����A�t���[����
			// �ǉ����܂��B�����āA�V��Ə��������܂�
			if(lpMeshBuilder != NULL)
				{
				// ���_��K�؂Ȉʒu�Ɉړ����܂�
				TRANSLATE();
	
				// ���b�V�����t���[���ɒǉ����܂�
				if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
					return FALSE;
				
				lpMeshBuilder->Release();

				// ��
				if(!bGetFloorMesh(lpMeshBuilder))
					return FALSE;

				// ���_��K�؂Ȉʒu�Ɉړ����܂�
				TRANSLATE();
	
				// ���b�V�����t���[���ɒǉ����܂�
				if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
					return FALSE;
				
				lpMeshBuilder->Release();

				// �V��
				if(!bGetCeilingMesh(lpMeshBuilder))
					return FALSE;

				// ���_��K�؂Ȉʒu�Ɉړ����܂�
				TRANSLATE();
	
				// ���b�V�����t���[���ɒǉ����܂�
				if(FAILED(lpDungeonFrame->AddVisual(lpMeshBuilder)))
					return FALSE;
				
				lpMeshBuilder->Release();
				}

			lpMeshBuilder = NULL;
			}


	// ���݂��܂ރt���[�����_���W�����ɒǉ����܂�
	AddGoldFrames(lpDungeonFrame);

	// �V�[�����Ńt���[���̈ʒu��ݒ肵�܂�
	if(FAILED(lpD3DRMCamera->SetPosition(lpD3DRMScene, fGetXPos(), 1.5f, fGetZPos())))
		return FALSE;

	// ������ݒ肵�܂�
	if(FAILED(lpD3DRMCamera->SetOrientation(lpD3DRMScene, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f)))
		return FALSE;

	// �g���I������C���^�[�t�F�C�X��������܂�
	lpTxtFloor->Release();
	lpTxtCeiling->Release();
	lpTxtWall->Release();
	lpTxtGold->Release();

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// ���݂̂��鏰�̃��b�V�����쐬���܂�
///////////////////////////////////////////////////////////////////////////////
BOOL bGetGoldFloorMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
	{
	D3DVECTOR Vertices[] = { -3.0f, 0.1f, 3.0f,  3.0f, 0.1f, 3.0f,	
							  3.0f, 0.1f, -3.0f,  -3.0f, 0.1f, -3.0f };

	int	Faces[] = {4, 0, 1, 2, 3, 0};			

	// ���b�V���r���_���쐬���܂�
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder)))	return FALSE;

	// ���ׂĂ̖ʂ̐F�𔒂ɐݒ肵�܂�
	// ���ꂪ�e�N�X�`���Ƒg�ݍ��킳��܂�
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));
	
	// �z�񂩂烁�b�V�����쐬���܂�
	lpMeshBuilder->AddFaces( 4, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// �e�N�X�`����ݒ肵�܂�
	lpMeshBuilder->SetTexture(lpTxtGold);

	// ���̃e�N�X�`���̍��W��ݒ肵�܂�
	lpMeshBuilder->SetTextureCoordinates(0,0.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(1,0.f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(2,1.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(3,1.0f,0.0f);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// ���̃��b�V�����쐬���܂�
BOOL bGetFloorMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { -3.0f, 0.0f, 3.0f,  3.0f, 0.0f, 3.0f,	
							  3.0f, 0.0f, -3.0f,  -3.0f, 0.0f, -3.0f };

	int	Faces[] = {4, 0, 1, 2, 3, 0};			

	// ���b�V���r���_���쐬���܂�
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder)))	return FALSE;

	// ���ׂĂ̖ʂ̐F�𔒂ɐݒ肵�܂�
	// ���ꂪ�e�N�X�`���Ƒg�ݍ��킳��܂�
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));
	
	// �z�񂩂烁�b�V�����쐬���܂�
	lpMeshBuilder->AddFaces( 4, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// �e�N�X�`����ݒ肵�܂�
	lpMeshBuilder->SetTexture(lpTxtFloor);

	// ���̃e�N�X�`���̍��W��ݒ肵�܂�
	lpMeshBuilder->SetTextureCoordinates(0,0.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(1,0.f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(2,1.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(3,1.0f,0.0f);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// �V��̃��b�V�����쐬���܂�
BOOL bGetCeilingMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { -3.0f, 3.0f, 3.0f,  -3.0f, 3.0f, -3.0f,	
							  3.0f, 3.0f, -3.0f, 3.0f, 3.0f, 3.0f };

	int	Faces[] = {4, 0, 1, 2, 3, 0};

	// ���b�V���r���_���쐬���܂�
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder)))
		return FALSE;

	// ���ׂĂ̖ʂ̐F�𔒂ɐݒ肵�܂�
	// ���ꂪ�e�N�X�`���Ƒg�ݍ��킳��܂�
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));
	
	// �z�񂩂烁�b�V�����쐬���܂�
	lpMeshBuilder->AddFaces( 4, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// �e�N�X�`����ݒ肵�܂�
	lpMeshBuilder->SetTexture(lpTxtCeiling);

	// �V��̃e�N�X�`�����쐬���܂�
	lpMeshBuilder->SetTextureCoordinates(0,0.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(1,0.f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(2,1.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(3,1.0f,0.0f);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// ��k�̉�L�̃��b�V�����쐬���܂�
BOOL bGetNSCorridorMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { -3.0f, 0.0f, -3.0f, -3.0f, 3.0f, -3.0f,	// �����̕�
							 -3.0f, 3.0f, 3.0f, -3.0f, 0.0f,  3.0f,	

							 3.0f, 0.0f,  3.0f, 3.0f, 3.0f,  3.0f,	// �����̕�
							 3.0f, 3.0f, -3.0f, 3.0f, 0.0f, -3.0f,	};

	int Faces[] = 	{4, 0, 1, 2, 3,	// ���̓���
		  	4, 4, 5, 6, 7,	// ���̓���
			0};		// �I���������܂�

	// ���b�V���r���_���쐬���܂�
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder))) return FALSE;

	// ���ׂĂ̕ǖʂ̐F�𔒂ɐݒ肵�܂�
	// ���ꂪ�e�N�X�`���Ƒg�ݍ��킳��܂�
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));
	
	// �z�񂩂烁�b�V�����쐬���܂�
	lpMeshBuilder->AddFaces( 8, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// �e�N�X�`����ݒ肵�܂�
	lpMeshBuilder->SetTexture(lpTxtWall);
	
	// �e�N�X�`�����W��ݒ肵�܂�
	// �e�ʂɂ�4�̒��_������A�����v�����ɔz�u����܂�
	// �P���ȃ��[�v�̓e�N�X�`�����W�����ׂē����l�ɐݒ肵�܂�
	// ���̍��W�͕ǖʂ̃p�l���̍��W�ł�
	for(int i = 0; i < 8; i+= 4){
		lpMeshBuilder->SetTextureCoordinates(i,0.0f,0.0f);
		lpMeshBuilder->SetTextureCoordinates(i+1,1.0f,0.0f);
		lpMeshBuilder->SetTextureCoordinates(i+2,1.0f,1.0f);
		lpMeshBuilder->SetTextureCoordinates(i+3,0.0f,1.0f);
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// �����̉�L�̃��b�V�����쐬���܂�
BOOL bGetEWCorridorMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { -3.0f, 0.0f, 3.0f,	-3.0f, 3.0f, 3.0f,		// �k�̕�
							 3.0f, 3.0f, 3.0f,	 3.0f, 0.0f,  3.0f,	

							 3.0f, 0.0f,  -3.0f,  3.0f, 3.0f,  -3.0f,	// ��̕�
							-3.0f, 3.0f, -3.0f,	-3.0f, 0.0f, -3.0f,	};

	int Faces[] = {4, 0, 1, 2, 3,	// �����̍� 
	 			   4, 4, 5, 6, 7,	// �����̉E
				   0};				// �I���������܂�


	// ���b�V���r���_���쐬���܂�
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder)))	return FALSE;

	// ���ׂĂ̕ǖʂ̐F�𔒂ɐݒ肵�܂�
	// ���ꂪ�e�N�X�`���Ƒg�ݍ��킳��܂�
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));

	// �z�񂩂烁�b�V�����쐬���܂�
	lpMeshBuilder->AddFaces( 8, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// �e�N�X�`����ݒ肵�܂�
	lpMeshBuilder->SetTexture(lpTxtWall);

	// �e�N�X�`�����W��ݒ肵�܂�
	// �e�ʂɂ�4�̒��_������A�����v�����ɔz�u����܂�
	// �P���ȃ��[�v�̓e�N�X�`�����W�����ׂē����l�ɐݒ肵�܂�
	for(int i = 0; i < 8; i+= 4){
		lpMeshBuilder->SetTextureCoordinates(i,0.0f,0.0f);
		lpMeshBuilder->SetTextureCoordinates(i+1,1.0f,0.0f);
		lpMeshBuilder->SetTextureCoordinates(i+2,1.0f,1.0f);
		lpMeshBuilder->SetTextureCoordinates(i+3,0.0f,1.0f);
	}

	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// �k���̊p�̃��b�V�����쐬���܂�
BOOL bGetNWCornerMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
	{
	D3DVECTOR Vertices[] = { -3.0f, 0.0f, 3.0f,	-3.0f, 3.0f, 3.0f,		// �k�̕�
							 3.0f, 3.0f, 3.0f,	 3.0f, 0.0f,  3.0f,	

							-3.0f, 0.0f,  -3.0f,  -3.0f, 3.0f, -3.0f,	// ���̕�
							-3.0f, 3.0f,  3.0f,	-3.0f, 0.0f, 3.0f,	};

	int Faces[] = {4, 0, 1, 2, 3,	// �����̍�
	 			   4, 4, 5, 6, 7,	// �����̉E
				   0};				// �I���������܂�


	// ���b�V���r���_���쐬���܂�
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder)))	return FALSE;

	// ���ׂĂ̕ǖʂ̐F�𔒂ɐݒ肵�܂�
	// ���ꂪ�e�N�X�`���Ƒg�ݍ��킳��܂�
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));

	// �z�񂩂烁�b�V�����쐬���܂�
	lpMeshBuilder->AddFaces( 8, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// �e�N�X�`����ݒ肵�܂�
	lpMeshBuilder->SetTexture(lpTxtWall);

	// �e�N�X�`�����W��ݒ肵�܂�
	// �e�ʂɂ�4�̒��_������A�����v�����ɔz�u����܂�
	// �P���ȃ��[�v�̓e�N�X�`�����W�����ׂē����l�ɐݒ肵�܂�
	for(int i = 0; i < 8; i+= 4){
		lpMeshBuilder->SetTextureCoordinates(i,0.0f,0.0f);
		lpMeshBuilder->SetTextureCoordinates(i+1,1.0f,0.0f);
		lpMeshBuilder->SetTextureCoordinates(i+2,1.0f,1.0f);
		lpMeshBuilder->SetTextureCoordinates(i+3,0.0f,1.0f);
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// �쐼�̊p�̃��b�V�����쐬���܂�
BOOL bGetSWCornerMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { 3.0f, 0.0f, -3.0f,	3.0f, 3.0f, -3.0f,		// ��̕�
							-3.0f, 3.0f, -3.0f,	-3.0f, 0.0f, -3.0f,	

							-3.0f, 0.0f,  -3.0f,  -3.0f, 3.0f, -3.0f,	// ���̕�
							-3.0f, 3.0f,  3.0f,	-3.0f, 0.0f, 3.0f,	};

	int Faces[] = {4, 0, 1, 2, 3,	// �����̍�
	 			   4, 4, 5, 6, 7,	// �����̉E
				   0};				// �I���������܂�

	// ���b�V���r���_���쐬���܂�
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder)))
		return FALSE;

	// ���ׂĂ̕ǖʂ̐F�𔒂ɐݒ肵�܂�
	// ���ꂪ�e�N�X�`���Ƒg�ݍ��킳��܂�
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));

	// �z�񂩂烁�b�V�����쐬���܂�
	lpMeshBuilder->AddFaces( 8, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// �e�N�X�`����ݒ肵�܂�
	lpMeshBuilder->SetTexture(lpTxtWall);

	// �e�N�X�`�����W��ݒ肵�܂�
	// �e�ʂɂ�4�̒��_������A�����v�����ɔz�u����܂�
	// �P���ȃ��[�v�̓e�N�X�`�����W�����ׂē����l�ɐݒ肵�܂�
	for(int i = 0; i < 8; i+= 4){
		lpMeshBuilder->SetTextureCoordinates(i,0.0f,0.0f);
		lpMeshBuilder->SetTextureCoordinates(i+1,1.0f,0.0f);
		lpMeshBuilder->SetTextureCoordinates(i+2,1.0f,1.0f);
		lpMeshBuilder->SetTextureCoordinates(i+3,0.0f,1.0f);
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// �k���̊p�̃��b�V�����쐬���܂�
BOOL bGetNECornerMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { -3.0f, 0.0f, 3.0f,	-3.0f, 3.0f, 3.0f,		// �k�̕�
							 3.0f, 3.0f, 3.0f,	 3.0f, 0.0f,  3.0f,	

							 3.0f, 0.0f,  3.0f,  3.0f, 3.0f,  3.0f,		// ���̕�
							 3.0f, 3.0f, -3.0f,	3.0f, 0.0f, -3.0f,	};

	int Faces[] = {4, 0, 1, 2, 3,	// �����̍�
	 			   4, 4, 5, 6, 7,	// �����̉E
				   0};				// �I���������܂�


	// ���b�V���r���_���쐬���܂�
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder)))	return FALSE;

	// ���ׂĂ̕ǖʂ̐F�𔒂ɐݒ肵�܂�
	// ���ꂪ�e�N�X�`���Ƒg�ݍ��킳��܂�
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));

	// �z�񂩂烁�b�V�����쐬���܂�
	lpMeshBuilder->AddFaces(	8, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// �e�N�X�`����ݒ肵�܂�
	lpMeshBuilder->SetTexture(lpTxtWall);

	// �e�N�X�`�����W��ݒ肵�܂�
	// �e�ʂɂ�4�̒��_������A�����v�����ɔz�u����܂�
	// �P���ȃ��[�v�̓e�N�X�`�����W�����ׂē����l�ɐݒ肵�܂�
	for(int i = 0; i < 8; i+= 4){
		lpMeshBuilder->SetTextureCoordinates(i,0.0f,0.0f);
		lpMeshBuilder->SetTextureCoordinates(i+1,1.0f,0.0f);
		lpMeshBuilder->SetTextureCoordinates(i+2,1.0f,1.0f);
		lpMeshBuilder->SetTextureCoordinates(i+3,0.0f,1.0f);
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// �쓌�̊p�̃��b�V�����쐬���܂�
BOOL bGetSECornerMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = {	3.0f, 0.0f, -3.0f,	3.0f, 3.0f, -3.0f,		// ��̕�
								-3.0f, 3.0f, -3.0f,	-3.0f, 0.0f, -3.0f,	

								3.0f, 0.0f,  3.0f,  3.0f, 3.0f,  3.0f,		// ���̕�
								3.0f, 3.0f, -3.0f,	3.0f, 0.0f, -3.0f,	};

	int Faces[] = {4, 0, 1, 2, 3,	// �����̍�
	 			   4, 4, 5, 6, 7,	// �����̉E
				   0};				// �I���������܂�


	// ���b�V���r���_���쐬���܂�
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder)))
		return FALSE;

	// ���ׂĂ̕ǖʂ̐F�𔒂ɐݒ肵�܂�
	// ���ꂪ�e�N�X�`���Ƒg�ݍ��킳��܂�
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));

	// �z�񂩂烁�b�V�����쐬���܂�
	lpMeshBuilder->AddFaces( 8, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// �e�N�X�`����ݒ肵�܂�
	lpMeshBuilder->SetTexture(lpTxtWall);

	// �e�N�X�`�����W��ݒ肵�܂�
	// �e�ʂɂ�4�̒��_������A�����v�����ɔz�u����܂�
	// �P���ȃ��[�v�̓e�N�X�`�����W�����ׂē����l�ɐݒ肵�܂�
	for(int i = 0; i < 8; i+= 4){
		lpMeshBuilder->SetTextureCoordinates(i,0.0f,0.0f);
		lpMeshBuilder->SetTextureCoordinates(i+1,1.0f,0.0f);
		lpMeshBuilder->SetTextureCoordinates(i+2,1.0f,1.0f);
		lpMeshBuilder->SetTextureCoordinates(i+3,0.0f,1.0f);
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// �����̕ǂ̃��b�V�����쐬���܂�
BOOL bGetWWallMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { -3.0f, 0.0f,  -3.0f,  -3.0f, 3.0f, -3.0f,	// ���̕�
								-3.0f, 3.0f,  3.0f,	-3.0f, 0.0f, 3.0f  };

	int	Faces[] = {4, 0, 1, 2, 3, 0};

	// ���b�V���r���_���쐬���܂�
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder)))
		return FALSE;

	// ���ׂĂ̕ǖʂ̐F�𔒂ɐݒ肵�܂�
	// ���ꂪ�e�N�X�`���Ƒg�ݍ��킳��܂�
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));
	
	// �z�񂩂烁�b�V�����쐬���܂�
	lpMeshBuilder->AddFaces( 4, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// �e�N�X�`����ݒ肵�܂�
	lpMeshBuilder->SetTexture(lpTxtWall);

	// �e�N�X�`�����W��ݒ肵�܂�
	lpMeshBuilder->SetTextureCoordinates(0,0.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(1,0.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(2,1.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(3,1.0f,0.0f);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// ���̕ǂ̃��b�V�����쐬���܂�
BOOL bGetEWallMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { 3.0f, 0.0f,  3.0f,  3.0f, 3.0f,  3.0f,	// ���̕�
								3.0f, 3.0f, -3.0f,	3.0f, 0.0f, -3.0f};

	int	Faces[] = {4, 0, 1, 2, 3, 0};

	// ���b�V���r���_���쐬���܂�
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder)))
		return FALSE;

	// ���ׂĂ̕ǖʂ̐F�𔒂ɐݒ肵�܂�
	// ���ꂪ�e�N�X�`���Ƒg�ݍ��킳��܂�
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));
	
	// �z�񂩂烁�b�V�����쐬���܂�
	lpMeshBuilder->AddFaces( 4, Vertices, 
							0, NULL, 
							(unsigned long *)Faces, NULL);

	// �e�N�X�`����ݒ肵�܂�
	lpMeshBuilder->SetTexture(lpTxtWall);

	// �e�N�X�`�����W��ݒ肵�܂�
	lpMeshBuilder->SetTextureCoordinates(0,0.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(1,0.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(2,1.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(3,1.0f,0.0f);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// �k�̕ǂ̃��b�V�����쐬���܂�
BOOL bGetNWallMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
	{
	D3DVECTOR Vertices[] = { -3.0f, 0.0f, 3.0f,	-3.0f, 3.0f, 3.0f,		// �k�̕�
								3.0f, 3.0f, 3.0f,	 3.0f, 0.0f,  3.0f};

	int	Faces[] = {4, 0, 1, 2, 3, 0};

	// ���b�V���r���_���쐬���܂�
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder))) return FALSE;

	// ���ׂĂ̕ǖʂ̐F�𔒂ɐݒ肵�܂�
	// ���ꂪ�e�N�X�`���Ƒg�ݍ��킳��܂�
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));
	
	// �z�񂩂烁�b�V�����쐬���܂�
	lpMeshBuilder->AddFaces( 4, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// �e�N�X�`����ݒ肵�܂�
	lpMeshBuilder->SetTexture(lpTxtWall);

	// �e�N�X�`�����W��ݒ肵�܂�
	lpMeshBuilder->SetTextureCoordinates(0,0.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(1,0.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(2,1.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(3,1.0f,0.0f);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// ��̕ǂ̃��b�V�����쐬���܂�
BOOL bGetSWallMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { 3.0f, 0.0f, -3.0f,	3.0f, 3.0f, -3.0f,		// ��̕�
							-3.0f, 3.0f, -3.0f,	-3.0f, 0.0f, -3.0f};

	int	Faces[] = {4, 0, 1, 2, 3, 0};

	// ���b�V���r���_���쐬���܂�
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder))) return FALSE;

	// ���ׂĂ̕ǖʂ̐F�𔒂ɐݒ肵�܂�
	// ���ꂪ�e�N�X�`���Ƒg�ݍ��킳��܂�
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));
	
	// �z�񂩂烁�b�V�����쐬���܂�
	lpMeshBuilder->AddFaces( 4, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// �e�N�X�`����ݒ肵�܂�
	lpMeshBuilder->SetTexture(lpTxtWall);

	// �e�N�X�`�����W��ݒ肵�܂�
	lpMeshBuilder->SetTextureCoordinates(0,0.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(1,0.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(2,1.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(3,1.0f,0.0f);

	return TRUE;
}



///////////////////////////////////////////////////////////////////////////////
// �k���̃h�A�̃��b�V�����쐬���܂�
BOOL bGetNDoorMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { -3.0f, 0.0f, 3.0f,	-3.0f, 3.0f, 3.0f,		// �k�̃h�A
								3.0f, 3.0f, 3.0f,	 3.0f, 0.0f,  3.0f};

	int	Faces[] = {4, 0, 1, 2, 3, 0};

	// ���b�V���r���_���쐬���܂�
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder)))
		return FALSE;

	// ���ׂĂ̕ǖʂ̐F�𔒂ɐݒ肵�܂�
	// ���ꂪ�e�N�X�`���Ƒg�ݍ��킳��܂�
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));
	
	// �z�񂩂烁�b�V�����쐬���܂�
	lpMeshBuilder->AddFaces( 4, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// �e�N�X�`����ݒ肵�܂�
	lpMeshBuilder->SetTexture(lpTxtWood);

	// �e�N�X�`�����W��ݒ肵�܂�
	lpMeshBuilder->SetTextureCoordinates(0,0.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(1,1.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(2,1.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(3,0.0f,1.0f);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// �쑤�̃h�A�̃��b�V�����쐬���܂�
BOOL bGetSDoorMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { 3.0f, 0.0f, -3.0f,	3.0f, 3.0f, -3.0f,		// ��̃h�A
							-3.0f, 3.0f, -3.0f,	-3.0f, 0.0f, -3.0f};

	int	Faces[] = {4, 0, 1, 2, 3, 0};

	// ���b�V���r���_���쐬���܂�
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder))) return FALSE;

	// ���ׂĂ̕ǖʂ̐F�𔒂ɐݒ肵�܂�
	// ���ꂪ�e�N�X�`���Ƒg�ݍ��킳��܂�
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));
	
	// �z�񂩂烁�b�V�����쐬���܂�
	lpMeshBuilder->AddFaces( 4, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// �e�N�X�`����ݒ肵�܂�
	lpMeshBuilder->SetTexture(lpTxtWood);

	// �e�N�X�`�����W��ݒ肵�܂�
  	lpMeshBuilder->SetTextureCoordinates(0,0.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(1,1.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(2,1.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(3,0.0f,1.0f);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// �����̃h�A�̃��b�V�����쐬���܂�
BOOL bGetEDoorMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { 3.0f, 0.0f,  3.0f,  3.0f, 3.0f,  3.0f,	// ���̃h�A
								3.0f, 3.0f, -3.0f,	3.0f, 0.0f, -3.0f};

	int	Faces[] = {4, 0, 1, 2, 3, 0};

	// ���b�V���r���_���쐬���܂�
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder))) return FALSE;

	// ���ׂĂ̕ǖʂ̐F�𔒂ɐݒ肵�܂�
	// ���ꂪ�e�N�X�`���Ƒg�ݍ��킳��܂�
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));
	
	// �z�񂩂烁�b�V�����쐬���܂�
	lpMeshBuilder->AddFaces( 4, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// �e�N�X�`����ݒ肵�܂�
	lpMeshBuilder->SetTexture(lpTxtWood);

	// �e�N�X�`�����W��ݒ肵�܂�
  	lpMeshBuilder->SetTextureCoordinates(0,0.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(1,1.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(2,1.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(3,0.0f,1.0f);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// �����̃h�A�̃��b�V�����쐬���܂�
BOOL bGetWDoorMesh(LPDIRECT3DRMMESHBUILDER& lpMeshBuilder)
{
	D3DVECTOR Vertices[] = { -3.0f, 0.0f,  -3.0f,  -3.0f, 3.0f, -3.0f,	// ���̃h�A
							-3.0f, 3.0f,  3.0f,	-3.0f, 0.0f, 3.0f  };


	int	Faces[] = {4, 0, 1, 2, 3, 0};

	// ���b�V���r���_���쐬���܂�
	if(FAILED(lpDirect3DRM->CreateMeshBuilder(&lpMeshBuilder))) return FALSE;

	// ���ׂĂ̕ǖʂ̐F�𔒂ɐݒ肵�܂�
	// ���ꂪ�e�N�X�`���Ƒg�ݍ��킳��܂�
	lpMeshBuilder->SetColor(D3DRGB(1, 1, 1));
	
	// �z�񂩂烁�b�V�����쐬���܂�
	lpMeshBuilder->AddFaces( 4, Vertices, 0, NULL, (unsigned long *)Faces, NULL);

	// �e�N�X�`����ݒ肵�܂�
	lpMeshBuilder->SetTexture(lpTxtWood);

	// �e�N�X�`�����W��ݒ肵�܂�
	lpMeshBuilder->SetTextureCoordinates(0,0.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(1,1.0f,0.0f);
	lpMeshBuilder->SetTextureCoordinates(2,1.0f,1.0f);
	lpMeshBuilder->SetTextureCoordinates(3,0.0f,1.0f);

	return TRUE;
}

//�G���[�n���h����COM�I�u�W�F�N�g�̉��
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
