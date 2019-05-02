/*
#include <deprecated.h>
#include <MFRC522.h>
#include <MFRC522Extended.h>
#include <require_cpp11.h>
*/

#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <RFID.h>
#include <Wire.h>

int card[] = {"5B1A3EE71","1316201B3E","37A3803420","B79974346E","179A82343B", "7919847A7"}; //array of allowed codes
int nameID[] = {"Gioele","Anna","Arturo","Riccardo","Lorenzo", "Gioele"}; //array for user name (order index)
int countEntry[] = {0,0,0,0,0};

#define greenLed 6 //external panel green warning light (ON when access is allowed)
#define redLed 7 //external panel red warning light (ON when access is denied)
#define OutbuzzerPin 8 //external panel buzzer pin
#define InbuzzerPin 4 //internal panel buzzer pin
#define SDA_DIO 10 //SDA pin of RFID
#define RST_DIO 9  //RST pin of RFID
#define lockRelay 5 //locker relay coil pin
#define callLed 2 //internal panel warning light for call
#define allarmPin 3 //external allarm relay coil pin

const int callButton = A0; //external switch for call (entry request)
const int openButton = A1; //internal switch for open the locker
const int deniedButton = A2; //internal switch for deny the call
const int abilitationSwitch = A3; //internal switch for enable the system

LiquidCrystal_I2C lcd(0x27,16,2); 
RFID RC522(SDA_DIO, RST_DIO);

void setup(){
	Serial.begin(9600);
	lcd.init();
	lcd.backlight();
	lcd.clear();
	SPI.begin();
	RC522.init();

	pinMode(greenLed, OUTPUT);
	pinMode(redLed, OUTPUT);
	pinMode(lockRelay, OUTPUT);
	pinMode(allarmPin, OUTPUT);
	pinMode(callLed, OUTPUT);

	digitalWrite(lockRelay, LOW); //force the state of the locker relay at low

	lcd.setCursor(3,0);
	lcd.print("RFID INIT");
	lcd.setCursor(1,1);
	lcd.print("STARTING SETUP");
	delay(2000);
}

void loop(){

	byte i;	

	if(analogRead(abilitationSwitch)>=500){ //check that the enabler is active 	
		lcd.clear();
			
		if(analogRead(callButton)>=500){ //check if the call button is active (for user without RFID card)
			lcd.clear();
			lcd.setCursor(0,0);
			lcd.print("SENDING CALL....");
			lcd.setCursor(0,1);
			lcd.print("  PLEASE  WAIT  ");
			delay(100);
			digitalWrite(callLed, HIGH); //turn on the call warning light for alert the operator

			for(int z=0; z=5; z++){ //generate a sound signal for alert the operator
				tone(InbuzzerPin, 1000, 500);
				tone(InbuzzerPin, 700, 500);
			}
			delay(1500);

			if(analogRead(openButton)>=500){ //check if the operator allow the entry
				tone(InbuzzerPin, 800, 300);
				tone(InbuzzerPin, 900, 300);
				digitalWrite(lockRelay, HIGH); //turn on locker relay coil pin for 400ms 
				delay(400);
				digitalWrite(lockRelay, LOW);
				approvedCommand(); //run "approvedCommand" function
				lcd.clear();
				lcd.setCursor(0,0);
				lcd.print("ACCESS  ALLOWED!");
				lcd.setCursor(0,1);
				lcd.print("YOU ARE WELCOME!");
				delay(2000);
				lcd.clear();
			}
			else if(analogRead(deniedButton)>=500){ //check if the operator deny the entry
				tone(InbuzzerPin, 800, 300);
				tone(InbuzzerPin, 900, 300);
				deniedCommand(); //run "deniedCommand" function
				lcd.clear();
				lcd.setCursor(0,0);
				lcd.print(" ACCESS  DENIED ");
				lcd.setCursor(0,1);
				lcd.print("KEEP UR ASS OUT!");
				delay(2000);
				lcd.clear();
			}
			digitalWrite(callLed, LOW); //turn off the call warning light 
		delay(300);
		}

		else{		
			lcd.setCursor(0,0);
			lcd.print("PRESENT THE CARD");
			lcd.setCursor(0,1);
			lcd.print(" TO RFID READER ");
		}
		
		if(RC522.isCard()){ //verify the presence of a card on the RFID reader
			lcd.clear();
			lcd.setCursor(0,0);
			lcd.print("   READING...   ");
			lcd.setCursor(0,1);
			lcd.print("  PLEASE  WAIT  ");
			delay(1000);

			RC522.readCardSerial(); //read card code
			String readCode ="";
			Serial.print("Card code: "); 

			//convert the read code in hexadecimal uppercase
			for(int i=0; i<6; i++){
				readCode+= String(RC522.serNum[i], HEX);
				readCode.toUpperCase();
			}
		
			Serial.println(readCode); //print converted code in Serial monitor
			Serial.println();

			for(int n=0; n<6; n++){ //increase the value of array index (from 0 to 5)
				if(readCode == card[n]){ //check if the converted code is equal to one of alowed code (define by array)
					Serial.print("Code OK");
					lcd.clear();
					lcd.setCursor(0,0);
					lcd.print(" CARD  APPROVED ");
					lcd.setCursor(0,1);
					lcd.print("WELCOME ");
					lcd.print(nameID[n]);
					countEntry[n] = countEntry[n] +1; //increase the value of entry counter 
					approvedCommad();
					entryCounter(); //run "entryCounter" function
					digitalWrite(lockRelay, HIGH); //turn on locker relay coil pin for 400ms
					delay(400);
					digitalWrite(lockRelay, LOW);
					delay(15);
				}
				else if(readCode != card[0] && readCode != card[1] && readCode != card[2] && readCode != card[3] && readCode != card[4] && readCode != card[5]){ //ceck if converted code is different about all the alowed code
					lcd.clear();
					lcd.setCursor(0,0);
					lcd.print("  CARD  DENIED  ");
					lcd.setCursor(0,1);
					lcd.print("KEEP UR ASS OUT!");
					deniedCommand();
				}
				delay(10);
			}
		}
	}
	else{
		digitalWrite(lockRelay, LOW); //force the state of the locker relay at low
		lcd.setCursor(0,0);
		lcd.print("SORRY, SYSTEM IS");
		lcd.setCursor(0,1);
		lcd.print(" OUT OF SERVICE ");
	}
}

void deniedCommand(){
	for(int x=0; x<6; x++){
		digitalWrite(redLed, HIGH);
		digitalWrite(greenLed, LOW);
		tone(OutbuzzerPin, 4000, 350);
		delay(500);
		digitalWrite(redLed, LOW);
		digitalWrite(greenLed, HIGH);
		tone(OutbuzzerPin, 4000, 350);
		delay(500);
	}
	digitalWrite(redLed, LOW);
	digitalWrite(greenLed, LOW);
}

void approvedCommand(){
	digitalWrite(redLed, LOW);
	digitalWrite(greenLed, HIGH);
	tone(OutbuzzerPin, 2000, 250);
	delay(100);
	tone(OutbuzzerPin, 2500, 250);
	delay(100);

	delay(1000);
	digitalWrite(redLed, LOW);
	digitalWrite(greenLed, LOW);
}

void entryCounter(){
	for(int n=0; n<6; n++){
		Serial.print(nameID[n]);
		Serial.print(": ");
		Serial.println(countEntry[n]);
		delay(1000);
	}
}