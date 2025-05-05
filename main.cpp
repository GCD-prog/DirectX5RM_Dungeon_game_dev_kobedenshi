
/////////////////////////////////////////////////////////////////////////////////////////////////
// �ꕔ���p����(nav.h, nav.cpp, Scene.cpp, sound.h/cpp dsound.h/cpp ) �Q�l�v���O�������Ђ̃R�[�h
// DirectX5�Q�[���v���O���~���O����  �o�Ŏ�: �C���v���X (1998/3/1)
// �W�F�[�\�� �R���u (��), Jason Kolb (����), �A�N���o�C�g (�|��)
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

//Direct3D �֘A
LPDIRECT3D lpDirect3D = NULL;
LPDIRECT3DDEVICE lpD3DDevice = NULL;

//Direct3D RM �֘A
LPDIRECT3DRM lpDirect3DRM = NULL;
LPDIRECT3DRMDEVICE lpD3DRMDevice = NULL;
LPDIRECT3DRMVIEWPORT lpD3DRMView = NULL;
LPDIRECT3DRMFRAME lpD3DRMScene = NULL;
LPDIRECT3DRMFRAME lpD3DRMCamera = NULL;
*/

char szDevice[128], szDDDeviceName[128] = "default";

#define MAX_DRIVERS 5                   // ���o����h���C�o�̍ő吔
static GUID DriverGUID[MAX_DRIVERS];     // �h���C�o��GUID���i�[����z��
static char DriverName[MAX_DRIVERS][50]; // �h���C�o�����i�[����z��
static int  NumDrivers = 0;             // ���o���ꂽ�h���C�o�̐� (�����l0)
static int  CurrDriver = -1;            // ���ݑI������Ă���h���C�o�̃C���f�b�N�X (�����l-1)
static int  BPP = 16;                   // ��ʂ̐F�[�x (16bit�ɐݒ�)

int nScore = 0;			// ���_
int nPossible = 10;		// �\�ȍō��_

void FrameCnt(void)
{
		static int cnt;
		static DWORD Nowtime,Prevtime;
		static char text0[10];		// FPS�J�E���g��
		static char text1[10];		// nScore ���_
		static char text2[10];		// nPossible �\�ȍō��_
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

		wsprintf(text1,"%d ���_", nScore);
		wsprintf(text2,"%d �ō��_", nPossible);

		// �f�o�C�X���\���p�̕�������쐬
		sprintf(text_d3d_device, "D3D Dev: %s", szDevice);
		sprintf(text_dd_device, "DD Dev : [%s]", szDDDeviceName);

		// �J�����̂ݕ\������
		lpBackbuffer->GetDC(&hdc);
		TextOut(hdc, 0, 0, text0, strlen(text0));		// FPS�J�E���g��
		TextOut(hdc, 50, 0, text1, strlen(text1));		// nScore ���_
		TextOut(hdc, 100, 0, text2, strlen(text2));	// nPossible �\�ȍō��_

		TextOut(hdc, 0, 80, text_d3d_device, strlen(text_d3d_device));   // Direct3D�f�o�C�X��
		TextOut(hdc, 0, 100, text_dd_device, strlen(text_dd_device));    // DirectDraw�f�o�C�X��

		lpBackbuffer->ReleaseDC(hdc);
}

