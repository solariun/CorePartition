///
/// @mainpage	Thread_embedXCode
///
/// @details	Description of the project
/// @n
/// @n
/// @n @a		Developed with [embedXcode+](https://embedXcode.weebly.com)
///
/// @author		GUSTAVO CAMPOS
/// @author		GUSTAVO CAMPOS
/// @date		10/03/2020 12:48
/// @version	<#version#>
///
/// @copyright	(c) GUSTAVO CAMPOS, 2020
/// @copyright	All rights reserved
///
/// @see		ReadMe.txt for references
///


///
/// @file		Thread_embedXCode.ino
/// @brief		Main sketch
///
/// @details	<#details#>
/// @n @a		Developed with [embedXcode+](https://embedXcode.weebly.com)
///
/// @author		GUSTAVO CAMPOS
/// @author		GUSTAVO CAMPOS
/// @date		10/03/2020 12:48
/// @version	<#version#>
///
/// @copyright	(c) GUSTAVO CAMPOS, 2020
/// @copyright	All rights reserved
///
/// @see		ReadMe.txt for references
/// @n
///


// Core library for code-sense - IDE-based
// !!! Help: http://bit.ly/2AdU7cu
#if defined(WIRING) // Wiring specific
#include "Wiring.h"
#elif defined(MAPLE_IDE) // Maple specific
#include "WProgram.h"
#elif defined(ROBOTIS) // Robotis specific
#include "libpandora_types.h"
#include "pandora.h"
#elif defined(MPIDE) // chipKIT specific
#include "WProgram.h"
#elif defined(DIGISPARK) // Digispark specific
#include "Arduino.h"
#elif defined(ENERGIA) // LaunchPad specific
#include "Energia.h"
#elif defined(LITTLEROBOTFRIENDS) // LittleRobotFriends specific
#include "LRF.h"
#elif defined(MICRODUINO) // Microduino specific
#include "Arduino.h"
#elif defined(TEENSYDUINO) // Teensy specific
#include "Arduino.h"
#elif defined(REDBEARLAB) // RedBearLab specific
#include "Arduino.h"
#elif defined(RFDUINO) // RFduino specific
#include "Arduino.h"
#elif defined(SPARK) || defined(PARTICLE) // Particle / Spark specific
#include "application.h"
#elif defined(ESP8266) // ESP8266 specific
#include "Arduino.h"
#elif defined(ARDUINO) // Arduino 1.0 and 1.5 specific
#include "Arduino.h"
#else // error
#error Platform not defined
#endif // end IDE

// Set parameters


// Include application, user and local libraries
// !!! Help http://bit.ly/2CL22Qp


// Define structures and classes


// Define variables and constants


// Prototypes
// !!! Help: http://bit.ly/2TAbgoI


// Utilities


// Functions


// Add setup code
void setup()
{

}

// Add loop code
void loop()
{

}
