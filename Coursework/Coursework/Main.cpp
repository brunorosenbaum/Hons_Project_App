// Main.cpp
#include "../DXFramework/System.h"
#include "App1.h"
#include  "App2_Cubes.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	App1* app = new App1();
	App2_Cubes* app2_cubes = new App2_Cubes(); 
	System* system;

	srand(time(NULL));

	// Create the system object.
	//system = new System(app, 1200, 675, true, false);
	system = new System(app2_cubes, 1200, 675, true, false);

	// Initialize and run the system object.
	system->run();

	// Shutdown and release the system object.
	delete system;
	system = 0;

	return 0;
}