void KeyControl(void)
{
	// ��������������

	// ���̃v���O�����̓��������A�����̈ʒu�ł͎g��Ȃ��B
	// �v���[���[�̈ʒu�ɍ��킹�ăJ�����̈ʒu���X�V���܂�
	// lpD3DRMCamera->AddRotation(D3DRMCOMBINE_REPLACE, 0.0f, 1.0f, 0.0f,fGetAngle());
	// lpD3DRMCamera->SetPosition(lpD3DRMScene, fGetXPos(), 1.5f, fGetZPos());
	// ���̍��W�ʒu�̕ϐ��̒l���Ƃ�Ă��Ȃ�����ړ����Ȃ��̂ł́H

	// ������
	if( (GetAsyncKeyState(VK_LEFT)&0x8000) || (GetAsyncKeyState('Q')&0x8000) ){
			TurnLeft();
	}

	// �E����
	if( (GetAsyncKeyState(VK_RIGHT)&0x8000) || (GetAsyncKeyState('E')&0x8000) ){
			TurnRight();
	}

	// �O�i
	if(GetAsyncKeyState('W')&0x8000){
		if(GetKeyState(VK_SHIFT) & 0x8000){		// Shift�L�[�ō�����
			RunForward();
		}else{
			StepForward();
		}
	}

	// ���
	if(GetAsyncKeyState('S')&0x8000){
		if(GetKeyState(VK_SHIFT) & 0x8000){		// Shift�L�[�ō�����
			RunBackward();
		}else{
			StepBackward();
		}
	}

	// ��
	if(GetAsyncKeyState('A')&0x8000){
		if(GetKeyState(VK_SHIFT) & 0x8000){		// Shift�L�[�ō�����
			SwingLeft();
		}else{
			TurnLeft();
		}
	}

	// �E
	if(GetAsyncKeyState('D')&0x8000){
		if(GetKeyState(VK_SHIFT) & 0x8000){		// Shift�L�[�ō�����
			SwingRight();
		}else{
			TurnRight();
		}
	}

	// �v���[���[�̈ʒu�ɍ��킹�ăJ�����̈ʒu���X�V���܂� �����ɔz�u lpD3DRMCamare
	lpD3DRMCamera->AddRotation(D3DRMCOMBINE_REPLACE, 0.0f, 1.0f, 0.0f,fGetAngle());
	lpD3DRMCamera->SetPosition(lpD3DRMScene, fGetXPos(), 1.5f, fGetZPos());

}

BOOL RenderFrame(void)
{

	if ( lpPrimary->IsLost() == DDERR_SURFACELOST )		lpPrimary->Restore();

		RECT Scrrc={0, 0, 640, 480};   // ��ʂ̃T�C�Y

		// �b�ԂU�O�t���[�����z���Ȃ��悤�ɂ���
		static DWORD nowTime, prevTime;
		nowTime = timeGetTime();
		if( (nowTime - prevTime) < 1000 / 60 ) return 0;
		prevTime = nowTime;

		// �L�[����
		KeyControl();

		// Direct3DRM �����_�����O����
		lpD3DRMScene->Move(D3DVAL(1.0)); 
		lpD3DRMView->Clear();

		// ���݂̃V�[�����o�b�N�o�b�t�@�ōĕ`�悵�܂��B����́A
		// ���݂̃T�[�t�F�C�X��GDI�T�[�t�F�C�X�łȂ��ꍇ�̏����ł�
		// ���̏ꍇ�A�������ăV�[�����ĕ`�悷��ƁA�X�R�A��\������O��
		// �V�[����1�t���[�������߂�̂�h���܂�
		lpD3DRMView->Render(lpD3DRMScene);  // �V�[���̃����_�����O
		lpD3DRMDevice->Update();              // �f�o�C�X�i�E�B���h�E�j�̍X�V

		//FPS�l�v��
		FrameCnt();

		// Flip�֐�
		lpPrimary->Flip(NULL, DDFLIP_WAIT);

		return TRUE;
}

