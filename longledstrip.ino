#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <FastLED.h>
#include <RemoteDebug.h>  //https://github.com/JoaoLopesF/RemoteDebug
#include <DNSServer.h>
#include <ESP8266mDNS.h>

#define STASSID "**REMOVED**"
#define STAPSK  "**REMOVED**"
#define HOSTNAME "longLEDstrip"
#define USE_MDNS true

#define DEBUG_DISABLED


// Gradient palette "bhw1_01_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_01.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 12 bytes of program space.

DEFINE_GRADIENT_PALETTE( bhw1_01_gp ) {
    0, 227,101,  3,
  117, 194, 18, 19,
  255,  92,  8,192};

// Gradient palette "Sunset_Real_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Sunset_Real.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( Sunset_Real_gp ) {
    0, 120,  0,  0,
   22, 179, 22,  0,
   51, 255,104,  0,
   85, 167, 22, 18,
  135, 100,  0,103,
  198,  16,  0,130,
  255,   0,  0,160};

// Gradient palette "bhw1_purplered_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_purplered.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 8 bytes of program space.
// Now modified by me, so not the original above.

DEFINE_GRADIENT_PALETTE( bhw1_purplered_gp ) {
    0, 255,  0,  0,
  128, 214, 81, 255,
  255, 255,  0, 255};

// Gradient palette "temperature_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/arendal/tn/temperature.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 144 bytes of program space.

DEFINE_GRADIENT_PALETTE( temperature_gp ) {
    0,   1, 27,105,
   14,   1, 40,127,
   28,   1, 70,168,
   42,   1, 92,197,
   56,   1,119,221,
   70,   3,130,151,
   99,  67,182,112,
  113, 121,201, 52,
  127, 142,203, 11,
  141, 224,223,  1,
  155, 252,187,  2,
  170, 247,147,  1,
  198, 229, 43,  1,
  212, 220, 15,  1,
  226, 171,  2,  2,
  255,  80,  3,  3};

// Gradient palette "GMT_hot_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/gmt/tn/GMT_hot.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 16 bytes of program space.

DEFINE_GRADIENT_PALETTE( GMT_hot_gp ) {
    0,   0,  0,  0,
   95, 255,  0,  0,
  191, 255,255,  0,
  255, 255,255,255};

// Gradient palette "cw1_030_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/cw/1/tn/cw1-030.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 16 bytes of program space.

DEFINE_GRADIENT_PALETTE( cw1_030_gp ) {
    0,   1,  2, 44,
   84,   9, 11,112,
  168,  77, 19,105,
  255, 255,100,137};

// Gradient palette "cw1_002_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/cw/1/tn/cw1-002.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 16 bytes of program space.

DEFINE_GRADIENT_PALETTE( cw1_002_gp ) {
    0,   2, 29, 52,
   84,  73,125,160,
  168, 239, 77, 43,
  255, 190, 27, 22};

// Gradient palette "500_LOVERS_ThankYou_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/colo/adgrapho/tn/500_LOVERS_ThankYou.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 40 bytes of program space.

DEFINE_GRADIENT_PALETTE( x500_LOVERS_ThankYou_gp ) {
    0, 222, 10,  7,
   51, 222, 10,  7,
   51, 255, 71, 21,
  102, 255, 71, 21,
  102, 242,173, 47,
  153, 242,173, 47,
  153, 184,233, 91,
  204, 184,233, 91,
  204,  39,182,137,
  255,  39,182,137};




RemoteDebug Debug;

const char* ssid = STASSID;
const char* password = STAPSK;

#define num_leds 300
#define sample_length 300  // for adjusting sensitivity on the fly to varying sound levels
#define brightness 150     // can set 0-255

CRGB leds[num_leds];
CRGB copy_leds[num_leds];
int readings[sample_length];   // array of readings to calculate moving min/max
int read_index = 0;            // the index of the current reading
unsigned long last_flash = 0;

CRGBPalette16 currentPalette;
int palette_index = 0;
CRGBPalette16 palettes[] = {
  RainbowColors_p,
  PartyColors_p,
  OceanColors_p,
  LavaColors_p,
  bhw1_purplered_gp,
  temperature_gp,
  GMT_hot_gp,
  bhw1_01_gp,
  Sunset_Real_gp,
  x500_LOVERS_ThankYou_gp,
  cw1_002_gp,
  cw1_030_gp
};
TBlendType    currentBlending;

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");

  setup_wifi();
  setup_OTA();
  setup_debug();
  
  // initialise LED strip
  FastLED.addLeds<WS2812B,6,GRB>(leds, num_leds);
  FastLED.clear();
  FastLED.setBrightness(255);
  
  currentPalette = palettes[palette_index];
  currentBlending = LINEARBLEND;
  
  // "blank" the reading sample list
  for (int a = 0; a < num_leds; a++) {
    readings[a] = 20;
  }
}

// measure peak envelope in last 15 secs
int moving_max = 1;
int moving_min = 255;
  
