#include "Menu.h"

int mainthread();

static int LastTick = 0;
int main()
{
	//  load game data
	g_SOCOM = std::make_unique<SOCOM>();

	//	Initialize Menu
	g_Menu = std::make_unique<Menu>();

	//	Initialize d3d window
	g_dxWindow = std::make_unique<DxWindow>();
	g_dxWindow->Init();

	//	Initialize Background Thread
	std::thread wcw(mainthread);

	while (g_Menu->bRunning)
	{
		bool bTimer = GetTickCount64() - LastTick > 500;
		if (GetAsyncKeyState(VK_RCONTROL) & 0x8000 && bTimer)
		{
			g_Menu->bShowMenu ^= 1;
			g_Menu->UpdateOverlayViewState(g_Menu->bShowMenu);
			switch (g_Menu->bShowMenu)
			{
				case(true): g_dxWindow->SetWindowFocus(g_dxWindow->GetWindowHandle()); break;
				case(false): g_dxWindow->SetWindowFocus(g_PSXMemory.GetPsxInfo().hWnd); break;
			}

			LastTick = GetTickCount64();
		}

		//	g_PSXMemory.update();
		//	g_SOCOM->Update();

		g_dxWindow->CloneUpdate(g_PSXMemory.GetPsxInfo().hWnd);
		g_dxWindow->Update(g_Menu->GetOverlay());

		std::this_thread::sleep_for(1ms);
		std::this_thread::yield();
	}

	wcw.join();

	g_dxWindow->Shutdown();
	g_SOCOM->ShutDown();

	return EXIT_SUCCESS;
}

int mainthread()
{
	while (g_Menu->bRunning)
	{
		g_PSXMemory.update();
		g_SOCOM->Update();

		std::this_thread::sleep_for(1ms);
		std::this_thread::yield();
	}

	return EXIT_SUCCESS;
}