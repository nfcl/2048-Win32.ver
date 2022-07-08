#include <Windows.h>
#include <time.h>

#pragma comment(lib,"Msimg32.lib")

#define WC_Title	L"2048"			//窗口标题
#define WC_Height	800 			//窗口高度
#define WC_Wide		651.8519  		//窗口宽度
#define beilv 4						//图片缩放倍率
int  pianyiX = -10;					//图片偏移量x
int  pianyiY = 0;					//图片偏移量y

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

enum GameState							//定义程序运行状态常量 用枚举类型
{
	GS_Menu,
	GS_Playing,
	GS_Result
}gamestate;

HBITMAP bmp[17];						//用于存储从0~65536的图片
HBITMAP bmp1[10];						//用于存储从0~9的图片
HBITMAP bmp2[1];						//用于存储从最终得分背景的图片

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
		bmp2[0] = (HBITMAP)LoadImage(NULL, L"素材/image/Result.bmp", IMAGE_BITMAP, WC_Wide, WC_Height, LR_LOADFROMFILE);
	}

	void Update(HWND hwnd)
	{
		hdc = GetDC(hwnd);
		mdc = CreateCompatibleDC(hdc);
		bufdc = CreateCompatibleDC(hdc);
		HBITMAP whitebmp = (HBITMAP)LoadImage(NULL, L"素材/image/WhiteBackGround.bmp", IMAGE_BITMAP, 1536, 841, LR_LOADFROMFILE);
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
		if (MessageBox(hwnd, L"重新开始", L"是否重新开始", MB_YESNO | MB_ICONQUESTION | MB_APPLMODAL) == 6)
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
		bmp[0] = (HBITMAP)LoadImage(NULL, L"素材/image/picture(0).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[1] = (HBITMAP)LoadImage(NULL, L"素材/image/picture(1).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[2] = (HBITMAP)LoadImage(NULL, L"素材/image/picture(2).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[3] = (HBITMAP)LoadImage(NULL, L"素材/image/picture(3).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[4] = (HBITMAP)LoadImage(NULL, L"素材/image/picture(4).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[5] = (HBITMAP)LoadImage(NULL, L"素材/image/picture(5).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[6] = (HBITMAP)LoadImage(NULL, L"素材/image/picture(6).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[7] = (HBITMAP)LoadImage(NULL, L"素材/image/picture(7).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[8] = (HBITMAP)LoadImage(NULL, L"素材/image/picture(8).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[9] = (HBITMAP)LoadImage(NULL, L"素材/image/picture(9).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[10] = (HBITMAP)LoadImage(NULL, L"素材/image/picture(10).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[11] = (HBITMAP)LoadImage(NULL, L"素材/image/picture(11).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[12] = (HBITMAP)LoadImage(NULL, L"素材/image/picture(12).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[13] = (HBITMAP)LoadImage(NULL, L"素材/image/picture(13).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[14] = (HBITMAP)LoadImage(NULL, L"素材/image/picture(14).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[15] = (HBITMAP)LoadImage(NULL, L"素材/image/picture(15).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);
		bmp[16] = (HBITMAP)LoadImage(NULL, L"素材/image/picture(16).bmp", IMAGE_BITMAP, 500 / beilv, 500 / beilv, LR_LOADFROMFILE);

		bmp1[0] = (HBITMAP)LoadImage(NULL, L"素材/image/number0.bmp", IMAGE_BITMAP, 214 / beilv, 414 / beilv, LR_LOADFROMFILE);
		bmp1[1] = (HBITMAP)LoadImage(NULL, L"素材/image/number1.bmp", IMAGE_BITMAP, 214 / beilv, 414 / beilv, LR_LOADFROMFILE);
		bmp1[2] = (HBITMAP)LoadImage(NULL, L"素材/image/number2.bmp", IMAGE_BITMAP, 214 / beilv, 414 / beilv, LR_LOADFROMFILE);
		bmp1[3] = (HBITMAP)LoadImage(NULL, L"素材/image/number3.bmp", IMAGE_BITMAP, 214 / beilv, 414 / beilv, LR_LOADFROMFILE);
		bmp1[4] = (HBITMAP)LoadImage(NULL, L"素材/image/number4.bmp", IMAGE_BITMAP, 214 / beilv, 414 / beilv, LR_LOADFROMFILE);
		bmp1[5] = (HBITMAP)LoadImage(NULL, L"素材/image/number5.bmp", IMAGE_BITMAP, 214 / beilv, 414 / beilv, LR_LOADFROMFILE);
		bmp1[6] = (HBITMAP)LoadImage(NULL, L"素材/image/number6.bmp", IMAGE_BITMAP, 214 / beilv, 414 / beilv, LR_LOADFROMFILE);
		bmp1[7] = (HBITMAP)LoadImage(NULL, L"素材/image/number7.bmp", IMAGE_BITMAP, 214 / beilv, 414 / beilv, LR_LOADFROMFILE);
		bmp1[8] = (HBITMAP)LoadImage(NULL, L"素材/image/number8.bmp", IMAGE_BITMAP, 214 / beilv, 414 / beilv, LR_LOADFROMFILE);
		bmp1[9] = (HBITMAP)LoadImage(NULL, L"素材/image/number9.bmp", IMAGE_BITMAP, 214 / beilv, 414 / beilv, LR_LOADFROMFILE);
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
			HBITMAP whitebmp = (HBITMAP)LoadImage(NULL, L"素材/image/WhiteBackGround.bmp", IMAGE_BITMAP, 1536, 841, LR_LOADFROMFILE);
			SelectObject(mdc, whitebmp);
			HBITMAP background = (HBITMAP)LoadImage(NULL, L"素材/image/BackGround5.bmp", IMAGE_BITMAP, WC_Wide, WC_Height, LR_LOADFROMFILE);
			SelectObject(bufdc, background);
			BitBlt(mdc, pianyiX, pianyiY, 1536, 841, bufdc, 0, 0, SRCCOPY);
		}//背景

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
		}//图片		
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
	void Update(HWND hwnd)				//程序在当前状态运行时要做的工作
	{
		hdc = GetDC(hwnd);
		mdc = CreateCompatibleDC(hdc);
		bufdc = CreateCompatibleDC(hdc);
		HBITMAP whitebmp = (HBITMAP)LoadImage(NULL, L"素材/image/WhiteBackGround.bmp", IMAGE_BITMAP, 1536, 841, LR_LOADFROMFILE);
		SelectObject(mdc, whitebmp);
		HBITMAP MenuBackGround = (HBITMAP)LoadImage(NULL, L"素材/image/MenuBackGround.bmp", IMAGE_BITMAP, WC_Wide, WC_Height, LR_LOADFROMFILE);
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
	//1.设计窗口

	WNDCLASSEX wndClass = {};
	wndClass.cbSize = sizeof(WNDCLASSEX);				//窗口大小
	wndClass.lpszClassName = L"WindowsName";			//窗口类名
	wndClass.lpfnWndProc = WndProc;						//返回函数
	wndClass.hInstance = hInstance;						//设置窗口句柄（可不做）
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);		//设置鼠标移到窗口上时的光标类型 IDC_ARROW是箭头
	wndClass.hIcon = LoadIcon(hInstance, (LPCTSTR)100);

	//2.注册窗口

	if ((int)RegisterClassEx(&wndClass) == false)	//传递窗口地址
	{											//有返回值表示注册成功和失败
		return -1;								//若失败直接退出
	}											

	//3.创建窗口

	HWND hwnd = CreateWindow
	( 
		L"WindowsName",						//第一个是窗口类名
		WC_Title,							//第二个是窗口标题名称
		WS_OVERLAPPEDWINDOW,				//第三个是窗口风格（直译）WS_OVERLAPPEDWINDOW是预设的一种类型
		CW_USEDEFAULT, CW_USEDEFAULT,		//第四个和第五个是窗口位置  CW_USEDEFAULT 是默认
		WC_Wide, WC_Height,					//第六个和第七个是窗口高度和宽度 
		NULL, NULL, hInstance, NULL			//后面都可以NULL
	);										//倒数第二个可以选择写WinMain的第一个参数也可以NULL
											//返回一个创建出来的窗口句柄

	//4.显示窗口

	ShowWindow(hwnd, nShowCmd);	//用创建时返回的句柄
	UpdateWindow(hwnd);			//固定步骤 更新窗口

	//游戏初始化

	GameStart(hwnd);

	//5.处理消息
	MSG msg = { 0 };//存放消息
	while (msg.message != WM_QUIT)//当接收到退出（点叉叉）时跳出循环
	{
		if (PeekMessage(&msg, 0, NULL, NULL, PM_REMOVE))	//从队列中去消息，取到消息返回true，没有则false
		{													//第一个参数是要保存到的MSG类型变量的地址，2，3，4都设置NULL暂时无用
															//最后一个是接收消息后是否移除消息 PM_REMOVE表示确定
			TranslateMessage(&msg);//翻译消息，传递消息变量地址
			DispatchMessage(&msg); //交给返回函数（这里是WndProc）处理
		}
	}

	//游戏结束

	UnregisterClass(L"WindowsName", hInstance);	//注销窗口	
	return 0;

}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)//处理窗口事件（键盘鼠标和窗口关闭之类）
{																			   //HWND 句柄，UINT 消息类型区分鼠标和键盘
																			   //WPARAM 键盘按键消息，LPARAM 鼠标光标信息			
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
		

	case WM_DESTROY:		//当传递的消息为退出时
		PostQuitMessage(0);	//停止程序
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