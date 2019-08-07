// This #include statement was automatically added by the Particle IDE.
#include <Adafruit_VS1053.h>

// Specifically for use with the Argon with a doubler, the pins are:

const int  MP3_RESET        = -1;                 // VS1053 reset pin (unused!)
const int  SD_CS            = D2;                 // SD Card chip select pin
const int  MP3_CS           = D3;                 // VS1053 chip select pin (output)
const int  DREQ             = D4;                 // VS1053 Data request, ideally an Interrupt pin
const int  MP3_DCS          = D5;                 // VS1053 Data/command select pin (output)

// Controlling the switch registers

const int CD4021_latch = A1;
const int CD4021_clock = A2;
const int CD4021_data = A3;

// Remembering button presses

int last_button = 0; // No buttons yet pressed, EMERGENCY = 1, CALL = 2, others to follow

// Initialise the music player

Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(MP3_RESET, MP3_CS, MP3_DCS, DREQ, SD_CS);
  
// Function declarations

int publishSSIDInfo(String command);
void showEventReceived();

void setup() {
    
    Serial.begin(9600);
    Serial.println("Setting up now...");
    
    // pin modes
    
    pinMode(D7, OUTPUT);
    pinMode(CD4021_latch,OUTPUT);
    
    // public functions
    
    Particle.function("PublishCurrentSSID", publishSSIDInfo);

    // First thing to do is deal with WiFi connections. We want to ensure that it never struggles with connecting.
    
    // Assume the WiFi module is on, it should be.
    
    WiFiAccessPoint ap[10]; // Argon can store up to 10 networks
    int found = WiFi.getCredentials(ap, 10);
    
    // These are the networks we know about right now:
    //
    //  SSID    Workspace1              PW  youareawesome   (default network from setup)
    //  SSID    Factory Phnom Penh      PW  followyourdreams
    //  SSID    L's iPhone              PW  something
    //  SSID    Little Garden           PW  Welcome1615
    //  SSID    Stem Phnom Penh         PW  Welcome2STEMPP
    
    WiFi.setCredentials("STEM Phnom Penh", "Welcome2STEMPP");
    WiFi.setCredentials("Little Garden", "Welcome1615");
    WiFi.setCredentials("L's iPhone", "something");
    WiFi.setCredentials("Factory Phnom Penh", "ihaveideas");
    WiFi.setCredentials("Workspace1", "youareawesome");
    
    Serial.println("Setup complete");
    
    int musicOK = 1;
    if (! musicPlayer.begin()) { // initialise the music player
     Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
     musicOK = 0;
     }

    if (musicOK){
        Serial.println(F("VS1053 found"));
        
        delay(20);
        
        musicPlayer.setVolume(1,1); // lower numbers are louder apparently
 
        delay(20);
 
        musicPlayer.sineTest(0x44, 1500);    // Make a tone to indicate VS1053 is working
    }

}

void loop() {
    
    delay(500);
    readSwitchState();

}

// function definitions

int publishSSIDInfo(String command){
    
    int success = Particle.publish("Current SSID", WiFi.SSID(), PUBLIC);
    
    showEventReceived();
    
    delay(500);
    WiFiAccessPoint aps[10];
    String found = String(WiFi.getCredentials(aps, 10));
    Particle.publish("Known SSIDs", found, PUBLIC);
    
    Serial.println("Publishing SSID");

    return success;
}

void showEventReceived(){
    
    digitalWrite(D7, HIGH);
    delay(200);
    digitalWrite(D7, LOW);
    
}

void readSwitchState(){
    
    digitalWrite(CD4021_latch, HIGH);
    delayMicroseconds(50);
    digitalWrite(CD4021_latch, LOW);
    byte panel1state = shiftIn(CD4021_data,CD4021_clock);
    // additional panels get added here with their caller functions
    byte panel2state = shiftIn(CD4021_data,CD4021_clock);
    printBits(panel1state);
    printBits(panel2state);
    
    if(panel1state){ // If anything has been pressed
        
        if(panel1state&B01001000){
            // this means that the emergency button was pressed
            if(last_button!=1){
                button_emergency();
                last_button=1;
            }
        }
        if(panel1state&B00010001){
            // this means that the call button was pressed
            if(last_button!=2){
                button_call();
                last_button=2;
            }
        }
    }
    
    if(panel2state){ // If anything has been pressed
        
        if(panel2state&B01000000){
            // this means that the button 1 was pressed
            if(last_button!=3){
                button_standard(1);
                last_button=3;
            }
        }
        if(panel2state&B00001000){
            // this means that the button 2 was pressed
            if(last_button!=4){
                button_standard(2);
                last_button=4;
            }
        }
        if(panel2state&B00000001){
            // this means that the button 3 was pressed
            if(last_button!=5){
                button_standard(3);
                last_button=5;
            }
        }
        if(panel2state&B00010000){
            // this means that the button 4 was pressed
            if(last_button!=6){
                button_standard(4);
                last_button=6;
            }
        }
    }
    
}

byte shiftIn(int myDataPin, int myClockPin) {
    
  int i;
  int temp = 0;
  int pinState;
  byte myDataIn = 0;

  pinMode(myClockPin, OUTPUT);
  pinMode(myDataPin, INPUT);

  for (i=7; i>=0; i--)
  {
    digitalWrite(myClockPin, 0);
    delayMicroseconds(0.2);
    temp = digitalRead(myDataPin);
    if (temp) {
      pinState = 1;
      myDataIn = myDataIn | (1 << i);
    }
    else {
      pinState = 0;
    }
    digitalWrite(myClockPin, 1);

  }
  return myDataIn;
}

void printBits(byte myByte){ // helpful do we can see all leading zeroes
 for(byte mask = 0x80; mask; mask >>= 1){
   if(mask  & myByte)
       Serial.print('1');
   else
       Serial.print('0');
 }
 Serial.println();
}

void button_emergency(){
    Particle.publish("Button Press", "EMERGENCY", PUBLIC);
    showEventReceived();
}

void button_call(){
    Particle.publish("Button Press", "CALL", PUBLIC);
    showEventReceived();
}

void button_standard(int number){
    String n = String(number);
    Particle.publish("Button Press", n, PUBLIC);
    showEventReceived();
}