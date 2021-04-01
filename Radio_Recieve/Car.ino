#include <SPI.h>
#include "RF24.h"
#include <nRF24L01.h>
#include <RF24_config.h>
#include <Servo.h>

class parseval {                                                              // The following class parses the integer value from the
																			  //int val;
																			  // incoming string
public:
	int Update(String temp, int startIndex, int EndIndex) {
		String temp2 = temp.substring(startIndex + 1, EndIndex);
		return temp2.toInt();
	}
};

int find(String inString, String identifier, int num) {                         // This function will search for and return the starting and
	int index[5];                                                          // ending index number for the sensor value from incoming string.
	index[0] = inString.indexOf(identifier);                                    // Values from this function are refered to the 'parseval'
	for (unsigned int i = 1; i <= sizeof(index[5]); i++) {                          // class
		index[i] = inString.indexOf(identifier, index[i - 1] + 1);
	}
	return index[num];
}

//To calculate communication speed
int timingCounter;
unsigned long int startingMillis;


//Create instances to parse from incoming data
parseval drive;
parseval steer;
parseval isReverse;
Servo steerServo;
Servo ESC;

//Settings for Radio communication
const uint64_t receiverAddr = 0xcbde079c3e;
const uint64_t senderAddr = 0xcbde079c3d;
RF24 radio(9, 10);

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);

	radio.begin();
	radio.openReadingPipe(1, receiverAddr);
	radio.openWritingPipe(senderAddr);
	radio.setPALevel(RF24_PA_MAX);
	Serial.println("PA Level = " + String(radio.getPALevel()));
	radio.startListening();
	
	//Starting timing counter
	startingMillis = millis();

	steerServo.attach(3);
	ESC.attach(5);
	//pinMode(5, OUTPUT);
	steerServo.write(90);
	ESC.writeMicroseconds(1500);
	Serial.println("Receiver Initialised");
}

// the loop function runs over and over again until power down or reset
void loop() {
	//Serial.println("Waiting for incoming data");
	char text[32];
	
	
	//Keep on writing until responce is recieved
	
	
	do
	{
		delay(20);
		char abc[] = "Ready";
		radio.stopListening();
		bool isWritten = radio.write(&abc, sizeof(abc));
		radio.startListening();
		if (isWritten) { /*Serial.println("Successfully written: Ready");*/ }
	} while (!radio.available());
	
	
	//Read the recieved value

	radio.read(&text, sizeof(text));

	//To ensure data is recieved.
	if (text[0] != 0) {
		//Serial.println("Recieved: " + String(text));
		timingCounter++;  //Counter increases if a successful transaction of data occurs.
	}
	/*
	if (isReverse.Update(text, find(text, "R", 0), find(text, "/", 1)) == 1) {
		ESC.writeMicroseconds(1000);
		delay(20);
		ESC.writeMicroseconds(1000);
		delay(20);
		ESC.writeMicroseconds(1500);
	}*/

	int driveVal = drive.Update(text, find(text, "f", 0), find(text, "/", 0));
	//Serial.println(driveVal);
	int steerVal = steer.Update(text, find(text, "y", 0), find(text, "/", 1));
	//Serial.println(steerVal);
	steerServo.write(steerVal);
	//analogWrite(5, driveVal);
	ESC.writeMicroseconds(driveVal);
	//Calculating successful communication rate

	if (timingCounter >= 1000) {
	    double temp = (millis() - startingMillis)/1000;
		Serial.println("Communication rate = " + String(timingCounter / temp) + "Hz");
		timingCounter = 0;
		startingMillis = millis();
		//delay(5000);
	}


}
