//#include <LowPower.h>
#include <Wire.h>
#include <RTClib.h>

// constants
const unsigned int pump_time = 667; // ms: calibrate this
const unsigned int sensor_interval = 2;

const float filter_per = 0.2f;
const float ph_threshold = 7;
const float ec_threshold = 500;

// pin assignments
const unsigned int ph_pump_pin = 5;
const unsigned int nutrient_pump_pin = 6;

const unsigned int led_pin = 4;
const unsigned int buzzer_pin = 7;

const unsigned int circulation_ctrl_pin = 9;
const unsigned int light_ctrl_pin = 8;

const unsigned int ph_sensor_pin = 11;
const unsigned int tds_sensor_pin = 12;

RTC_DS1307 rtc;

double waterflow;
double ph;
double ec;

void setup() {
  Serial.begin(9600);

  pinMode(led_pin, OUTPUT);
  pinMode(light_ctrl_pin, OUTPUT);
  pinMode(buzzer_pin, OUTPUT);

  test_init();

  if (!rtc.begin()) {
    Serial.println("RTC not found");
    while (1);
  }

  // Set time only if the RTC is stopped
  if (!rtc.isrunning()) {
    Serial.println("Setting RTC time...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  pinMode(ph_pump_pin, OUTPUT);
  digitalWrite(ph_pump_pin, LOW);  
  
  pinMode(nutrient_pump_pin, OUTPUT);
  digitalWrite(nutrient_pump_pin, LOW);

  digitalWrite(led_pin, LOW);

  pinMode(circulation_ctrl_pin, OUTPUT);
  digitalWrite(circulation_ctrl_pin, LOW);

  digitalWrite(light_ctrl_pin, LOW);

  pinMode(ph_sensor_pin, INPUT);

  // for flow rate sensor
  waterflow = 0;
  //attachInterupt(0, pulse, rising); // digital pin 2

  float ph = 7; // ph, midpoint of sensor range
  float ec = 500; //ppm, midpoint of sensor range
}

void loop() {

  // check for conditions
  // flow rate
  // level sensor
  // 

  // record ph, tds
  ph = filter_per * read_ph() + (1 - filter_per) * ph;
  ec = filter_per * read_ec() + (1 - filter_per) * ec;

  // update ph, tds
  if (ph < ph_threshold) {
    dispense(ph_pump_pin, pump_time);
  }
  if (ec < ec_threshold) {
    dispense(nutrient_pump_pin, pump_time);
  }

  // sleep 2 seconds
  //LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
}
