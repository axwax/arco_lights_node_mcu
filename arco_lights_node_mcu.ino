#include <Adafruit_NeoPixel.h>

// set to pin connected to data input of WS8212 (NeoPixel) strip
#define PIXEL_PIN   15 // (pin D7)

// any pin with analog input (used to initialize random number generator)
#define POT_PIN     1 // (pin 2)

// number of LEDs (NeoPixels) in your strip
// (please note that you need 3 bytes of RAM available for each pixel)
#define NUMPIXELS   32

// max LED brightness (1 to 255) – start with low values!
// (please note that high brightness requires a LOT of power)
#define BRIGHTNESS  40

// increase to get narrow spots, decrease to get wider spots
#define FOCUS       65

// decrease to speed up, increase to slow down (it's not a delay actually)
#define DELAY       3000


Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

// we have 3 color spots (reg, green, blue) oscillating along the strip with different speeds
float spdr, spdg, spdb;
float offset;

// the real exponent function is too slow, so I created an approximation (only for x < 0)
float myexp(float x) {
  return (1.0/(1.0-(0.634-1.344*x)*x));
}

int alpha = 0;
int reading = 0;

void setup() {
  // initialize pseudo-random number generator with some random value
  randomSeed(analogRead(POT_PIN));

  // assign random speed to each spot
  spdr = 1.0 + random(200) / 100.0;
  spdg = 1.0 + random(200) / 100.0;
  spdb = 1.0 + random(200) / 100.0;

  // set random offset so spots start in random locations
  offset = random(10000) / 100.0;

  // initialize LED strip
  strip.begin();
  strip.show();
}

void loop() {
  // use real time to recalculate position of each color spot
  long ms = millis();
  // scale time to float value
  float m = offset + (float)ms/DELAY;
  // add some non-linearity
  m = m - 42.5*cos(m/552.0) - 6.5*cos(m/142.0);

  // recalculate position of each spot (measured on a scale of 0 to 1)
  float posr = 0.5 + 0.55*sin(m*spdr);
  float posg = 0.5 + 0.55*sin(m*spdg);
  float posb = 0.5 + 0.55*sin(m*spdb);

  // read piezo
  reading  = analogRead(POT_PIN);
  if(reading<50){
    reading=0;
  }  
  alpha = map(reading, 0, 1023, 0, 255);

  // now iterate over each pixel and calculate its color
  for (int i=0; i<NUMPIXELS; i++) {
    // pixel position on a scale from 0.0 to 1.0
    float ppos = (float)i / NUMPIXELS;
 
    // distance from this pixel to the center of each color spot
    float dr = ppos-posr;
    float dg = ppos-posg;
    float db = ppos-posb;

    if(alpha > 50){
      strip.setPixelColor(i,alpha,alpha,0);
    }
    else{
      // set each color component from 0 to max BRIGHTNESS, according to Gaussian distribution
      strip.setPixelColor(i,
        constrain(BRIGHTNESS*myexp(-FOCUS*dr*dr),0,BRIGHTNESS),
        constrain(BRIGHTNESS*myexp(-FOCUS*dg*dg),0,BRIGHTNESS),
        constrain(BRIGHTNESS*myexp(-FOCUS*db*db),0,BRIGHTNESS)
        );
    }
  }

  // send data to LED strip
  strip.show();
}
