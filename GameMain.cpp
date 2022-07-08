#include <Windows.h>
#include <time.h>

#pragma comment(lib,"Msimg32.lib")

#define WC_Title	L"2048"			//���ڱ���
#define WC_Height	800 			//���ڸ߶�
#define WC_Wide		651.8519  		//���ڿ��
#define beilv 4						//ͼƬ���ű���
int  pianyiX = -10;					//ͼƬƫ����x
int  pianyiY = 0;					//ͼƬƫ����y

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam); 
void GameStart(HWND hwnd);
void GameUpdate(HWND hwnd);

int lenx;
int leny;
HDC hdc;
HDC mdc;
HDC bufdc;
int s[4][4];
int Record1[4][4];
int Record2[4][4];
int gradeR1, gradeR2;
int g[16] =
{
	4		,8		,16		,32		,
	64		,128	,256	,512	,
	1024	,2048	,4096	,8192	,
	16384	,32768	,65536
};
int grade;

enum GameState							//�����������״̬���� ��ö������
{
	GS_Menu,
	GS_Playing,
	GS_Result
}gamestate;

HBITMAP bmp[17];						//���ڴ洢��0~65536��ͼƬ
HBITMAP bmp1[10];						//���ڴ洢��0~9��ͼƬ
HBITMAP bmp2[1];						//���ڴ洢�����յ÷ֱ�����ͼƬ

struct Control
{
	void record1()
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				Record1[i][j] = s[i][j];
			}
		}
		gradeR1 = grade;
	}

	void record2()
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				Record2[i][j] = Record1[i][j];
			}
		}
		gradeR2 = gradeR1;
	}

	void Recall()
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				s[i][j] = Record2[i][j];
			}
		}
		grade = gradeR2;
	}

	int Check()
	{
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				if (s[i][j] == s[i + 1][j] || s[i][j] == s[i][j + 1] || s[i][j] == 0)
				{
					return 1;
				}
			}
			if (s[3][i] == s[3][i + 1] || s[3][i] == 0 || s[3][i + 1] == 0)
			{
				return 1;
			}
			if (s[i][3] == s[i + 1][3] || s[i][3] == 0 || s[3][i + 1] == 0)
			{
				return 1;
			}
		}
		return 0;
	}

	void generate()
	{
		srand(time(0));
		int pos = rand() % 16;
		int n = 0;
		while (1)
		{
			int x = n % 4;
			int y = n / 4;
			if (s[x][y] == 0)
			{
				if (pos-- <= 0)
				{
					s[x][y] = (rand() % 7 == 0 ? 2 : 1);
					break;
				}
			}
			n = (n + 1) % 16;
		}
	}

	void ssz()
	{
		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				int tmp = s[i][j];
				s[i][j] = s[3 - j][i];
				s[3 - j][i] = s[3 - i][3 - j];
				s[3 - i][3 - j] = s[j][3 - i];
				s[j][3 - i] = tmp;
			}
		}
	}

	void nsz()
	{
		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				int tmp = s[j][3 - i];
				s[j][3 - i] = s[3 - i][3 - j];
				s[3 - i][3 - j] = s[3 - j][i];
				s[3 - j][i] = s[i][j];
				s[i][j] = tmp;
			}
		}
	}

	int Up()
	{
		int flag = 0;
		for (int j = 0; j <= 3; j++)
		{
			int tmp[4];
			int l = 0, r = 3;
			for (int i = 0; i <= 3; i++)
			{
				if (s[i][j] != 0)
				{
					tmp[l++] = s[i][j];
				}
				else
				{
					tmp[r--] = s[i][j];
				}
			}
			if (l == 0)
			{
				continue;
			}
			for (int i = 0; i <= 3; i++)
			{
				if (s[i][j] != tmp[i])
				{
					flag = 1;
				}
				s[i][j] = tmp[i];
			}
			for (int i = 0; i < l-1; i++)
			{
				if (s[i][j] == s[i + 1][j])
				{
					flag = 1;
					s[i][j] += 1;
					grade += g[s[i][j] - 2];
					for (int m = i + 1; m <= 2; m++)
					{
						s[m][j] = s[m + 1][j];
					}
					s[l - 1][j] = 0;
					l--;
				}
			}
		}
		return flag;
	}

	int Down()
	{
		control.ssz();
		control.ssz();
		int flag = control.Up();
		control.nsz();
		control.nsz();
		return flag;
	}

	int Left()
	{
		control.ssz();
		int flag = control.Up();
		control.nsz();
		return flag;
	}

	int Right()
	{
		control.nsz();
		int flag = control.Up();
		control.ssz();
		return flag;
	}
}control;

