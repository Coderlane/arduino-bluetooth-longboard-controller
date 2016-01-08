#include <Throttle.h>
#include <Connection.h>
#include <HardwareBluetoothRN42.h>

#define LOW_RESISTANCE 10 // 10 Ohms measured
#define HIGH_RESISTANCE 60 // 60 Ohms measured
#define REFERENCE_VOLTAGE 3.3 // 3.3V

const int high_throttle_value = 1023;
const int low_throttle_value = 0;

//Must be analog pin
const int throttle_pin = 23;   // A5

const int status_pin = 2;      // D2
const int status_led_pin = 3;  // D3

long cur_millis = 0;
long prev_millis = 0;
long interval_millis = 250;
int status_led_state = LOW;
int recent_disconnect = 0;

Throttle throttle(throttle_pin, low_throttle_value, high_throttle_value);
HardwareBluetoothRN42 bluetooth(Serial1, status_pin, 0, "BlueController");
Connection connection(bluetooth);

void connection_up();
void connection_lost();
void connection_down();

int percent_to_str(int percent, char *str);

void setup()
{
  delay(1000);
  analogReference(DEFAULT);
  bluetooth.setup();
  pinMode(status_led_pin, OUTPUT);
}

void loop ()
{
  cur_millis = millis();
  if (cur_millis - prev_millis > interval_millis) {
    prev_millis = cur_millis;

    if (bluetooth.connected()) {
      connection_up();
    } else if (recent_disconnect < 50) {
      connection_lost();
    } else {
      connection_down();
    }
  }
}

inline void connection_up()
{
  char str[4];
  // Connection is up.

  recent_disconnect = 0;
  interval_millis = 250;

  int percent = throttle.read();
  // Send the data
  connection.write(percent);

  if (status_led_state != HIGH) {
    // Turn on LED
    digitalWrite(status_led_pin, HIGH);
    status_led_state = HIGH;
  }
}

inline void connection_lost()
{
  // Connection recently lost.
  // Attempt to resend quickly :)

  interval_millis = 500;
  recent_disconnect++;

  // Blink LED
  if (status_led_state == LOW) {
    digitalWrite(status_led_pin, HIGH);
    status_led_state = HIGH;
  } else {
    digitalWrite(status_led_pin, LOW);
    status_led_state = LOW;
  }
}

inline void connection_down()
{
  // Connection has been lost for a while.

  interval_millis = 2000;
  if (status_led_state != LOW) {
    // Turn off LED
    digitalWrite(status_led_pin, LOW);
    status_led_state = LOW;
  }
}

// Change percent to a string w/ zeros in it.
// Str must be char[4] or more.
// Percent should be 0 to 100;
// If it is not, return -1 and set str[0] to the null terminator.
int percent_to_str(int percent, char *str)
{
  int hundreds, tens, ones;
  if (percent < 0 || percent > 100) {
    str[0] = '\0';
    return -1;  
  }
  
  hundreds = percent / 100;
  tens = (percent - (hundreds * 100)) / 10;
  ones = percent - (tens * 10);
  
  str[0] = '0' + hundreds;
  str[1] = '0' + tens;
  str[2] = '0' + ones;
  str[3] = '\0';
  return 0;
}
