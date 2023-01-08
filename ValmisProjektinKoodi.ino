
#include <Keypad.h>               //näppäimistö kirjasto
#include <LiquidCrystal.h>        //LCD-näytön kirjasto
#include <SPI.h>                  //SPI väylä RFID ja LCD-näyttöä varten
#include <RFID.h>                 //RFID kirjasto
 
#define SDA_DIO 9                 //RFID:n SDA pinni
#define RESET_DIO 10              //RFID:n Reset pinni

int calibrationTime = 15;         //Asetetaan PIR liiketunnistimen kalibrointi aika
int Motionsensor_PIN = 7;         //PIR liiketunnistimen digital pin arduinossa
int Summeri_PIN = 22;             //Summerin digital pin arduinossa
int Led = 53;                     //Ledin digital pin arduinossa



//LCD-näytön pinnit
const int rs = 6, en = 8, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);          


const byte ROW_NUM = 4;           //Vaakasuorien rivien määrä
const byte COLUMN_NUM = 3;        //Pystysuorien rivien määrä

//näppäimistön layout
char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte pin_rows[ROW_NUM] = {14, 15, 16, 17};  //Kytketään näppäimistön vaakasuorat rivit
byte pin_column[COLUMN_NUM] = {18, 19, 20}; //Kytketään näppäimistön pystysuorat rivit

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

const String password = "1234";             //näppäimistön salasana
String input_password;                      //näppäimistöltä luettu salasana

RFID RC522(SDA_DIO, RESET_DIO);             //Kerrotaan RFID:lle SDA ja Reset pinnit


boolean alarmset=false;                     //hälytystila pois
bool alarm=false;                           //hälytys pois


void setup() {
 Serial.begin(9600);                            //käynnistetään Serial monitor 9600 Baudinopeudella
  pinMode(Motionsensor_PIN, INPUT);             //asetetaan digitalpinnistä INPUT
  pinMode(Led, OUTPUT);                         //Asetetaan digitalpinnistä OUTPUT
  SPI.begin();                                  // Käynnistetään SPI väylä
  RC522.init();                                 //käynnistetään RFID lukija
    Serial.print("calibrating sensor ");
    for(int i = 0; i < calibrationTime; i++){
      Serial.print(".");                        //Tulostetaan serial monitorille . calibroinnin aikana sekunnin välein kalibroinnin aikana
      delay(1000);
      }
    Serial.println(" done");                    //Tulostetaan serial monitorille
    Serial.println("SENSOR ACTIVE");            //Tulostetaan serial monitorille
    delay(50);
    
    
  input_password.reserve(32);                   //Maksimissaan voi syöttää 33 merkkiä näppäimistöltä
  lcd.begin(16, 2);                             //Käynnistetään LCD-näyttö
  lcd.home();
  lcd.print("alarm not set");                   //Tulostetaan LCD-näytölle
  
  
  
  
}



void loop() {

int Liikkeentila = digitalRead(Motionsensor_PIN);   //Luetaan liiketunnistinta

  rfid();                                           //Luetaan RFID käyttäen alla olevaa aliohjelmaa
  salasana();                                       //Luetaan näppäimistöä käyttäen alla olevaa aliohjelmaa
  
  
if(alarm==true){                                    //Hälytyksen tullessa laitetaan summeri ja Ledi päälle
  lcd.clear();                                      //Tyhjennetään LCD-näyttö
  lcd.print("motion detected");                     //Kirjoitetaan LCD-näytölle
  lcd.setCursor(1,1);                               //Siirretään LCD-näytön osoittajaa
  lcd.print("read the card");                       //Kirjoitetaan näytölle uudestaan
      delay(200);
      tone(Summeri_PIN, 800);                       //Summeri päälle
      delay(2000);                                  //Summeri soi 2s
      noTone(Summeri_PIN);                          //Summeri pois päältä
      
}
  
if(alarmset == false){
  if(alarm==false){
  lcd.clear();                                      //Tyhjennetään LCD-näyttö
  lcd.print("alarm OFF");                           //Tulostetaan LCD-näytölle
  digitalWrite(Led, LOW);                           //Ledi poist päältä
  }
}
 
if (alarmset == true){                              //Onko hälytystila päällä
  if(alarm==false){                                 //Onko hälytys jo päällä
 
  lcd.clear();                                      //Tyhjennetään LCD-näyttö
  lcd.print("alarm set");                           //Tulostetaan LCD-näytölle
  
    if (Liikkeentila == HIGH) {
     digitalWrite(Led, HIGH);                       //Ledi päälle
      Serial.println("Liike havaittu");             //Tulostetaan LCD-näytölle
      alarm=true;                                   //Asetetaan hälytys päälle
    
    }
}     
  }

    }



void rfid()
{
 
  if (RC522.isCard())                               //Luetaan onko korttia huomattu
  {
    alarmset = !alarmset;                           //Muutetaan hälytystilaa
    alarm=false;                                    //Hälytys pois päältä
   
    RC522.readCardSerial();                         //Luetaan kortin UID numero
    Serial.println("Card detected:");               //Tulostetaan Serial monitorille
    for(int i=0;i<5;i++)
    {
    Serial.print(RC522.serNum[i],DEC);              //Tulostetaan serial monitorille kortin numero
    
    
    }
    Serial.println();
    Serial.println();
  }
  delay(1000);
  
}

void salasana(){
   char key = keypad.getKey();                    //Luetaan näppäimistöä
 lcd.setCursor(0, 1);                             //Asetetaan LCD-näytön osoittajan paikka
  if (key){
    lcd.print(key);                               //Tulostetaan LCD-näytölle

     if(key == '*') {                             //Jos näppäilty merkki on *
      input_password = "";                        //tyhjennetään syötetty salasana
    } else if(key == '#') {                       //Jos näppäilty merkki on # syötetään salasana
      if(password == input_password) {            //Jos syötetty salasana on oikein
        lcd.print("correct");                     //Tulostetaan LCD-näytölle
        alarm = false;                            //Asetetaan hälytystila pois päältä
        alarmset = false;                         //Asetetaan hälytys pois päältä

        
      } else {
        lcd.print("incorrect");                   //Tulostetaan LCD-näytölle
      }

      input_password = "";                        //Tyhjennetään syötetty salasana
    } else {
      input_password += key;                      //Lisätään salasanaan näppäilty numero
    }
}
}
