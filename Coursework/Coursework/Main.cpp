// Main.cpp
#include "../DXFramework/System.h"
#include  "LightningAppTK.h"
#include  "LightningAppJY.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	LightningAppTK* lightning_appTK = new LightningAppTK();
	LightningAppJY* lightning_appJY = new LightningAppJY(); 
	System* system;

	srand(time(NULL));

	// Create the system object.
	//system = new System(app, 1200, 675, true, false);
	system = new System(lightning_appJY, 1200, 675, true, false);

	// Initialize and run the system object.
	system->run();
	// Shutdown and release the system object.
	delete system;
	system = 0;

	return 0;
}