void loop() {
  ArduinoOTA.handle();
  Debug.handle();

  // give me time to connect debugger
  if (millis() < 12000) {
    debugV("waiting for connection");
    delay(100);
    return;
  }

  // BUGGY RIGHT NOW 
  EVERY_N_MILLISECONDS(30000) {
    change_palette_periodically();
  }

  // sample the microphone (using a sparkfun sound detector, with envelope connected to A0)
  int x = analogRead(A0);
 
  debugV("Audio level: %d", x);
  int old_reading = readings[read_index];
  readings[read_index] = x;
  // advance to the next position in the array:
  read_index++;
  // if we're at the end of the array...
  if (read_index >= sample_length) {
    // ...wrap around to the beginning:
    read_index = 0;
  }
  
  // suboptminal - need to implement sliding window algorithm
  if (x <= moving_min) {
    moving_min = x;
  } else if (old_reading == 0 || old_reading <= moving_min) { // only rescan the sample window if you just booted out the lowest value
    debugV("re-scanning for moving min");
    moving_min = 1024;
    for (int z = 0; z < sample_length; z++) {
      moving_min = min(moving_min, readings[z]);
    }
  }
  if (x >= moving_max) {
    moving_max = x;
  } else if (old_reading == 0 || old_reading >= moving_max) { // only rescan the sample window if you just booted out the highest value
    debugV("re-scanning for moving max");
    moving_max = 1;
    for (int z = 0; z < sample_length; z++) {
      moving_max = max(moving_max, readings[z]);
    }
  }

  debugV("moving_min: %d, moving_max: %d", moving_min, moving_max);

  // TODO if min and max are too close, spread them out a bit? 

  // temporary for messing with, make min + 50 the floor for max
  moving_max = max(moving_min + 50, moving_max);
  
  // map the hue between 0 and 255 based on the rolling min/max to keep something interesting on display
  int hue = map(x, moving_min, moving_max, 0, 255);
  
  debugV("Hue: %d", hue);

  if (hue > 235 && millis() - last_flash >= 300) { // don't flash for every sample - no more than once every 300ms
    // I like the idea of a bright flash for peak noise.... 
    // backup the led state as-is
    memmove( &copy_leds[0], &leds[0], num_leds * sizeof( CRGB) );

    // set one in every 4 LEDs to white, for a brief flash
    for (int j = 0; j < num_leds - 4; j += 4) {
      leds[j] = CRGB(255, 255, 255);
    }
 
    FastLED.show();
    delay(10);
    // reset the strp back to prior state - so the white just flashes on and off without scrolling
    memmove( &leds[0], &copy_leds[0], num_leds * sizeof( CRGB) );

    //leds[(num_leds / 2)] = CHSV(hue, 255, brightness);
    leds[(num_leds / 2)] = ColorFromPalette( currentPalette, hue, 255, currentBlending);
    last_flash = millis();
  } else if (hue < 10) {
    // maybe black for really quiet?
    leds[(num_leds / 2)] = CRGB(0, 0, 0);
  } else {
    // then just use hue spectrum for colour
    // vary brightness a bit by the hue factor/volume level as well
    int bness = map(hue, 0, 255, 50, 255);
//    int dim_factor = 255 / (moving_max - moving_min);
 //   bness /= dim_factor;
 
 //   leds[(num_leds / 2)] = CHSV(hue, 255, bness);
    leds[(num_leds / 2)] = ColorFromPalette( currentPalette, hue, bness, currentBlending);

  }
  // TODO when it's quiet and there's little differentiation in volume - mute the brightness?? TODO
  
  FastLED.show();
  for (int z = num_leds - 1; z > (num_leds/2); z--) {
    leds[z] = leds[z - 1];
  }
  for (int z = 0; z < (num_leds/2); z++) { // todo check whether that -1 is required....
    leds[z] = leds[z + 1];
  }
  delayToSyncFrameRate(60);
}

// delayToSyncFrameRate - delay how many milliseconds are needed
//   to maintain a stable frame rate.
static void delayToSyncFrameRate( uint8_t framesPerSecond)
{
  static uint32_t msprev = 0;
  uint32_t mscur = millis();
  uint16_t msdelta = mscur - msprev;
  uint16_t mstargetdelta = 1000 / framesPerSecond;
  if( msdelta < mstargetdelta) {
    delay( mstargetdelta - msdelta);
  }
  msprev = mscur;
}

void setup_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  WiFi.hostname(HOSTNAME);

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); 
}

// boilerplate for setting up over the air firmware updates
void setup_OTA() {

  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname(HOSTNAME);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

   Serial.println("Start updating " + type);

  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
   Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  

}

// boilerplate for setting up remote (wifi) debugging
void setup_debug() {
  if (MDNS.begin(HOSTNAME)) {
      Serial.print("* MDNS responder started. Hostname -> ");
      Serial.println(HOSTNAME);
  }

  MDNS.addService("telnet", "tcp", 23);
  
  Debug.begin(HOSTNAME); // Initialize the WiFi debugging/logging server
  Debug.setResetCmdEnabled(true); // Enable the reset command
  Debug.showProfiler(true); // Profiler (Good to measure times, to optimize codes)
  Debug.showColors(true); // Colors
  Debug.setSerialEnabled(true); // All messages send to serial too, and can be seen in serial monitor

  Serial.println("* Arduino RemoteDebug Library");
  Serial.print("* WiFI connected. IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("* Please use the telnet client (telnet for Mac/Unix or putty and others for Windows)");
  Serial.println("* or the RemoteDebugApp (in browser: http://joaolopesf.net/remotedebugapp)");
}


void change_palette_periodically() {
  palette_index++;
  if (palette_index >= sizeof(palettes) / sizeof(CRGBPalette16)) {
    palette_index = 0;
  }
  
  currentPalette = palettes[palette_index];
}
