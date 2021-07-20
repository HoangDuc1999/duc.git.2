#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <Ethernet.h>
#include <SPI.h>
#include <VL53L1X.h>

static RTC_NOINIT_ATTR int people_st = 0; // save data in RTC memory
VL53L1X ROIcenter;

#define SHUTDOWN_PIN 2
#define INTERRUPT_PIN 3

#define NOBODY                    0
#define SOMEONE                   1
#define LEFT                      0
#define RIGHT                     1

#define DISTANCES_ARRAY_SIZE                         2  // number of samples // 1 sample corresponding 2 measurement time
#define DIST_THRESHOLD                               1600  // mm

uint8_t Center[2] = {175, 231}; // ROI center
static int zone = 0;
static int Distance = 0;
static int PeopleCount= 0;
static int cnt = 5;
int printPeople = 0;

uint16_t readDistance(VL53L1X distanceSensor, int center){ // function Measure the distance
  distanceSensor.setROISize(5, 16); // ROW of SPAD : 5, COLUMN of SPAD: 16
  distanceSensor.setROICenter(center);
  distanceSensor.startContinuous(15);
  distanceSensor.read();
  Serial.println (distanceSensor.read ());
  distanceSensor.stopContinuous();
  return distanceSensor.ranging_data.range_mm;
}

int ProcessPeopleCountingData(int16_t Distance, uint8_t zone) {  // function Counting people
  static int PathTrack[] = {0,0,0,0};  // static int => declared once and persisted during the program
  static int PathTrackFillingSize = 1; // init this to 1 as we start from state where nobody is any of the zones
  static int LeftPreviousStatus = NOBODY;
  static int RightPreviousStatus = NOBODY;
  static uint16_t Distances[2][DISTANCES_ARRAY_SIZE];
  static uint8_t DistancesTableSize[2] = {0,0};  
  uint16_t MinDistance;
  uint8_t i;  
  int CurrentZoneStatus = NOBODY;
  int AllZonesCurrentStatus = 0;
  int AnEventHasOccured = 0;

  if (DistancesTableSize[zone] < DISTANCES_ARRAY_SIZE) {
    Distances[zone][DistancesTableSize[zone]] = Distance;
    DistancesTableSize[zone] ++;
  } else {
    for (i=1; i < DISTANCES_ARRAY_SIZE; i++){
      Distances[zone][i-1] = Distances[zone][i]; 
      }
    Distances[zone][DISTANCES_ARRAY_SIZE-1] = Distance;
  }

  MinDistance = Distances[zone][0];
  if (DistancesTableSize[zone] >= 2) {
    for (i=1; i<DistancesTableSize[zone]; i++) {
      if (Distances[zone][i] < MinDistance){
        MinDistance = Distances[zone][i];
        }
    }
  }
   if (MinDistance < DIST_THRESHOLD) {
    // Someone is in !
    CurrentZoneStatus = SOMEONE; // SOMEONE = 1, NOBODY = 0
  }
 if (zone == LEFT) { // LEFT = 0 // RIGHT = 1 => zone0 = LEFT , zone1 =RIGHT
    if (CurrentZoneStatus != LeftPreviousStatus) {
      // event in left zone has occured
      AnEventHasOccured = 1;
      
      if (CurrentZoneStatus == SOMEONE) {
        AllZonesCurrentStatus += 1;
      }
      // need to check right zone as well ...
      if (RightPreviousStatus == SOMEONE) {
        // event in left zone has occured
        AllZonesCurrentStatus += 2;
      }
      // remember for next time
      LeftPreviousStatus = CurrentZoneStatus;
    }
  }
   else {    
    if (CurrentZoneStatus != RightPreviousStatus) {
      
      // event in left zone has occured
      AnEventHasOccured = 1;
      if (CurrentZoneStatus == SOMEONE) {
        AllZonesCurrentStatus += 2;
      }
      // need to left right zone as well ...
      if (LeftPreviousStatus == SOMEONE) {
        // event in left zone has occured
        AllZonesCurrentStatus += 1;
      }
      // remember for next time
      RightPreviousStatus = CurrentZoneStatus;
    }
  }

if (AnEventHasOccured) {
    if (PathTrackFillingSize < 4) {
      PathTrackFillingSize ++;
    }    
    // if nobody anywhere lets check if an exit or entry has happened
    if ((LeftPreviousStatus == NOBODY) && (RightPreviousStatus == NOBODY)) {
      
      // check exit or entry only if PathTrackFillingSize is 4 (for example 0 1 3 2) and last event is 0 (nobobdy anywhere)
      if (PathTrackFillingSize == 4) {
        // check exit or entry. no need to check PathTrack[0] == 0 , it is always the case
        
        if ((PathTrack[1] == 1)  && (PathTrack[2] == 3) && (PathTrack[3] == 2)) {
          // This an entry
          PeopleCount ++;
          // reset the table filling size in case an entry or exit just found
          DistancesTableSize[0] = 0;
          DistancesTableSize[1] = 0;
          printf("Walk In, People Count=%d\n", PeopleCount);
        } else if ((PathTrack[1] == 2)  && (PathTrack[2] == 3) && (PathTrack[3] == 1)) {
          if (PeopleCount == 0){
            Serial.println ("Can't count negative number");
          }else {
          // This an exit
          PeopleCount --;
          }
          // reset the table filling size in case an entry or exit just found
          DistancesTableSize[0] = 0;
          DistancesTableSize[1] = 0;
          printf("Walk Out, People Count=%d\n", PeopleCount);
        } else {
          // reset the table filling size also in case of unexpected path
          DistancesTableSize[0] = 0;
          DistancesTableSize[1] = 0;
          printf("Wrong path\n");
        }
      }
      PathTrackFillingSize = 1;
    }
    else {   
      PathTrack[PathTrackFillingSize-1] = AllZonesCurrentStatus;
    }
  }   
  return(PeopleCount);
}

void setup(void){
    Wire.begin();
    Wire.setClock(400000); // use 400 kHz I2C
    Serial.begin(115200);
    Serial.println("Begin.....");     
 
    if (!ROIcenter.init()){
        Serial.println("VL53L1X custom sensor, Failed to detect and initialize ROIcenter!");
        while (1);
    }
    delay(10);             
    ROIcenter.setTimeout(500);  
    ROIcenter.setDistanceMode(VL53L1X::Long); // setMode :LONG (4M)
    ROIcenter.setMeasurementTimingBudget(33); // 33ms

    esp_reset_reason_t reason = esp_reset_reason();
    if (reason != 3){
      people_st = 0;
    }
    PeopleCount = people_st; // Remake data PeopleCount by data saved in people_st (RTC memmory)
}

void loop(void){      

    Distance = readDistance (ROIcenter, Center[zone] );

    if(Distance == 0){   // if distance was continuously 0 => cnt -- => 5 times => reset ESP
        cnt--;  // declared = 5;
        people_st = PeopleCount; // save current value of PeopleCount in peope_st for backup  
        Serial.print("BACKUP, current count: ");
        Serial.println(people_st);
        if(cnt == 0){ 
            Serial.println("Status, Detected problem with sensor, resetting ....");
            ESP.restart();
        }
     }    
    printPeople = ProcessPeopleCountingData(Distance, zone); // update the status of number of people
    Serial.println (printPeople);
    zone = zone +1;
    zone = zone % 2; // 1 or 0
}
   

