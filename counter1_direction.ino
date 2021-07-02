#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <Ethernet.h>
#include <SPI.h>
#include <SparkFun_VL53L1X.h>
// declare people_st was save RTC so after reset doesn't earased data
static RTC_NOINIT_ATTR int people_st = 0; 

SFEVL53L1X ROIcenter;
//Optional interrupt and shutdown pins.
#define SHUTDOWN_PIN 2
#define INTERRUPT_PIN 3

#define dis_Threshold 1300
#define dis_Ignore 60

int people = 0;
int cnt = 10;
//declare dis array 2
uint16_t dis[2]; 

// define ROI centers
uint8_t center[2] = {175, 231}; // or 159, 231

// object status
enum Object {NOBODY, SOMEONE, NODIR}; // NODIR ( can't identified the direction)

// zone status
enum Zone {NONE, OUTSIDE, INSIDE, STABLE};

enum Object currentStatus = NOBODY;
enum Zone zone;
enum Zone buff = NONE;

enum Zone currentZone(uint16_t distance0, uint16_t distance1){ // enum return NON, OUTSIDE, INSIDE, STABLE
  if(distance0 < dis_Ignore || distance1 < dis_Ignore){   
      Serial.println ("NONE");
      return NONE;
  } else if (distance0 < dis_Threshold && distance1 > dis_Threshold){ 
      Serial.println ("OUTSIDE");
      return OUTSIDE;
  } else if (distance1 < dis_Threshold && distance0 > dis_Threshold){ 
      Serial.println ("INSIDE");
      return INSIDE;
  } else if (distance0 < dis_Threshold && distance1 < dis_Threshold){ 
      Serial.println ("STABLE");
      return STABLE;
  } else {
    Serial.println ("NONE");
    return NONE;}
}

void setup(void)
{
    Wire.begin(); 
    // use 400 kHz I2C
    Wire.setClock(400000); 
    Serial.begin(115200);
    Serial.println("Begin.....");

    if (ROIcenter.init() != 0) 
    {
        ESP_LOGE("VL53L1X custom sensor", "Failed to detect and initialize sensor!");  // log or logging technique // using announcement the status
        while (1); // turn back to check the conditioner
    }

    ROIcenter.setDistanceModeLong(); //Set to 4M range
    ROIcenter.setTimingBudgetInMs(33); //Set the timing budget for a measurement
    ROIcenter.setIntermeasurementPeriod(33); //Get time between measurements in ms 
 
    // Check reason if press reset button
    esp_reset_reason_t reason = esp_reset_reason();  // reset reson ?? why need ?
    if (reason != 3){ 
        people_st = 0;
    }
    people = people_st;
}

void loop(void)
{
    // Calculate distance to in/out zones
    for (int i = 0; i < 2; i++)  // find the value distance0 and disance0
    {
      ROIcenter.startRanging(); // Begins taking measurements
      ROIcenter.setDistanceModeLong(); // Set to 4M range
      // Set the height and width of the ROI(region of interest) in SPADs, lowest possible option is 4. Set optical center based on above table
      ROIcenter.setROI(8, 16, center[i]); //center 175 và center 231 // after the first measurement, switch center from 175 into 231
      dis[i] = ROIcenter.getDistance(); // return distance 
      ROIcenter.stopRanging(); //Stops taking measurements // after measure the first distance stop measurement
      ROIcenter.clearInterrupt(); // Clear the interrupt flag // clear the interrupt 
      Serial.print ("Zone : ");
      Serial.println (i);
      Serial.print ("distance: ");
      Serial.println (dis[i]);
      if(dis[i] == 0){   // if distance = 0, reduce cnt, // people_st = people ???  // avoid case sensor can't measure the distance so result is 0 => backup
        cnt--;
        people_st = people;
        delay(1000);
        Serial.print("BACKUP, current count: ");
        Serial.println(people_st);
        if(cnt == 0){ // when the distance continuously repeat => reset ESP
            Serial.println("Status, Detected problem with sensor, resetting ....");
            ESP.restart();
        }
      }
       delay (100);
    }
    
    uint16_t distance0=dis[0];  // save value distane have just been measured in distance[0]
    uint16_t distance1=dis[1];  // save value distane have just been measured in distance[1]

    enum Zone current_zone = currentZone(distance0, distance1); // based on value distance0, distance1 => status of current_zone (NONE, STABLE, INSIDE, OUTSIDE)
    
    // enum Zone buff => buff (NONE, STABLE, INSIDE, OUTSIDE) 
    if((buff == INSIDE || buff == OUTSIDE) && current_zone == NONE){
        currentStatus == NOBODY;
        buff = NONE;
    }
    if (currentStatus == SOMEONE && current_zone != STABLE){  // currentStatus (enum Zone SOMEONE, NOBODY, NODIR)
      if(current_zone == NONE){
        currentStatus = NOBODY;
        buff = NONE;
      }
      if ((buff == OUTSIDE) && (current_zone == INSIDE))
      { 
        people++;
        currentStatus == NOBODY;
        buff = NONE;
      } else if ((buff == INSIDE) && (current_zone == OUTSIDE))
      {
        if(people == 0){
            Serial.println("ISSUE, Someone went out but current people is 0, can't count down");
        } else {
            people--;
        }
        currentStatus == NOBODY;
        buff = NONE;
      }
    } else if(currentStatus == NOBODY && (current_zone == INSIDE || current_zone == OUTSIDE)){
      Serial.println("Status, Detected someone");
      buff = current_zone;
      currentStatus = SOMEONE;
    } else if(currentStatus == NOBODY && current_zone == STABLE){
      currentStatus = NODIR;
      buff = STABLE;
      Serial.println("Status, Detected someone but didn't know direction");
    } else if(currentStatus == NODIR){
      if (current_zone == INSIDE){
        people++;
        buff = NONE;
        currentStatus = NOBODY;
      } else if(current_zone == OUTSIDE){
        if(people == 0){
            Serial.println("ISSUE, Someone went out but current people is 0, can't count down");
        } else {
            people--;
        }
        buff = NONE;
        currentStatus = NOBODY;
      } else if(current_zone == NONE){
        buff = NONE;
        currentStatus = NOBODY;
      }
    }
    Serial.print("people: ");
    Serial.println(people);  
    // return;
}
