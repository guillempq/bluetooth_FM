/////////////// CONNEXIONS //////////////////
// RADIO: RESET-12 SCL-A5 SDA-A4 Vin GND
// BLUETOOTH: TX-3 RX-2
/////////////////////////////////////////////

#include <SoftwareSerial.h>
#include <Wire.h>
#include <bc127.h>
#include <Adafruit_Si4713.h>

SoftwareSerial swPort(3,2);  // RX, TX
BC127 BTModu(&swPort);

#define RESETPIN 12
#define FMSTATION 8750      // 10230 == 102.30 MHz
Adafruit_Si4713 radio = Adafruit_Si4713(RESETPIN);

//Variables millis()
long previousMillis = 0;
long interval = 500;

// Variable pels boto
const int button_play_pin = 10;
int button_play_state = HIGH;
int button_play_mode = 0; // 0=pause, 1=play

void setup()
{
  Serial.begin(9600);
  Serial.println("TEST: bluetooth i transmissor FM (Si4713)");

  swPort.begin(9600);
  pinMode(button_play_pin, INPUT);
  
  if (! radio.begin()) {  // begin with address 0x63 (CS high default)
    Serial.println("Couldn't find radio?");
    while (1);
  }
  
  // Estat del bluetooth
  BTTest();
  
  // Estat FM
  FMTest();
}

void loop()
{
  button_play_state = digitalRead(button_play_pin);
  
  if(millis() - previousMillis >= interval) {
    if (button_play_state == LOW){
      BTModu.musicCommands(BC127::UP);
      /*
      Serial.println(millis() - previousMillis);
      previousMillis = millis();

      if (button_play_mode == 0) 
        button_play_mode = 1;
      else
        button_play_mode = 0;
        
      play(button_play_mode);
      */
    }
    // save the last time 
  }
}

void BTTest()
{
  int connectionResult = 0;
  String address;
  
  Serial.print("Inquiry result: ");
  Serial.println(BTModu.inquiry(5));
  
  for (byte i = 0; i < 5; i++)
  {
    
    // Gets an address from the array of stored addresses. The return value allows
    // the user to CHECK on whether there was in fact a valid address at the
    // requested index
    if (BTModu.getAddress(i, address))
    {
      Serial.print("\tDispositiu trobat a l'address: ");
      Serial.println(address);
      if (address.startsWith("18DC5")) // PC=C01885D9598B Android Guillem=18DC56F97243
      {
        Serial.print(String("\tAndroid ") + address + 
               String(", trobat a l'index ") + 
               BTModu.getAddress(i, address) + "\n"); 
        //Serial.println(i);
        Serial.println("Estat: " + BTModu.connectionState());
        Serial.print("Resultat de la connexio: ");
        
        // Attempts to connect to one of the Bluetooth devices which has an address
        // stored in the _addresses array.
        // connect(char index, connType connection); 
        // connect(String address, connType connection); 
            // Connection: SPP, BLE, 
            // A2DP, AVRCP, HFP, PBAP
        connectionResult = BTModu.connect(i, BC127::AVRCP);
        Serial.println(connectionResult);
        break;
      }
    }
  }
  if (connectionResult == 0)
    Serial.println("----------No trobat!---------");
  else
  {
    if (connectionResult == 1)
    {
      Serial.print("Entering data mode...");
      if (BTModu.enterDataMode()) 
      {
        Serial.println("OK!");
        swPort.println("Hello, world!");
        swPort.flush();
        delay(500);
        Serial.print("Exiting data mode...");
        if (BTModu.exitDataMode()) Serial.println("OK!");
        else Serial.println("Failure!");
      }
      else Serial.println("Failure!");
    }
  }
  
}

void FMTest()
{
  Serial.print("\nSet TX power");
  radio.setTXpower(115);  // dBuV, 88-115 max

  Serial.print("\nTuning into "); 
  Serial.print(FMSTATION/100); 
  Serial.print('.'); 
  Serial.println(FMSTATION % 100);
  radio.tuneFM(FMSTATION); // 102.3 mhz

  // This will tell you the status in case you want to read it from the chip
  radio.readTuneStatus();
  Serial.print("\tCurr freq: "); 
  Serial.println(radio.currFreq);
  Serial.print("\tCurr freqdBuV:"); 
  Serial.println(radio.currdBuV);
  Serial.print("\tCurr ANTcap:"); 
  Serial.println(radio.currAntCap);

  // begin the RDS/RDBS transmission
  radio.beginRDS();
  radio.setRDSstation("On Live");
  radio.setRDSbuffer("Coneccio amb el mobil!");

  Serial.println("RDS on!");  

  radio.setGPIOctrl(_BV(1) | _BV(2));  // set GP1 and GP2 to output
}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
void play(int mode)
{
  // mode 0=pause, mode 1=play
  if (mode == 0){
    BTModu.musicCommands(BC127::PAUSE);
    Serial.println("Pause");
  } else {
    BTModu.musicCommands(BC127::PLAY);
    Serial.println("Play");
  }
}
