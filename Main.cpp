#include "Menu.h"
#include "SOCOM.h"
static bool bRunning{ true };

int GameUpdate();

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
	g_SOCOM = std::make_unique<SOCOM>();

	std::thread wcw(GameUpdate);

	while (bRunning)
	{
		if (GetAsyncKeyState(VK_END) & 0x8000)
			bRunning = false;
		
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

int GameUpdate()
{
	while (bRunning)
	{
		g_SOCOM->Update();

		std::this_thread::sleep_for(1ms);
		std::this_thread::yield();
	}

	return EXIT_SUCCESS;
}