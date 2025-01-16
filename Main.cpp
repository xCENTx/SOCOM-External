#include "Menu.h"

int mainthread();

static int LastTick = 0;
int main()
{
	g_Memory = std::make_unique<Memory>("pcsx2-qt.exe");
	if (Memory::GetHandle() == INVALID_HANDLE_VALUE || !Memory::GetEEMemory())
	{
		printf("failed to attach to pcsx2 process.\n");
		return EXIT_FAILURE;
	}

	g_dxWindow = std::make_unique<DxWindow>();
	g_Menu = std::make_unique<Menu>();

	std::thread wcw(mainthread);

	while (g_Menu->bRunning)
	{
		bool bTimer = GetTickCount64() - LastTick > 500;
		if (GetAsyncKeyState(VK_LCONTROL) & 0x8000 && bTimer)
		{
			g_Menu->bShowMenu ^= 1;
			g_Menu->UpdateOverlayViewState(g_Menu->bShowMenu);
			switch (g_Menu->bShowMenu)
			{
				case(true): g_dxWindow->SetWindowFocus(g_dxWindow->GetWindowHandle()); break;
				case(false): g_dxWindow->SetWindowFocus(vmProcess.hWnd); break;
			}

			LastTick = GetTickCount64();
		}

		g_dxWindow->CloneUpdate(vmProcess.hWnd);
		g_dxWindow->Update(g_Menu->GetOverlay());

		std::this_thread::sleep_for(1ms);
		std::this_thread::yield();
	}

	wcw.join();

	g_SOCOM.release();
	g_Memory.release();
	g_dxWindow.release();

	return EXIT_SUCCESS;
}

int mainthread()
{
	while (g_Menu->bRunning)
	{
		g_SOCOM->Update();

		std::this_thread::sleep_for(1ms);
		std::this_thread::yield();
	}

	return EXIT_SUCCESS;
}