struct GameResult
{
	void Init(HWND hwnd)
	{
		bmp2[0] = (HBITMAP)LoadImage(NULL, L"�ز�/image/Result.bmp", IMAGE_BITMAP, WC_Wide, WC_Height, LR_LOADFROMFILE);
	}

	void Update(HWND hwnd)
	{
		hdc = GetDC(hwnd);
		mdc = CreateCompatibleDC(hdc);
		bufdc = CreateCompatibleDC(hdc);
		HBITMAP whitebmp = (HBITMAP)LoadImage(NULL, L"�ز�/image/WhiteBackGround.bmp", IMAGE_BITMAP, 1536, 841, LR_LOADFROMFILE);
		SelectObject(mdc, whitebmp);
		SelectObject(bufdc, bmp2[0]);
		BitBlt(mdc, pianyiX, pianyiY, 1536, 841, bufdc, 0, 0, SRCCOPY);
		int s[7] = { 0,0,0,0,0,0,0 };
		int ws = grade == 0 ? 0 : -1;
		int grade1 = grade;
		while (grade1 > 0)
		{
			s[++ws] = grade1 % 10;
			grade1 /= 10;
		}
		double x = WC_Wide / 2 + 1.0 * (214 / beilv) * (ws / 2);
		for (int i = 0; i <= ws; i++)
		{

			SelectObject(bufdc, bmp1[s[i]]);
			TransparentBlt
			(
				mdc,
				pianyiX + x - i * 214 / beilv, pianyiY + 400,
				214 / beilv, 414 / beilv,
				bufdc,
				0, 0,
				214 / beilv, 414 / beilv,
				RGB(255, 255, 255)
			);

		}
		BitBlt(hdc, 0, 0, 1536, 841, mdc, 0, 0, SRCCOPY);
		DeleteDC(bufdc);
		DeleteDC(mdc);
		ReleaseDC(hwnd, hdc);
	}
		
