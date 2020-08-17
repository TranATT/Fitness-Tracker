
// Andy Tran
// Last Edit: 3/14/19
/* The purpose of the code is to create two functions that can count the amount of steps the user takes and the amount of
 * flights of stairs that the user climbs. The code will automatically calibrate to any person that uses the fitness tracker. 
 * The code is meant to work on the fitness tracker that we designed and will keep track of the steps and flights of stairs
 * on the MIT Bluetooth App.
 */
#include <Wire.h>
#include <DFRobot_LIS2DH12.h>

#include "DFRobot_BMP388.h"
#include "DFRobot_BMP388_I2C.h"
#include "Wire.h"
#include "SPI.h"
#include "math.h"
#include "bmp3_defs.h"
#define CALIBRATE_Altitude



//Color to pick on graph is Red
// Use the y axis
// Base line values: (-750 ,-620) hard coded in for Kevin while testing
DFRobot_LIS2DH12 LIS; //Accelerometer
DFRobot_BMP388_I2C bmp388; // Barometer

float seaLevel;
int steps = 0; //Step variable
//int highbase = -620; // Kevins coordinates while testing
//int lowbase = -750; // Kevins coordinates while testing
int vibration = 5; //Vibration Pin
int stairs = 0; //Stair variable
int avgAltitude = 3; // Average Altitude Change for 1 Flight of Stairs in meters
float base = 28.0; //Base Sea Level Value 
int highbase = 0; // Max value set to 0
int lowbase = 0; // Min value set to 0
int count = 0; // Intitial count value
int goal = 10; // Goal indicated by the User on the MIT App



void setup()
{
  pinMode(vibration, OUTPUT);
  //Accelerometer Setup
  Wire.begin();
  Serial.begin(9600);
  while(!Serial);
  delay(100); 
  // Set measurement range
  // Ga: LIS2DH12_RANGE_2GA
  // Ga: LIS2DH12_RANGE_4GA
  // Ga: LIS2DH12_RANGE_8GA
  // Ga: LIS2DH12_RANGE_16GA
  while(LIS.init(LIS2DH12_RANGE_16GA) == -1)
  {  //Equipment connection exception or I2C address error
    //Serial.println("No I2C devices found");
    delay(1000);
  }
  
  //Barometric and Altitude Setup
  Serial.begin(9600);
  /*Initialize bmp388*/
  while(bmp388.begin())
  {
    //Serial.println("Initialize error!");
    delay(1000);
  }
  /*You can use an accurate altitude to calibrate sea level air pressure. 
   *And then use this calibrated sea level pressure as a reference to obtain the calibrated altitude.
   *In this case,525.0m is chendu accurate altitude.
   */
  delay(100);
  seaLevel = bmp388.readSeaLevel(28.0); // Set the standard sea leavel at Irvine to 28
  //Serial.print("seaLevel : ");
  //Serial.print(seaLevel);
  //Serial.println(" Pa");
  
}

void loop()
{
  acceleration(); // Steps Function
  barometric(); // Stairs Function
  
  
}



/*!
 *  @brief Print the position result.
 */
void acceleration(void)
{
  int16_t x, y, z; // We choose y as our coordinate

  
  LIS.readXYZ(x, y, z); // Reads in the coordinates
  LIS.mgScale(x, y, z);
  
  
  //Calibration
  if (count == 0) // Sets the first y coordinate equal to the max
  {
    //Serial.println("Place your arm by your side in resting position");
    highbase = y;
    count = count + 1;
   
  }
  
  if (count < 30) //Runs through 30 times in order to determine which values are the highbase and lowbase
  {
    
    if (y > highbase) // If y user coordinate is greater than high base value then set low base equal to high base and high base equal to y user coordinate
    {
      lowbase = highbase;
      highbase = y;
      
    }
    if (highbase > y) // If high base value is greater than y user coordinate, then set lowbase equal to the y user coordinate
    {
      lowbase = y;
    }
    
    count = count + 1;
    
  }
  
  if (count == 30) //Finishes finding the high and low base values and tells to user to begin walking
  {
    Serial.println("You may now begin walking");
    count = count + 1;
    delay(3000);
  }
  
  if (count == 31) // Begins the steps functions to count
  {
    if (y > highbase) // If the walk is greater than the high range then increment step
    {
      steps = steps + 1;
      Serial.write(6);
      Serial.print("Number of Steps: ");
      Serial.println(steps);
      delay(600);
    }
    if (y < lowbase) // If the walk is less than the low range then increment step
    {
      steps = steps + 1;
      Serial.write(6);
      Serial.print("Number of Steps: ");
      Serial.println(steps);
      delay(600);
    }
    
    
    if (steps == goal) // Vibration Motor Function that will buzz the motor when the user reaches the indicated goal on the app
    {
      digitalWrite(vibration, HIGH);
      delay(300);
      digitalWrite(vibration, LOW);
    }
  }

}

void barometric(void)
{
   #ifdef CALIBRATE_Altitude
  /* Read the calibrated altitude */
  float altitude = bmp388.readCalibratedAltitude(seaLevel); // Initializes the initial altitude
  
  if (abs(altitude - base) >= avgAltitude)// Calculates the stairs increment by seeing if the (altitude - base) is greater than the average altitude of 3.0 m
  {
    stairs = stairs + 1; // Increments stairs
    Serial.write(8);
    Serial.print("Flight of Stairs Climbed: ");
    Serial.println(stairs);
    delay(500);
    base = altitude; // Sets the base altitude to the new altitude
    
  }
 
  #else
  /* Read the altitude */
  float altitude = bmp388.readAltitude();
  
  #endif
  delay(100);
   /* Read the atmospheric pressure, print data via serial port.*/
  float Pressure = bmp388.readPressure();
 
  
  delay(100);
  /* Read the temperature, print data via serial port.*/
  float Temperature = bmp388.readTemperature();
  
  delay(1000);
}
