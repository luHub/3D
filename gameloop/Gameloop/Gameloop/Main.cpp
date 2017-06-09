#include <iostream>
#include <ctime>
#include <ratio>
#include <chrono>
#include <cstdint>
#include <ratio>
/*GameLoop Example  MonoBehaviour */
using namespace std::chrono;
int main() {
	bool OnEnable=true;
	bool isStarted = false;
	int physicsTimeframe = 100;

	//timeframe (30 FPS)
	float fixedUpdateSeconds = 1.0f / 30.0f;

	while (OnEnable==true) {
		//Read Time0
		high_resolution_clock::time_point timeStamp = std::chrono::high_resolution_clock::now();
			
		if (isStarted==false) {
			start();
			//Start is onle called one
			isStarted = true;
		}

		//Physics Timeframe
		//Using CPU High Resolution Timer
		runFixedUpdate(timeStamp);

		//UpdateCameraPosition
				
		
	}
}

//
void start() {}
void runFixedUpdate(high_resolution_clock::time_point tp) {}
void viewMatLocation() {}