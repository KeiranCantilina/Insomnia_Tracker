
/***************************************************
 * Wicked Device Wildfire v4 MQTT-Adafruit.io Example
 * 
 * 
 * This is the Adafruit MQTT Library ESP8266 Example, 
 * modified to work on the Wildfire v4 boards made by WickedDevice
 * (http://shop.wickeddevice.com/) using the WickedDevice ESP8266 AT Client
 * library (found at https://github.com/WickedDevice/ESP8266_AT_Client).
 * 
 * This bit of code serves as an example of how to get the Wildfire v4 to 
 * work with io.adafruit.com using MQTT, since the code shipped with the 
 * boards (supporting the sparkfun.io Phant servers) is now obsolete.
 * 
 * Written/rehashed/butchered by Keiran Cantilina
 * Attribution-NonCommercial licesnse 3.0 United States (CC BY-NC 3.0 US)
 * 
 * Boilerplate from original Adafruit MQTT Library ESP8266 Example code
 * is included below. Please include both the above and below text in any
 * redistribution. 
 * 
 */


/***************************************************
  Adafruit MQTT Library ESP8266 Example

  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board & Feather
  ----> https://www.adafruit.com/product/2471
  ----> https://www.adafruit.com/products/2821

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution

 ****************************************************/
 
 
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h" // Found at https://github.com/adafruit/Adafruit_MQTT_Library
#include <ESP8266_AT_Client.h> // Found at https://github.com/WickedDevice/ESP8266_AT_Client
#include <RGBLED.h> // Found at https://github.com/BretStateham/RGBLED
#include <EnableInterrupt.h>


// sketch must instantiate a buffer to hold incoming data
// 1500 bytes is way overkill for MQTT, but if you have it, may as well
// make space for a whole TCP packet
#define ESP8266_INPUT_BUFFER_SIZE (1500)
uint8_t input_buffer[ESP8266_INPUT_BUFFER_SIZE] = {0};     


/************************* Init LED & switch stuff****************************/

RGBLED rgbLed(9,10,8,COMMON_CATHODE); // 9,10,8 for smd led chip, 8,9,10 for through-hole
int LED_gnd = 11;

#define BUTTON_PIN 24
#define DEBOUNCE_DELAY 100 // in ms


uint32_t last_interrupt_time = 0;
uint8_t led_status = 0; //  Debugging LED flag
volatile int flag = 0;
volatile long times_run = 0;

/************************* WiFi Access Point *********************************/

#define NETWORK_SSID     "WiFi-2.4-DD90"
#define NETWORK_PASSWORD "wfb3a2jdma25c"

// Arduino digital the pin that is used to reset/enable the ESP8266 module
int esp8266_enable_pin = 23; 

// Serial1 is the 'stream' the AT command interface is on
Stream * at_command_interface = &Serial1;  


/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "Keirancantilina"
#define AIO_KEY         "5526f708aeb648ea907a38d95af2b5a2"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
//WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// ORR...Wildfire client object
ESP8266_AT_Client esp(esp8266_enable_pin, at_command_interface); // instantiate the client object


// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&esp, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish photocell = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/keiran-block");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/keiran-block");

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {

  // Save RGB instance data as string to variable (in case it needs to be output for debugging)
  String ledType = (rgbLed.commonType==0) ? "COMMON_CATHODE" : "COMMON_ANODE";
  
  // Setup the button with an internal pull-up :
  pinMode(BUTTON_PIN,INPUT_PULLUP);

  // Setup interrupt handling and LED for debug purposes
  pinMode(LED_BUILTIN, OUTPUT);
  enableInterrupt(BUTTON_PIN, isr_handler, CHANGE); // CHANGE for dome lights, RISING for little button switches

  // This is our fake ground connection for the rgb led
  pinMode(LED_gnd, OUTPUT);
  digitalWrite(LED_gnd, LOW);

  // Start with LED red to show setup
  rgbLed.writeRGB(255,0,0);
  
  Serial.begin(115200);
  delay(10);
  Serial1.begin(115200);              // AT command interface

  // For just in case of network debugging needs
  char ip_str[16] = {0};
  char mac_str[18] = {0};
  uint8_t mac_addr[6] = {0};
  uint32_t ip_addr = 0;
  
  esp.setInputBuffer(input_buffer, ESP8266_INPUT_BUFFER_SIZE); // connect the input buffer up
  esp.reset();                                                 // reset the module
  
  
  Serial.println(F("Keiran's Long-Distance Friendship Cube"));

  // Connect to WiFi access point.
  Serial.print("Set Mode to Station...");
  esp.setNetworkMode(1);
  Serial.println("OK");
  Serial.print("Connecting to ");
  Serial.print(NETWORK_SSID);
  Serial.print("...");
  esp.connectToNetwork(NETWORK_SSID, NETWORK_PASSWORD, 60000, NULL); 
  Serial.println("OK");  
  Serial.println("WiFi connected! Yay");
  
  // Setup MQTT subscription for onoff feed.
  //mqtt.subscribe(&onoffbutton);
}

//Initialize variable used for payload value (RGB color)
volatile long x=0;


void loop() {

  //THis used to be a function but for whatever reason it broke mqtt connection even before it was ever called...somehow it works just pasted in...
  // x = rand_color();

  //Randomly generate RGB values for output as the payload. Values are 0-255 with 111 added to eliminate problems with 0 padding.
  long red_int = random(111,366);
  long green_int = random(111,366);
  long blue_int = random(111,366);

  long million = 1000000;
  long thousand = 1000;
  
  // Ints get concatenated together into the payload "string". For whatever reason strings and chars are no good so this whole thing is a giant long. WHYYYY
  x = 0;
  
  mqtt.subscribe(&onoffbutton);
  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here
  
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();
  

  Adafruit_MQTT_Subscribe *subscription;

  
  
  while (flag == 0 && (subscription = mqtt.readSubscription(1000))) {
    if (subscription == &onoffbutton) {
      Serial.println();
      Serial.print(F("Message recieved: "));
      Serial.println((char *)onoffbutton.lastread);
      Serial.println();
      rgbLed.writeRGB(string_to_red(onoffbutton.lastread)-111,string_to_green(onoffbutton.lastread)-111,string_to_blue(onoffbutton.lastread)-111);
    }  
  }

  //If first time running, turn LED off to indicate ready status
  if (times_run == 0){
    rgbLed.writeRGB(0,0,0);
  }
  
  if (flag == 1){
    //x = rand_color();
    Serial.print(F("\nSending color value: "));
    Serial.print(x);
    Serial.print("...");
    if (! photocell.publish(x)) {
      Serial.println(F("Failed"));
      Serial.println();
    } else {
      Serial.println(F("OK!"));
      Serial.println();
    }
    flag = 0;
  }
    

  Serial.println("No message so far! Will look again...");

  // Increment times run counter
  if(times_run == 0){
    times_run++;
  }
  
  
  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  
//  if(! mqtt.ping()) {
//    mqtt.disconnect();
//  }
  
}




// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT...");

  uint8_t retries = 3;
  
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         rgbLed.writeRGB(255,0,0);   /// Red light for error
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
  Serial.println();
}



// Function to take string with color data and turn it into rgb values
int string_to_red(String color_string){
  String r = color_string.substring(0, 3);
  return r.toInt();
}
int string_to_green(String color_string){
  String g = color_string.substring(3, 6);
  return g.toInt();
}
int string_to_blue(String color_string){
  String b = color_string.substring(6, 9);
  return b.toInt();
}



// Interrupt handling
void isr_handler() {
  uint32_t interrupt_time = millis();
  // Debounce
  if (interrupt_time - last_interrupt_time > DEBOUNCE_DELAY) {
    led_status = !led_status;
    //digitalWrite(LED_BUILTIN, led_status);  // Enable this for visual debugging switch/interrupt stuff
    flag = 1;
  }
  last_interrupt_time = interrupt_time;
}