	void OnWindowMessage(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
	{
		if (MessageBox(hwnd, L"���¿�ʼ", L"�Ƿ����¿�ʼ", MB_YESNO | MB_ICONQUESTION | MB_APPLMODAL) == 6)
		{
			gamestate = GS_Playing;
			GameStart(hwnd);
		}
	}
}gameresult;

struct GamePlaying
{
	void Init(HWND hwnd)
	{
		bmp[0] = (HBITMAP)LoadImage(NULL, L"�ز�/image/picture(0).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[1] = (HBITMAP)LoadImage(NULL, L"�ز�/image/picture(1).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[2] = (HBITMAP)LoadImage(NULL, L"�ز�/image/picture(2).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[3] = (HBITMAP)LoadImage(NULL, L"�ز�/image/picture(3).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[4] = (HBITMAP)LoadImage(NULL, L"�ز�/image/picture(4).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[5] = (HBITMAP)LoadImage(NULL, L"�ز�/image/picture(5).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[6] = (HBITMAP)LoadImage(NULL, L"�ز�/image/picture(6).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[7] = (HBITMAP)LoadImage(NULL, L"�ز�/image/picture(7).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[8] = (HBITMAP)LoadImage(NULL, L"�ز�/image/picture(8).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[9] = (HBITMAP)LoadImage(NULL, L"�ز�/image/picture(9).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[10] = (HBITMAP)LoadImage(NULL, L"�ز�/image/picture(10).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[11] = (HBITMAP)LoadImage(NULL, L"�ز�/image/picture(11).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[12] = (HBITMAP)LoadImage(NULL, L"�ز�/image/picture(12).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[13] = (HBITMAP)LoadImage(NULL, L"�ز�/image/picture(13).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[14] = (HBITMAP)LoadImage(NULL, L"�ز�/image/picture(14).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[15] = (HBITMAP)LoadImage(NULL, L"�ز�/image/picture(15).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[16] = (HBITMAP)LoadImage(NULL, L"�ز�/image/picture(16).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);

		bmp1[0] = (HBITMAP)LoadImage(NULL, L"�ز�/image/number0.bmp", IMAGE_BITMAP, 214 / beilv, 414 / beilv, LR_LOADFROMFILE);
		bmp1[1] = (HBITMAP)LoadImage(NULL, L"�ز�/image/number1.bmp", IMAGE_BITMAP, 214 / beilv, 414 / beilv, LR_LOADFROMFILE);
		bmp1[2] = (HBITMAP)LoadImage(NULL, L"�ز�/image/number2.bmp", IMAGE_BITMAP, 214 / beilv, 414 / beilv, LR_LOADFROMFILE);
		bmp1[3] = (HBITMAP)LoadImage(NULL, L"�ز�/image/number3.bmp", IMAGE_BITMAP, 214 / beilv, 414 / beilv, LR_LOADFROMFILE);
		bmp1[4] = (HBITMAP)LoadImage(NULL, L"�ز�/image/number4.bmp", IMAGE_BITMAP, 214 / beilv, 414 / beilv, LR_LOADFROMFILE);
		bmp1[5] = (HBITMAP)LoadImage(NULL, L"�ز�/image/number5.bmp", IMAGE_BITMAP, 214 / beilv, 414 / beilv, LR_LOADFROMFILE);
		bmp1[6] = (HBITMAP)LoadImage(NULL, L"�ز�/image/number6.bmp", IMAGE_BITMAP, 214 / beilv, 414 / beilv, LR_LOADFROMFILE);
		bmp1[7] = (HBITMAP)LoadImage(NULL, L"�ز�/image/number7.bmp", IMAGE_BITMAP, 214 / beilv, 414 / beilv, LR_LOADFROMFILE);
		bmp1[8] = (HBITMAP)LoadImage(NULL, L"�ز�/image/number8.bmp", IMAGE_BITMAP, 214 / beilv, 414 / beilv, LR_LOADFROMFILE);
		bmp1[9] = (HBITMAP)LoadImage(NULL, L"�ز�/image/number9.bmp", IMAGE_BITMAP, 214 / beilv, 414 / beilv, LR_LOADFROMFILE);
	}

	void Start(HWND hwnd)
	{
		grade = 0;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				s[i][j] = 0;
			}
		}
		control.generate();
		control.generate();
		control.record1();
		control.record2();
	}

	void Update(HWND hwnd)
	{
		hdc = GetDC(hwnd);
		mdc = CreateCompatibleDC(hdc);
		bufdc = CreateCompatibleDC(hdc);

		{
			HBITMAP whitebmp = (HBITMAP)LoadImage(NULL, L"�ز�/image/WhiteBackGround.bmp", IMAGE_BITMAP, 1536, 841, LR_LOADFROMFILE);
			SelectObject(mdc, whitebmp);
			HBITMAP background = (HBITMAP)LoadImage(NULL, L"�ز�/image/BackGround5.bmp", IMAGE_BITMAP, WC_Wide, WC_Height, LR_LOADFROMFILE);
			SelectObject(bufdc, background);
			BitBlt(mdc, pianyiX, pianyiY, 1536, 841, bufdc, 0, 0, SRCCOPY);
		}//����

		{
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					SelectObject(bufdc, bmp[s[j][i]]);
					TransparentBlt
					(
						mdc,
						pianyiX + 60 + i * (500 / beilv + 10), pianyiY + 200 + j * (500 / beilv + 10),
						500 / beilv, 500 / beilv,
						bufdc,
						0, 0,
						500 / beilv, 500 / beilv,
						RGB(255, 255, 255)
					);
				}
			}
		}//ͼƬ		
		int s[7] = { 0,0,0,0,0,0,0 };
		int ws = grade == 0 ? 0 : -1;
		int grade1 = grade;
		while (grade1 > 0)
		{
			s[++ws] = grade1 % 10;
			grade1 /= 10;
		}
		for (int i = 0; i <= ws; i++)
		{

				SelectObject(bufdc, bmp1[s[i]]);
				TransparentBlt
				(
					mdc,
					pianyiX + (7 - i) * 214 / beilv + 140, pianyiY + 62,
					214 / beilv, 414 / beilv,
					bufdc,
					0, 0,
					214 / beilv, 414 / beilv,
					RGB(255, 255, 255)
				);

		}
		BitBlt(hdc, 0, 0, 1536, 841 , mdc, 0, 0, SRCCOPY);

		DeleteDC(bufdc);
		DeleteDC(mdc);
		ReleaseDC(hwnd, hdc);

	}

	void OnWindowMessage(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
	{
		switch (message)
		{
		case WM_KEYDOWN:
			if (wparam == 82)
			{
				control.Recall();
				break;
			}
			switch (wparam)
			{
			case VK_LEFT:
				control.record1();
				if (control.Left())
				{
					control.generate();
					control.record2();
				}
				if (!control.Check())																	
				{
					gamestate = GS_Result;
				}
				break;
			case VK_RIGHT:
				control.record1();
				if (control.Right())
				{
					control.generate();
					control.record2();
				}
				if (!control.Check())
				{
					gamestate = GS_Result;
				}
				break;
			case VK_UP:
				control.record1();
				if (control.Up())
				{
					control.generate();
					control.record2();
				}
				if (!control.Check())
				{
					gamestate = GS_Result;
				}
				break;
			case VK_DOWN:
				control.record1();
				if (control.Down())
				{
					control.generate();
					control.record2();
				}
				if (!control.Check())
				{
					gamestate = GS_Result;
				}
				break;
			}
		}
	}
}gameplaying;

struct GameMenu
{
	void Update(HWND hwnd)				//�����ڵ�ǰ״̬����ʱҪ���Ĺ���
	{
		hdc = GetDC(hwnd);
		mdc = CreateCompatibleDC(hdc);
		bufdc = CreateCompatibleDC(hdc);
		HBITMAP whitebmp = (HBITMAP)LoadImage(NULL, L"�ز�/image/WhiteBackGround.bmp", IMAGE_BITMAP, 1536, 841, LR_LOADFROMFILE);
		SelectObject(mdc, whitebmp);
		HBITMAP MenuBackGround = (HBITMAP)LoadImage(NULL, L"�ز�/image/MenuBackGround.bmp", IMAGE_BITMAP, WC_Wide, WC_Height, LR_LOADFROMFILE);
		SelectObject(bufdc, MenuBackGround);
		BitBlt(mdc, pianyiX, pianyiY, WC_Wide, WC_Height, bufdc, 0, 0, SRCCOPY);
		BitBlt(hdc, 0, 0, 1536, 841, mdc, 0, 0, SRCCOPY);
		DeleteDC(bufdc);
		DeleteDC(mdc);
		ReleaseDC(hwnd, hdc);
	}

	void OnWindowMessage(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
	{
		gamestate = GS_Playing;
		gameplaying.Start(hwnd);
	}
}gamemenu;

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR LpCmdLine, _In_ int nShowCmd)
{
	//1.��ƴ���

	WNDCLASSEX wndClass = {};
	wndClass.cbSize = sizeof(WNDCLASSEX);				//���ڴ�С
	wndClass.lpszClassName = L"WindowsName";			//��������
	wndClass.lpfnWndProc = WndProc;						//���غ���
	wndClass.hInstance = hInstance;						//���ô��ھ�����ɲ�����
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);		//��������Ƶ�������ʱ�Ĺ������ IDC_ARROW�Ǽ�ͷ
	wndClass.hIcon = LoadIcon(hInstance, (LPCTSTR)100);

	//2.ע�ᴰ��

	if ((int)RegisterClassEx(&wndClass) == false)	//���ݴ��ڵ�ַ
	{											//�з���ֵ��ʾע��ɹ���ʧ��
		return -1;								//��ʧ��ֱ���˳�
	}											

	//3.��������

	HWND hwnd = CreateWindow
	( 
		L"WindowsName",						//��һ���Ǵ�������
		WC_Title,							//�ڶ����Ǵ��ڱ�������
		WS_OVERLAPPEDWINDOW,				//�������Ǵ��ڷ��ֱ�룩WS_OVERLAPPEDWINDOW��Ԥ���һ������
		CW_USEDEFAULT, CW_USEDEFAULT,		//���ĸ��͵�����Ǵ���λ��  CW_USEDEFAULT ��Ĭ��
		WC_Wide, WC_Height,					//�������͵��߸��Ǵ��ڸ߶ȺͿ�� 
		NULL, NULL, hInstance, NULL			//���涼����NULL
	);										//�����ڶ�������ѡ��дWinMain�ĵ�һ������Ҳ����NULL
											//����һ�����������Ĵ��ھ��

	//4.��ʾ����

	ShowWindow(hwnd, nShowCmd);	//�ô���ʱ���صľ��
	UpdateWindow(hwnd);			//�̶����� ���´���

	//��Ϸ��ʼ��

	GameStart(hwnd);

	//5.������Ϣ
	MSG msg = { 0 };//�����Ϣ
	while (msg.message != WM_QUIT)//�����յ��˳������棩ʱ����ѭ��
	{
		if (PeekMessage(&msg, 0, NULL, NULL, PM_REMOVE))	//�Ӷ�����ȥ��Ϣ��ȡ����Ϣ����true��û����false
		{													//��һ��������Ҫ���浽��MSG���ͱ����ĵ�ַ��2��3��4������NULL��ʱ����
															//���һ���ǽ�����Ϣ���Ƿ��Ƴ���Ϣ PM_REMOVE��ʾȷ��
			TranslateMessage(&msg);//������Ϣ��������Ϣ������ַ
			DispatchMessage(&msg); //�������غ�����������WndProc������
		}
	}

	//��Ϸ����

	UnregisterClass(L"WindowsName", hInstance);	//ע������	
	return 0;

}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)//�������¼����������ʹ��ڹر�֮�ࣩ
{																			   //HWND �����UINT ��Ϣ�����������ͼ���
																			   //WPARAM ���̰�����Ϣ��LPARAM �������Ϣ			
	switch (message)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_KEYDOWN:
		switch (gamestate)
		{
		case GS_Menu:
			gamemenu.OnWindowMessage(hwnd, message, wparam, lparam);
			gamemenu.Update(hwnd);
			break;
		case GS_Playing:
			gameplaying.OnWindowMessage(hwnd, message, wparam, lparam);
			gameplaying.Update(hwnd);
			break;
		case GS_Result:
			gameresult.Update(hwnd);
			gameresult.OnWindowMessage(hwnd, message, wparam, lparam);
			break;
		} 
		break;

	case WM_SIZE: 
		leny = HIWORD(lparam);
		lenx = LOWORD(lparam);
		pianyiX = -10 + (lenx - WC_Wide+39) / 2;
		pianyiY = 0 + (leny - WC_Height+16) / 2;
		switch (gamestate)
		{
		case GS_Menu:
			gamemenu.Update(hwnd);
			break;
		case GS_Playing:
			gameplaying.Update(hwnd);
			break;
		case GS_Result:
			gameresult.Update(hwnd);
			break;
		}
		break;
		

	case WM_DESTROY:		//�����ݵ���ϢΪ�˳�ʱ
		PostQuitMessage(0);	//ֹͣ����
		break;

	default:
		return DefWindowProc(hwnd, message, wparam, lparam);
		break;

	}
	return 0;
}

void GameStart(HWND hwnd)
{
	gameplaying.Init(hwnd);
	gameresult.Init(hwnd);
	gamestate = GS_Menu;
	gamemenu.Update(hwnd);
}

void GameUpdate(HWND hwnd)
{
	switch (gamestate)
	{
	case GS_Menu:
		gamemenu.Update(hwnd);
		break;
	case GS_Playing:
		gameplaying.Update(hwnd);
		break;
	case GS_Result:
		gameresult.Update(hwnd);
		break;
	}
}