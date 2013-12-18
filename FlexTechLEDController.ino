/**
 * FlexTechLEDController
 *
 * Stephen Paine 2013 (https://sites.google.com/site/smpaine/)
 *
 * Extension of the code found at: http://seeedstudio.com/wiki/index.php?title=Twig_-_Chainable_RGB_LED
 * (page changed in January of 2013; version referenced:
 *   http://seeedstudio.com/wiki/index.php?title=Grove_-_Chainable_RGB_LED&oldid=18188
 * )
 * Controls an LED digitally addressable light sight made by FlexTech: http://www.gemmy.com/christmas/christmas-lighting/item/christmas-lighting/flextech-lightshow-multi-color-87081
 * (page no longer there)
 *
 * Contains extensions (made by me) to the original P9613 controller code from seedstudio (FlexTech light set uses P9618, but appears to be controllable by the P9613 code).
 *
 * Additions (convenience functions) made to simplify controlling the LEDs indiviually, since the controller chips are originally meant to control 3 RGB LEDs, and in this light
 * set they are controlling two groups of 9 LEDs (first chip controls 9 Red & 9 Blue, second chip controls 9 Green & 9 Yellow; pattern repeats for total of 6 controller chips,
 * controlling 3 complete color groups).
 *
 * Also added ability to send commands via Console (telnet localhost 6571 on Yun), allowing control from a computer (or other TTL Console source).
 *
 * Anyone can use this code for whatever purpose they feel like - have fun!
 *
 * 12/18/2013 - Updated for Arduino Yun - using Console instead of Serial now.
 *
 */
 
#include <Console.h>

#define uint8 unsigned char 
#define uint16 unsigned int
#define uint32 unsigned long int

const double versionNum = 1.1;

// For this code, # of leds in light set; must be divisible by 4 and 3!
// 108/4=27 LEDs per color group
// 108/3=36 LEDs per section (each section contains a red, blue, green, and yellow section
const uint16_t numLeds=108;

// Setup pins for Clock and Data
//const int Clkpin = 12;
//const int Datapin = 11;
const int Clkpin = 6;
const int Datapin = 5;

// Variables for storing LED brightness values, and index of each color group into main led array
uint8_t leds[numLeds];
uint16_t rbLeds[numLeds/4], gyLeds[numLeds/4];

// Variables for performing Console commands
uint16_t cmd;
uint8_t brightness;

// These two variables are for controlling the Demo
int isInited=0, demoSection=7, displayedHelp=0;

void ClkProduce(void) {
  digitalWrite(Clkpin, LOW);
  delayMicroseconds(1); 
  digitalWrite(Clkpin, HIGH);
  delayMicroseconds(1);   
}

void Send32Zero(void) {
  unsigned char i;

  for (i=0; i<32; i++) {
    digitalWrite(Datapin, LOW);
    ClkProduce();
  }
}

uint8 TakeAntiCode(uint8 dat) {
  uint8 tmp = 0;

  if ((dat & 0x80) == 0) {
    tmp |= 0x02; 
  }

  if ((dat & 0x40) == 0) {
    tmp |= 0x01; 
  }

  return tmp;
}

// gray data
void DataSend(uint32 dx) {
  uint8 i;

  for (i=0; i<32; i++) {
    if ((dx & 0x80000000) != 0) {
      digitalWrite(Datapin, HIGH);
    } 
    else {
      digitalWrite(Datapin, LOW);
    }

    dx <<= 1;
    ClkProduce();
  }
}

// data processing
void DataDealWithAndSend(uint8 r, uint8 g, uint8 b) {
  uint32 dx = 0;

  dx |= (uint32)0x03 << 30;             // highest two bits 1，flag bits
  dx |= (uint32)TakeAntiCode(b) << 28;
  dx |= (uint32)TakeAntiCode(g) << 26;	
  dx |= (uint32)TakeAntiCode(r) << 24;

  dx |= (uint32)b << 16;
  dx |= (uint32)g << 8;
  dx |= r;

  DataSend(dx);
}

void setLed(uint8_t led, uint8_t brightness) {
  leds[led]=brightness;
}

void refreshLeds() {
  int i;
  Send32Zero();
  for (i=0; i<numLeds; i+=3) {
    /*
    Console.print(i);
     Console.print(", ");
     Console.println(leds[i]);
     Console.print(i+1);
     Console.print(", ");
     Console.println(leds[i+1]);
     Console.print(i+2);
     Console.print(", ");
     Console.println(leds[i+2]);
     */
    DataDealWithAndSend(leds[i], leds[i+1], leds[i+2]);
  }
  Send32Zero();
}

