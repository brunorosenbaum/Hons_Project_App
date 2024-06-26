// Timer object.
// Calculate delta/frame time and FPS.
#include <fstream>
#include <string>

#include "timer.h"

// Initialise timer. Check for high performance timers.
Timer::Timer()
{
	QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
	if (frequency == 0)
	{
		MessageBox(NULL, L"No support for high performance timer", L"ERROR", MB_OK);
	}

	// Find out how many times the frequency counter ticks every second.
	ticksPerS = (float)(frequency);

	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	elapsedTime = 0.f;
	frames = 0.f;
	fps = 0.f;
}


Timer::~Timer()
{
}

// Once per frame calculate delta timer and update FPS calculation.
void Timer::frame()
{
	INT64 currentTime;
	float timeDifference;

	// Query the current time.
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
	timeDifference = (float)(currentTime - startTime);
	frameTime = timeDifference / ticksPerS;

	// Calc FPS
	frames += 1.f;
	elapsedTime += frameTime;
	if (elapsedTime > 1.0f)
	{
		fps = frames / elapsedTime;
		frames = 0.0f;
		elapsedTime = 0.0f;
	}
	
	// Restart the timer.
	startTime = currentTime;

	return;
}



float Timer::getTime()
{
	return frameTime;
}

float Timer::getFPS()
{
	return fps;
}

void Timer::outputCSV(float elapsed, std::string& timeFile, std::string& fpsFile)
{
	
	std::ofstream csvFile(timeFile);
	std::ofstream csvFPSFile(fpsFile); 
	
	csvFile << std::to_string(elapsed) /*<< "," << getFPS()*/; //1st column 1st row for time, 2nd column for fps
	csvFPSFile << std::to_string(getFPS()); 

}
