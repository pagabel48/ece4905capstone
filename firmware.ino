#include <Wire.h>
#include <RTClib.h>

// pin assignments
#define PH_UP_LEVEL_PIN 18        // pin 24
#define PH_DOWN_LEVEL_PIN 19      // pin 25
#define NUTRIENT_LEVEL_PIN 20    // pin 26
#define RESERVOIR_LEVEL_PIN 18   // pin 24

#define PH_UP_PUMP_PIN 18        // pin 24
#define PH_DOWN_PUMP_PIN 19      // pin 25
#define NUTRIENT_PUMP_PIN 20     // pin 26

#define LED_PIN 11               // pin 15
#define BUZZER_PIN 12            // pin 16
#define BUTTON_PIN 13            // pin 17

#define CIRCULATION_CTRL_PIN 8   // pin 11
#define LIGHT_CTRL_PIN 9         // pin 12

#define FLOW_SENSOR_PIN 10       // pin 14

#define PH_SENSOR_PIN 26         // pin 31, adc
#define EC_SENSOR_PIN 27         // pin 32, adc

#define SDA_PIN 0                // pin 1, i2c
#define SCL_PIN 1                // pin 2, i2c

/**
pump time should be set to get desired ph, ec change
  -> based off of calculations, and testing

threshold and setpoints can be changed as desired

ph setpoint = 7, threshold = 0.5
each time more liquid dispensed, ph should go up by ~.75

ec setpoint = 500, threshold = 100
ec up by ~150 when triggered

any two pumps can fire in a cycle so minimum loop period 
must be greater than 2 largest pump times, plus some headroom
*/

// constants
const unsigned int NUTRIENT_PUMP_TIME = 667; // ms: calibrate this
const unsigned int PH_UP_PUMP_TIME = 667; // ms: calibrate this
const unsigned int PH_DOWN_PUMP_TIME = 667; // ms: calibrate this

  // how frequently main loop is called (in ms), start small (500ms) for testing, ~1min for production
const unsigned int LOOP_PERIOD_MS = 500;
const unsigned int SENSOR_PERIOD_MS = 40;

const float FILTER_PER = 0.1f;
const float VREF = 3.3;

const float PH_SETPOINT = 7.0f; // dH
const float EC_SETPOINT = 500.0f; // ppm

const float PH_THRESHOLD = 0.5f; // dH
const float EC_THRESHOLD = 100.0f; // ppm

// min num rotations of flow sensor to establish normal flow
const float FLOW_ROTATIONS_MIN = 300;

  // hour in military time
const int DAY_START = 7;
const int DAY_END = 19;

// global variables
RTC_DS1307 rtc; //ds1307

repeating_timer_t led_timer;
repeating_timer_t sensor_timer;

  // placeholder values, needed since using weighted avg. to filter
volatile int waterflow = 0;
float ph = 7.0f;
float ec = 500.0f;

bool system_operational = true;

void setup() {
  Serial.begin(9600);

  Wire.setSDA(SDA_PIN);
  Wire.setSCL(SCL_PIN);
  Wire.begin(); // init i2c

  // setup float level switches
  pinMode(PH_UP_LEVEL_PIN, INPUT_PULLUP);
  pinMode(PH_DOWN_LEVEL_PIN, INPUT_PULLUP);
  pinMode(NUTRIENT_LEVEL_PIN, INPUT_PULLUP);
  pinMode(RESERVOIR_LEVEL_PIN, INPUT_PULLUP);

  // setup ph, ec sensors
  pinMode(PH_SENSOR_PIN, INPUT);
  pinMode(EC_SENSOR_PIN, INPUT);

  // setup dispensing pumps
  pinMode(PH_UP_PUMP_PIN, OUTPUT);
  digitalWrite(PH_UP_PUMP_PIN, LOW);  
  
  pinMode(PH_DOWN_PUMP_PIN, OUTPUT);
  digitalWrite(PH_DOWN_PUMP_PIN, LOW);  
  
  pinMode(NUTRIENT_PUMP_PIN, OUTPUT);
  digitalWrite(NUTRIENT_PUMP_PIN, LOW);

  // setup led, buzzer, button
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // setup cutoff pins
  pinMode(CIRCULATION_CTRL_PIN, OUTPUT);
  digitalWrite(CIRCULATION_CTRL_PIN, HIGH);

  pinMode(LIGHT_CTRL_PIN, OUTPUT);
  digitalWrite(LIGHT_CTRL_PIN, LOW);

  // setup flow sensor, increments via interupt every sensor rotation
  pinMode(FLOW_SENSOR_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), flow_sensor_pulse, RISING);

  // connect to real time clock
  if (!rtc.begin()) {
    Serial.println("RTC not found");
  }

  // begin reading from the sensors
  start_sensors();

  // test program, comment this out for production
  test_init();

  // setup rtc, if plugged in read time from computer, if not continue
  set_rtc_time();

  // attach interupt so that button issues a reset, uses inline function
  attachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN), 
    button_reset, 
    FALLING);
}

// main task ran at a set interval
void loop () {
  long next_run = millis() + LOOP_PERIOD_MS;

  adjust_system();

  // while loop for redundancy
  while ((long)(next_run - millis()) > 0) {
    sleep_us((next_run - millis()) * 1000);
  }
}

bool adjust_system() {

  // make sure liquid levels are normal, if not alert user, wait for reset
  if (system_operational && liquid_levels_low()) {

    start_led_flash(500);

    system_operational = false;
  }

  // waterflow and ph read async via interupt

  // make sure flow isn't blocked, if it is alert user, turn off pump
  if (waterflow < FLOW_ROTATIONS_MIN) {

    // cancel timer if it is running, we want led to stay on
    stop_led_flash();

    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);

    digitalWrite(CIRCULATION_CTRL_PIN, LOW);

    system_operational = false;
  }

  // everything normal so adjust water parameters
  if (system_operational){
    // comment out for prod
    Serial.println(ph);
    Serial.println(ec);

    // adjust ec
    if (ec < EC_SETPOINT - EC_THRESHOLD) {
      dispense_liquid(NUTRIENT_PUMP_PIN, NUTRIENT_PUMP_TIME);
    }

    // adjust ph
    if (ph < PH_SETPOINT - PH_THRESHOLD) {
      dispense_liquid(PH_UP_PUMP_PIN, PH_UP_PUMP_TIME);
    } else if (ph > PH_SETPOINT - PH_THRESHOLD) {
      dispense_liquid(PH_DOWN_PUMP_PIN, PH_DOWN_PUMP_TIME);
    }
  }

  // turn on/off light
  int hour = rtc.now().hour();
  if (hour >= DAY_START && hour < DAY_END) {
      digitalWrite(LIGHT_CTRL_PIN, HIGH);
  }
  else {
      digitalWrite(LIGHT_CTRL_PIN, LOW);
  }

  //reset flow sensor rotation counter
  waterflow = 0;

  // satisfy interupt, continue to call this function
  return true;
}