void clearAllLeds() {
  static int i;
  for (i=0; i<numLeds; i++) {
    setLed(i, 0x00);
  }
}

void setGroup(uint16_t *group, uint8_t highOrLow, uint8_t groupLen, uint8_t brightness) {
  int i;

  for (i=0; i<groupLen; i++) {
    if (highOrLow == 1) {
      // Set high-half of 16 bit
      setLed((uint8_t)((group[i] & 0xFF00) >> 8), brightness);
    } 
    else {
      // Set low-half of 16 bit
      setLed((uint8_t)(group[i] & 0xFF), brightness);
    }
  }
}

void setup()  {
  static int i,j;

  pinMode(Datapin, OUTPUT);
  pinMode(Clkpin, OUTPUT);

  // Allocate LED array memory - stores brightness for each LED
  //if (NULL != (leds = (uint8_t *)malloc(numLeds))) {
  memset(leds, 0x00, numLeds); // Init to brightness 0 (off)
  //}

  int numGroups=(numLeds/4)/9;

  //Allocate color groups - store LED array index of all of each color

  for (i=0; i<numGroups; i++) {
    for (j=0; j<9; j++) {
      rbLeds[(i*9)+j] = ((((i*9*4)+j) << 8) & 0xFF00);
    }
  }

  for (i=0; i<numGroups; i++) {
    for (j=0; j<9; j++) {
      rbLeds[(i*9)+j] |= (((i*9*4)+j+9) & 0xFF);
    }
  }

  for (i=0; i<numGroups; i++) {
    for (j=0; j<9; j++) {
      gyLeds[(i*9)+j] = ((((i*9*4)+j+18) << 8) & 0xFF00);
    }
  }

  for (i=0; i<numGroups; i++) {
    for (j=0; j<9; j++) {
      gyLeds[(i*9)+j] |= (((i*9*4)+j+27) &0xFF);
    }
  }

  // initialize serial communication:
  Bridge.begin();
  Console.begin();
}

void displayHelp() {
  // Print out useful information
  Console.print("FlexTechLEDController (for RGBY set) Version ");
  Console.println(versionNum);
  Console.println("\r\nStephen Paine 2012");
  Console.print("\r\nNumber of LEDs: ");
  Console.println(numLeds);
  Console.println("\r\n\r\nCommands are given as <cmdCode>,<brightness (all but clear/demo)>,...\\n");
  Console.println("\r\n Command Codes are:");
  Console.println("\t0xC1A4 (49572)\tClears all LEDs (turns them off)");
  Console.println("\t0xDE30 (56880)\tBegins/Returns to Demo Mode");
  Console.println("\t0x04ED (1261)\tSets all Red LED groups");
  Console.println("\0xB10E (45326)\tSets all Blue LED groups");
  Console.println("\0x64E7 (25831)\tSets all Green LED groups");
  Console.println("\0x9E10 (40464)\tSets on all Yellow LED groups");
  Console.println("\t<LED #>\t\tSets given LED # (ex: 0 sets first LED)");
  Console.println("\r\n\r\nExample command:\r\n49572,1261,255,25831,50\\n");
  Console.println("would clear all, set all red groups to full brightness, and set all green groups to a brightness of 50.\r\n");
  Console.println("Ready\r\n");
}

void loop()  {
  if (!displayedHelp && !isInited && Console) {
    displayHelp();
    displayedHelp=1;
  }
  
  //static int i,j;

  // if there's any Console available, read it:
  while (Console.available() > 0) {
    isInited=1;

    if (Console.available() <= 2) {
      if (Console.peek() == '\n') {
        Console.read();
        displayHelp();
        continue;
      }

      if (Console.peek() == '\r') {
        Console.read();
        displayHelp();
        continue;
      }
    }

    if (Console.peek() < 48 || Console.peek() > 57) {
      Console.read();
      displayHelp();
      continue;
    }

    cmd=Console.parseInt();

    Console.print("Received command: ");
    Console.print(cmd, HEX);
    Console.print("\r\n");

    if (cmd==0xC1A4) {
      //49572
      //Clear leds (all off)
      clearAllLeds();
    } 
    else if (cmd==0xDE30) {
      //56880
      // Go back to demo mode
      isInited=0;
      demoSection=0;
    }
    else {
      brightness=Console.parseInt();
      if (brightness<0 || brightness>255) {
        brightness=255;
      }
      Console.print("Received brightness: ");
      Console.print(brightness, HEX);
      Console.print("\r\n");

      if (cmd==0x04ED) {
        //1261
        // turn on all red
        setGroup(rbLeds, 1, numLeds/4, brightness);
      } 
      else if (cmd==0xB10E) {
        //45326
        // turn on all blue
        setGroup(rbLeds, 0, numLeds/4, brightness);
      } 
      else if (cmd==0x64E7) {
        //25831
        // turn on all green
        setGroup(gyLeds, 1, numLeds/4, brightness);
      } 
      else if (cmd==0x9E10) {
        //40464
        // turn on all yellow
        setGroup(gyLeds, 0, numLeds/4, brightness);
      } 
      else {
        setLed(cmd, brightness);
      }
    }

    // look for the newline. That's the end of the command
    if (Console.read() == '\n') {
      refreshLeds();
    }
  }

  if (!isInited) {
    demo();
  }

}

