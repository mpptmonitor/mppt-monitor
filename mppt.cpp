//Libraries
#include <TimerOne.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

// ina
Adafruit_INA219 ina219batt(0x44);
Adafruit_INA219 ina219solar(0x40);


//Constants
#define bulk_voltage_max 14.5
#define bulk_voltage_min 13
#define absorption_voltage 14.7
#define float_voltage_max 13
#define battery_min_voltage 9
#define solar_min_voltage 12
#define charging_current 2000        // mA
#define absorption_max_current 2000  //mA
#define absorption_min_current 200
#define float_voltage_min 13.2
#define float_voltage 13.4
#define float_max_current 220
#define Serial_refresh_rate 1000
byte BULK = 0;  //Give values to each mode
byte ABSORPTION = 1;
byte FLOAT = 2;
byte mode = 0;  //We start with mode 0 BULK


//Outputs
#define PWM_out 9
#define load_enable 5

//Variables
float bat_voltage = 0;
float bat_current = 0;
int pwm_value = 0;
float solar_current = 0;
float current_factor = 0.185;  //Value defined by manufacturer ACS712 5A
float solar_voltage = 0;
float solar_power = 0;
float output_power = 0;
String load_status = "OFF";
int pwm_percentage = 0;
unsigned int before_millis = 0;
unsigned int now_millis = 0;
String mode_str = "BULK";


float solar_voltage_sum = 0;
float bat_voltage_sum = 0;
float solar_current_sum = 0;
float bat_current_sum = 0;
int count = 0;

bool h12Flag;
bool pmFlag;

// set for ATS
int hour = 21;
int minute = 35;
int second = 0;


void setup() {

  if (!ina219batt.begin()) {
    Serial.println("Failed to find INA219_1 chip");
    while (1) {
      delay(10);
    }
  }
  if (!ina219solar.begin()) {
    Serial.println("Failed to find INA219_2 chip");
    while (1) {
      delay(10);
    }
  }

  pinMode(load_enable, OUTPUT);
  digitalWrite(load_enable, LOW);  //Start with the relay turned off
  Timer1.initialize(100);          // 100 us => 10 kHz
  Timer1.pwm(PWM_out, 0);          // Start with 0 duty

  Serial.begin(115200);
  Wire.begin();
  before_millis = millis;  //Used for LCD refresh rate
}

void loop() {
  solar_voltage = ina219solar.getBusVoltage_V();
  bat_voltage = ina219batt.getBusVoltage_V();
  solar_current = ina219solar.getCurrent_mA() / 1000.0;  // Convert to amperes
  bat_current = ina219batt.getCurrent_mA() / 1000.0;     // Convert to amperes
  solar_power = solar_voltage * solar_current;
  output_power = bat_voltage * bat_current;
  pwm_percentage = map(pwm_value, 0, 1023, 0, 100);

  // Accumulate sums for average calculation
  solar_voltage_sum += solar_voltage;
  bat_voltage_sum += bat_voltage;
  solar_current_sum += solar_current;
  bat_current_sum += bat_current;
  count++;

  now_millis = millis();
  if (now_millis - before_millis > Serial_refresh_rate) {
    before_millis = now_millis;

    // Calculate averages
    float avg_solar_voltage = solar_voltage_sum / count;
    float avg_bat_voltage = bat_voltage_sum / count;
    float avg_solar_current = solar_current_sum / count;
    float avg_bat_current = bat_current_sum / count;
    float avg_solar_power = avg_solar_voltage * avg_solar_current;
    float avg_output_power = avg_bat_voltage * avg_bat_current;

    // Print dalam format array
    Serial.print("[");
    Serial.print(solar_voltage);
    Serial.print(", ");
    Serial.print(bat_voltage);
    Serial.print(", ");
    Serial.print(solar_current);
    Serial.print(", ");
    Serial.print(bat_current);
    Serial.print(", ");
    Serial.print(solar_power);
    Serial.print(", ");
    Serial.print(output_power);
    Serial.print(", ");
    Serial.print(pwm_percentage);
    Serial.print(", ");
    Serial.print(mode_str);
    Serial.print(", ");
    Serial.print(load_status);
    Serial.println("]");

    // Reset sums and count for next period
    solar_voltage_sum = 0;
    bat_voltage_sum = 0;
    solar_current_sum = 0;
    bat_current_sum = 0;
    count = 0;
  }


  if (bat_voltage < battery_min_voltage) {
    digitalWrite(load_enable, LOW);  //We DISABLE the load if battery is undervoltage
    load_status = "OFF";
  } else {
    digitalWrite(load_enable, HIGH);  //We ENABLE the load if battery charged
    load_status = "ON";
  }

  ///////////////////////////FLOAT///////////////////////////
  if (mode == FLOAT) {
    if (bat_voltage < float_voltage_min) {
      mode = BULK;
      mode_str = "BULK";
    }

    else {
      if (solar_current > float_max_current) {  //If we exceed max current value, we change mode
        mode = BULK;
        mode_str = "BULK";
      }  //End if >

      else {
        if (bat_voltage > float_voltage) {
          pwm_value--;
          pwm_value = constrain(pwm_value, 0, 1023);
        }

        else {
          pwm_value++;
          pwm_value = constrain(pwm_value, 0, 1023);
        }
      }  //End else > float_max_current

      Timer1.pwm(PWM_out, pwm_value);
    }
  }  //END of mode == FLOAT



  //Bulk/Absorption
  else {
    if (bat_voltage < bulk_voltage_min) {
      mode = BULK;
      mode_str = "BULK";
    } else if (bat_voltage > bulk_voltage_max) {
      mode_str = "ABSORPTION";
      mode = ABSORPTION;
    }

    ////////////////////////////BULK///////////////////////////
    if (mode == BULK) {
      if (solar_current > charging_current) {
        pwm_value--;
        pwm_value = constrain(pwm_value, 0, 1023);
      }

      else {
        pwm_value++;
        pwm_value = constrain(pwm_value, 0, 1023);
      }
      Timer1.pwm(PWM_out, pwm_value);

    }  //End of mode == BULK


    /////////////////////////ABSORPTION/////////////////////////
    if (mode == ABSORPTION) {
      if (solar_current > absorption_max_current) {  //If we exceed max current value, we reduce duty cycle
        pwm_value--;
        pwm_value = constrain(pwm_value, 0, 1023);
      }  //End if > absorption_max_current

      else {
        if (bat_voltage > absorption_voltage) {
          pwm_value--;
          pwm_value = constrain(pwm_value, 0, 1023);
        }

        else {
          pwm_value++;
          pwm_value = constrain(pwm_value, 0, 1023);
        }

        if (solar_current < absorption_min_current) {
          mode = FLOAT;
          mode_str = "FLOAT";
        }
      }  //End else > absorption_max_current

      Timer1.pwm(PWM_out, pwm_value);
    }  // End of mode == absorption_max_current

  }  //END of else mode == FLOAT

}  //End void loop
