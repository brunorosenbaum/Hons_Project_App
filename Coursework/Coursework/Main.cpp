// Main.cpp
#include "../DXFramework/System.h"
#include  "LightningApp.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	LightningApp* lightning_app = new LightningApp(); 
	System* system;

	srand(time(NULL));

	// Create the system object.
	//system = new System(app, 1200, 675, true, false);
	system = new System(lightning_app, 1200, 675, true, false);

	// Initialize and run the system object.
	system->run();

	// Shutdown and release the system object.
	delete system;
	system = 0;

	return 0;
}