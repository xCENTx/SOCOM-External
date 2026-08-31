#include "Menu.h"

int mainthread();
int emuThread();

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
	//	std::thread wcw(mainthread);
	//	std::thread ecw(emuThread);

	while (g_Menu->bRunning)
	{
		bool bTimer = GetTickCount64() - LastTick > 500;

		// SHOW / HIDE MENU
		{
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
		}

		/* PCSX2 MEMORY UPDATE */
		auto t0 = std::chrono::steady_clock::now();
		{
			g_PSXMemory.update();
		}

		/* SOCOM UPDATE */
		auto t1 = std::chrono::steady_clock::now();
		{
			g_SOCOM->Update();
		}

		/* DX WINDOW UPDATE */
		auto t2 = std::chrono::steady_clock::now();
		{
			g_dxWindow->CloneUpdate(g_PSXMemory.GetPsxInfo().hWnd);
			g_dxWindow->Update(g_Menu->GetOverlay());
		}

		auto t3 = std::chrono::steady_clock::now();


		g_Menu->m_refreshTimes[0] = std::chrono::duration<float, std::milli>(t1 - t0).count();		// pcsx2
		g_Menu->m_refreshTimes[1] = std::chrono::duration<float, std::milli>(t2 - t1).count();		// socom
		g_Menu->m_refreshTimes[2] = std::chrono::duration<float, std::milli>(t3 - t2).count();		// window
		g_Menu->m_refreshTimes[3] = std::chrono::duration<float, std::milli>(t3 - t0).count();		// total

		//	std::this_thread::sleep_for(1ms);
		std::this_thread::yield();
	}

	//	wcw.join();
	//	ecw.join();

	g_dxWindow->Shutdown();
	g_SOCOM->ShutDown();

	return EXIT_SUCCESS;
}

int mainthread()
{
	while (g_Menu->bRunning)
	{
		g_SOCOM->Update();

		//	std::this_thread::sleep_for(1ms);
		std::this_thread::yield();
	}

	return EXIT_SUCCESS;
}

int emuThread()
{

	while (g_Menu->bRunning)
	{
		g_PSXMemory.update();

		std::this_thread::sleep_for(100ms);
		std::this_thread::yield();
	}

	return EXIT_SUCCESS;
}