//-------------------------------------------
// Name: BPPToDDBD()
// BPP(bits per pixel)��DirectDraw�̃r�b�g�[�x�t���O�ɕϊ�
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
// Desc: Direct3D�f�o�C�X��񋓂��邽�߂̃R�[���o�b�N�֐�
//       �n�[�h�E�F�ARGB > �n�[�h�E�F�AMono > HEL RGB > HEL Mono �̏��ŗD��
//       (�O�����Z�q�� if/else �ɏ�������)
//-------------------------------------------
static HRESULT WINAPI enumDeviceFunc(LPGUID lpGuid, LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC lpHWDesc, LPD3DDEVICEDESC lpHELDesc, LPVOID lpContext)
{

	LPD3DDEVICEDESC lpDesc = NULL;
	bool isHardware = false;
	bool isMono = false;
	int* lpCurrentDriverIndex = (int*)lpContext; // ���ݑI�𒆂̃h���C�o�C���f�b�N�X

	// �f�o�C�X�L�q�q�̑I�� (�n�[�h�E�F�A�D��)
	if (lpHWDesc && lpHWDesc->dcmColorModel != 0) {
		lpDesc = lpHWDesc;
		isHardware = true;
	} else if (lpHELDesc && lpHELDesc->dcmColorModel != 0) {
		lpDesc = lpHELDesc;
		isHardware = false;
	} else {
		// �L���ȃf�o�C�X�L�q�q���Ȃ��ꍇ�̓X�L�b�v
		return D3DENUMRET_OK;
	}

	// �w�肵���F�[�x(BPP)�ɑΉ����Ă��邩�`�F�b�N
	if (!(lpDesc->dwDeviceRenderBitDepth & BPPToDDBD(BPP))) {
		return D3DENUMRET_OK;
	}

	// �h���C�o����z��ɋL�^
	if (NumDrivers < MAX_DRIVERS) {
		memcpy(&DriverGUID[NumDrivers], lpGuid, sizeof(GUID));
		strncpy(DriverName[NumDrivers], lpDeviceDescription, 49);
		DriverName[NumDrivers][49] = '\0'; // NULL�I�[�ۏ�

		// �D�揇�ʂɊ�Â��� CurrDriver ���X�V
		isMono = (lpDesc->dcmColorModel & D3DCOLOR_MONO);
		int currentPriority = 0; // 0: HEL Mono, 1: HEL RGB, 2: HW Mono, 3: HW RGB

		// currentPriority �v�Z
		if (isHardware) {
			// �n�[�h�E�F�A�̏ꍇ
			if (isMono) {
				currentPriority = 2; // HW Mono
			} else {
				currentPriority = 3; // HW RGB
			}
		} else {
			// �\�t�g�E�F�A (HEL) �̏ꍇ
			if (isMono) {
				currentPriority = 0; // HEL Mono
			} else {
				currentPriority = 1; // HEL RGB
			}
		}

		int selectedPriority = -1; // �I���ς݃h���C�o�̗D��x (-1 �͖��I��)
		if (*lpCurrentDriverIndex != -1) {
			// ���ɑI������Ă���h���C�o�̗D��x�� DriverName ���琄��
			bool selectedIsHardware = false;
			bool selectedIsMono = false;

			// "(HW)" �܂��� "Hardware" ���܂܂�Ă���΃n�[�h�E�F�A�Ƃ݂Ȃ�
			if (strstr(DriverName[*lpCurrentDriverIndex], "(HW)") || strstr(DriverName[*lpCurrentDriverIndex], "Hardware")) {
				selectedIsHardware = true;
			}
			// "Mono" ���܂܂�Ă���� Mono �Ƃ݂Ȃ�
			if (strstr(DriverName[*lpCurrentDriverIndex], "Mono")) {
				selectedIsMono = true;
			}

			// selectedPriority �v�Z
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

		// ���D��x�̍����h���C�o������������X�V
		if (currentPriority > selectedPriority) {
			*lpCurrentDriverIndex = NumDrivers;
		}

		NumDrivers++; // ���������h���C�o���𑝂₷
		if (NumDrivers == MAX_DRIVERS) {
			return D3DENUMRET_CANCEL; // �z�񂪈�t�ɂȂ�����I��
		}
	}

	return D3DENUMRET_OK;

}

//-------------------------------------------
// Name: EnumDrivers()
// Desc: �g�p�\��Direct3D�h���C�o��񋓂��A�œK�Ȃ��̂�I������
//-------------------------------------------
static BOOL EnumDrivers()
{

	LPDIRECTDRAW lpDD_enum = NULL; // �񋓗p�̈ꎞ�I��DirectDraw�I�u�W�F�N�g
	LPDIRECT3D   lpD3D_enum = NULL; // �񋓗p�̈ꎞ�I��Direct3D�I�u�W�F�N�g
	HRESULT hr;

	// �ꎞ�I��DirectDraw�I�u�W�F�N�g���쐬
	hr = DirectDrawCreate(NULL, &lpDD_enum, NULL);
	if (FAILED(hr)) {
		MessageBox(NULL, "�h���C�o�񋓗p DirectDraw �쐬���s", "�������G���[", MB_OK);
		return FALSE;
	}

	// �ꎞ�I��Direct3D�C���^�[�t�F�[�X���擾
	hr = lpDD_enum->QueryInterface(IID_IDirect3D, (void**)&lpD3D_enum);
	if (FAILED(hr)) {
		MessageBox(NULL, "�h���C�o�񋓗p Direct3D �擾���s", "�������G���[", MB_OK);
		lpDD_enum->Release();
		return FALSE;
	}

	// �h���C�o��񋓂��čœK�Ȃ��̂�I�� (���ʂ̓O���[�o���ϐ� CurrDriver �Ɋi�[)
	NumDrivers = 0; // �h���C�o�������Z�b�g
	CurrDriver = -1; // �I�𒆃h���C�o�����Z�b�g
	hr = lpD3D_enum->EnumDevices(enumDeviceFunc, &CurrDriver);
	if (FAILED(hr)) {
		MessageBox(NULL, "Direct3D �f�o�C�X�񋓎��s", "�������G���[", MB_OK);
	}

	// �ꎞ�I�u�W�F�N�g�����
	lpD3D_enum->Release();
	lpDD_enum->Release();

	// �h���C�o��������Ȃ��������A�I������Ȃ������ꍇ
	if (NumDrivers == 0 || CurrDriver == -1) {
		MessageBox(NULL, "�g�p�\��Direct3D�h���C�o��������܂���B", "�������G���[", MB_OK);
		return FALSE;
	}

	// �f�o�b�O�o�͂Ɏg�p����h���C�o����\��
	char log[100];
	sprintf(log, "�I�����ꂽ�h���C�o: %d - %s\n", CurrDriver, DriverName[CurrDriver]);
	OutputDebugString(log);

	return TRUE;

}

LRESULT APIENTRY WndFunc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch(msg){

	case WM_CREATE:

		break;

	case WM_KEYDOWN:

		// Esc�L�[�Ńv���O�������I�����܂�
		if(wParam == VK_ESCAPE){

			// dsound.cpp
			ShutdownDirectSound();
			// sound.cpp
			StopBackgroundMusic();

			//��ʃ��[�h�����ɖ߂�
			lpDirectDraw->SetCooperativeLevel(hwnd, DDSCL_NORMAL);
			lpDirectDraw->RestoreDisplayMode();

			lpScreen->Release();

			ReleaseAll(); //�e�I�u�W�F�N�g��ReleaseAll()�ŉ������

			PostQuitMessage(0);

		}

		break;

	case WM_DESTROY:

			// dsound.cpp
			ShutdownDirectSound();

			// sound.cpp midi_file
			StopBackgroundMusic();

			lpScreen->Release();

			ReleaseAll(); //�e�I�u�W�F�N�g��ReleaseAll()�ŉ������

			PostQuitMessage(0);

		break;

	// sound.cpp midi_file
	// BGM����~����ƁA���̂��Ƃ��ʒm����܂�
	// "play"�R�}���h���Ď��s���܂�
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

		ShowCursor(FALSE); //�J�[�\�����B��

		//Direct3DRM�̍\�z
		Direct3DRMCreate(&lpDirect3DRM);

		//DirectDrawClipper�̍\�z
		DirectDrawCreateClipper(0, &lpDDClipper, NULL);


		// --- DirectDraw�I�u�W�F�N�g�̍쐬 (��ʃ��[�h�ݒ蓙�̂��߂ɐ�ɍ쐬) ---
		// �f�t�H���g��DirectDraw�h���C�o���g�p (NULL ���w��)
		if(FAILED(DirectDrawCreate(NULL, &lpDirectDraw, NULL))) {
			MessageBox(hwnd, "DirectDrawCreate Fail", "Error", MB_OK);
			ReleaseAll(); return FALSE;
		}
		// �Ƃ肠����DirectDraw�h���C�o�����Z�b�g (EnumDrivers�̌��ʂƂ͕�)
		lstrcpy(szDDDeviceName, "�v���C�}�� �f�B�X�v���C �h���C�o");

		// --- �������x���Ɖ�ʃ��[�h�ݒ� ---
		if (FAILED(lpDirectDraw->SetCooperativeLevel(hwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT))) {
			MessageBox(hwnd,"SetCooperativeLevel Fail","Error",MB_OK); ReleaseAll(); return FALSE;
		}
		// �f�B�X�v���C���[�h�ύX (BPP�ϐ����g�p)
		if (FAILED(lpDirectDraw->SetDisplayMode(640, 480, BPP))) {
			char err[100]; sprintf(err, "%d�r�b�g �f�B�X�v���C���[�h�ݒ莸�s", BPP);
			MessageBox(hwnd, err, "�������G���[", MB_OK); ReleaseAll(); return FALSE;
		}


		lpDirectDraw->SetCooperativeLevel(hwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT);

		//�f�B�X�v���C���[�h�ύX
		lpDirectDraw->SetDisplayMode(640, 480, 16);

		//��{�T�[�t�F�X�ƃt�����g�o�b�t�@�̐��� (�P���쐬)
		ZeroMemory(&Dds, sizeof(DDSURFACEDESC));
		Dds.dwSize = sizeof(DDSURFACEDESC);
		Dds.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		Dds.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_VIDEOMEMORY | DDSCAPS_3DDEVICE;
		Dds.dwBackBufferCount = 1;

		lpDirectDraw->CreateSurface(&Dds, &lpPrimary, NULL);

		//�o�b�N�o�b�t�@�̃|�C���^�擾
		Ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
		lpPrimary->GetAttachedSurface(&Ddscaps, &lpBackbuffer);

		// Z-Buffer�쐬
		//��{�T�[�t�F�X�ƃo�b�t�@�P���쐬
		ZeroMemory(&Dds, sizeof(DDSURFACEDESC));
		Dds.dwSize = sizeof(DDSURFACEDESC);
		Dds.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_ZBUFFERBITDEPTH;
		Dds.dwHeight = 640;
		Dds.dwWidth = 480;
		Dds.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY;
		Dds.dwZBufferBitDepth = 16;

		lpDirectDraw->CreateSurface(&Dds, &lpZbuffer, NULL);

		lpBackbuffer->AddAttachedSurface(lpZbuffer);

		//DirectDraw�ł̃A�N�Z�X���ł���悤�ɁAClipper������
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

		//�w�i�T�[�t�F�X���쐬
		Dds.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		Dds.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		Dds.dwWidth = 640;
		Dds.dwHeight = 480;
		lpDirectDraw->CreateSurface(&Dds, &lpScreen, NULL);

		//�J���[�L�[�̎w�� ��:RGB(255, 255, 255)�@��:RGB(0, 0, 0)
		ddck.dwColorSpaceLowValue=RGB(0, 0, 0);
		ddck.dwColorSpaceHighValue=RGB(0, 0, 0);
		lpScreen->SetColorKey(DDCKEY_SRCBLT, &ddck);

		//�e�T�[�t�F�X�ɉ摜��ǂݍ���
		LoadBMP(lpScreen, "datafile\\back.BMP");  //�w�i

		DirectDrawCreateClipper(0, &lpDDClipper, NULL);

		// --- Direct3D�f�o�C�X�̗񋓂ƑI�� ---
		if (!EnumDrivers()) {
			// �G���[���b�Z�[�W�� EnumDrivers ���ŕ\�������
			ReleaseAll();
			return FALSE;
		}

		// --- Direct3DRM�f�o�C�X�쐬 ---
		// EnumDrivers�őI�����ꂽGUID (DriverGUID[CurrDriver]) ���g�p����
		HRESULT ddret = lpDirect3DRM->CreateDeviceFromSurface(
							&DriverGUID[CurrDriver], // �I�����ꂽ�h���C�o��GUID
							lpDirectDraw,           // �쐬�ς݂�DirectDraw�I�u�W�F�N�g
							lpBackbuffer,
							&lpD3DRMDevice );

		if (FAILED(ddret)) {
			char err[200];
			sprintf(err, "�I�����ꂽ�h���C�o(%s)�ł�RM�f�o�C�X�쐬�Ɏ��s���܂����B", DriverName[CurrDriver]);
			MessageBox(hwnd, err, "�������G���[", MB_OK);
			ReleaseAll(); return FALSE;
		}
		// �I�����ꂽDirect3D�f�o�C�X���� szDevice �ɃR�s�[
		strncpy(szDevice, DriverName[CurrDriver], sizeof(szDevice) - 1);
		szDevice[sizeof(szDevice) - 1] = '\0';


		// --- �f�o�C�X�i���ݒ� ---
		lpD3DRMDevice->SetQuality(D3DRMFILL_SOLID | D3DRMSHADE_GOURAUD | D3DRMLIGHT_ON);

		lpDirect3DRM->CreateFrame(NULL, &lpD3DRMScene);

		//�J�������쐬
								// �e�t���[�� ,	�q�t���[��
		lpDirect3DRM->CreateFrame(lpD3DRMScene, &lpD3DRMCamera);

		lpD3DRMCamera->SetPosition(lpD3DRMScene, D3DVAL(0.0), D3DVAL(0.0), D3DVAL(0.0));

		//�f�o�C�X�ƃJ��������Direct3DRMViewPort���쐬
		lpDirect3DRM->CreateViewport(lpD3DRMDevice, lpD3DRMCamera, 0, 0, 640, 480, &lpD3DRMView);

		// �w�i�̉��s���A�v���[���[���_���W�������Ō��邱�Ƃ��ł���ő勗���ɐݒ肵�܂��B
		// ��������ƁA�_���W�����S�̂̎�����ז����邱�ƂȂ��A�����̃N���b�s���O�v���[�����ł��邾���߂��ɐݒ�ł��܂�
		lpD3DRMView->SetBack(D3DVAL(70.f));

		// �t�����g�N���b�s���O�v���[������ɋ߂��ɐݒ肵�܂�
		lpD3DRMView->SetFront(0.5f);

		// nav.cpp �i�r�Q�[�V�����V�X�e�������������܂� (���W���� ���ꂪ�Ȃ��ƈړ��ł��Ȃ�)
		SetupNavigator();		

		// scene.cpp �{�v���O������3D�I�u�W�F�N�g�̌Ăяo��, �V�[�����쐬���܂�
		BuildScene();

		// sound.cpp �V�[�����`�悳�ꂽ��ABGM(midi)���J�n���܂�
		StartBackgroundMusic(hwnd);

		while(1){

			if(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
				{
					if(!GetMessage(&msg, NULL, 0, 0))
						return msg.wParam;
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}else{

							// �Q�[�����[�v
							RenderFrame();

					}
		}
		return msg.wParam;

}