void demo() {
  static int led = 0, i, j;
  if (demoSection==0) {
    for (i=0; i<10; i++) {
      for (j=0; j<numLeds; j++) {
        setLed(j, 0xFF);
      }
      refreshLeds();
      delay(500);
      for (j=0; j<numLeds; j++) {
        setLed(j, 0x00);
      }
      refreshLeds();
      delay(500);
    }
    demoSection++;
  } 
  else if (demoSection==1) {
    for (i=0; i<numLeds; i++) {
      setLed(i, 0xFF);
      refreshLeds();
      delay(5);
    }
    delay(50);
    demoSection++;
  } 
  else if (demoSection==2) {
    for (i=numLeds-1; i>=0; i--) {
      setLed(i, 0x00);
      refreshLeds();
      delay(5);
    }
    delay(50);
    demoSection++;
  } 
  else if (demoSection==3) {
    for (i=numLeds-1; i>=0; i--) {
      setLed(i, 0x00);
      setLed(i, 0xFF);
      refreshLeds();
      delay(5);
    }
    delay(50);
    demoSection++;
  } 
  else if (demoSection==4) {
    for (i=0; i<numLeds; i++) {
      setLed(i, 0x00);
      setLed(i, 0x00);
      refreshLeds();
      delay(5);
    }
    delay(50);
    demoSection++;
  } 
  else if (demoSection==5) {
    // Fade through all LEDs (this will be a long one, so
    // we will return back to main after each led to
    // not block serial for too long
    for (i=0; i<255; i+=51) {
      setLed(led, i);
      refreshLeds();
    }
    for (i=255; i>=0; i-=51) {
      setLed(led, i);
      refreshLeds();
    }
    if (led<numLeds) {
      led++;
    } 
    else {
      led=numLeds-1;
      demoSection++;
    }
  } 
  else if (demoSection==6) {
    // Fade through all LEDs (this will be a long one, so
    // we will return back to main after each led to
    // not block serial for too long
    for (i=0; i<255; i+=51) {
      setLed(led, i);
      refreshLeds();
    }
    for (i=255; i>=0; i-=51) {
      setLed(led, i);
      refreshLeds();
    }
    if (led>0) {
      led--;
    } 
    else {
      led=0;
      demoSection++;
    }
   } else if (demoSection==7) {
    // Pulse through color groups
    clearAllLeds();
    refreshLeds();
    delay(100);
    setGroup(rbLeds, 1, numLeds/4, 255);
    refreshLeds();
    delay(100);
    setGroup(rbLeds, 0, numLeds/4, 255);
    refreshLeds();
    delay(100);
    setGroup(gyLeds, 1, numLeds/4, 255);
    refreshLeds();
    delay(100);
    setGroup(gyLeds, 0, numLeds/4, 255);
    refreshLeds();
    delay(100);
    setGroup(rbLeds, 1, numLeds/4, 0);
    refreshLeds();
    delay(100);
    setGroup(rbLeds, 0, numLeds/4, 0);
    refreshLeds();
    delay(100);
    setGroup(gyLeds, 1, numLeds/4, 0);
    refreshLeds();
    delay(100);
    setGroup(gyLeds, 0, numLeds/4, 0);
    refreshLeds();
    delay(500);
    demoSection++;
  }
  else if (demoSection<12) {
    // Fade all LEDs on, then off
    for (i=0; i<255; i+=1) {
      for (j=0; j<numLeds; j++) {
        setLed(j, i);
      }
      refreshLeds();
    }
    for (i=255; i>=0; i-=1) {
      for (j=0; j<numLeds; j++) {
        setLed(j, i);
      }
      refreshLeds();
    }
    demoSection++;
  } 
  else {
    demoSection=0;
  }
}

