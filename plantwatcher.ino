/***************************************************************************
 Det h�r programmet h�ller koll p� en v�xts fuktighet i jorden och �ppnar
 en ventil f�r vatten att komma igenom om jorden blir f�r torr. 
 
 Om vattenniv�n blir f�r l�g, kommer programmet att skicka ett SMS och varna
 om att det snart m�ste fyllas p�. 
 ***************************************************************************/

#include<Servo.h>
#include<SoftwareSerial.h>
#include <Adafruit_FONA.h>


/***************************************************************************
 H�r kommer inst�llningar f�r GPRS modulen s� att den vet vilka in och
 utg�ngar som anv�nds p� i kommunikationen mellan Arduino och GPRS modulen.
***************************************************************************/
 
#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

char replybuffer[255]; //Buffert till SMS svar
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX); 
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);
char admin[] = "0734170866";
char PIN[] = "1160";

/***************************************************************************
 H�r kommer inst�llningarna f�r jordfuktsm�taren, vattenniv�m�taren samt
 till motorn som ska �ppna ventilen f�r vattnet. Initierar variabler f�r
 de olika niv�erna f�r att l�ttare kunna arbeta med dem, samt s�tta max
 och miniv�rden d� motorn inte har varit helt konsekvent (har skiftat max
 vinkel med ett par grader beroendes p� belastning).
***************************************************************************/


Servo myServo; //Namnet p� motorn
int const humPin = A0;
int const watPin = A1 ;

int watlvl; //Variabel f�r vattenm�taren
int humlvl; //Variabel f�r jordfuktsm�taren

int valvopen = 0; //gradantal p� motor f�r �ppen
int valvclosed = 164; //gradantal p� motor f�r st�ngd

int sek = 1000;
int minut = 60000;

int counter = 0;

//LED p� pin 3!!!



/***************************************************************************
 H�r startar programmet med sj�lva setup/installationen av alla moduler och 
 registrerar var allt �r kopplat, om det ska ha str�m och om det �r in eller
 utg�ende (i vissa fall).

 Startar seriellmonitorn som g�r att vi kan fels�ka samt kommunicera mellan
 Arduino UNO och GPRS modulen. 
***************************************************************************/

void setup() 
{
  Serial.begin(115200); //seriel f�r monitorn
  fonaSS.begin(4800);   //Hastigheten som vi kommunicerar med GPRS modulen


  Serial.println(F("V�cker Gustav!"));

  //Startar om GPRS modulen och v�ntar p� en OK signal tillbaka.
  if (! fona.begin(fonaSS)) 
    {            
      Serial.println(F("Kan inte starta GPRS"));
      while (1);
    }
  fona.unlockSIM(PIN);
  delay(2000);
  Serial.println(F("GPRS har startat!"));


  myServo.attach(5); //beskriver att motorn finns p� port 5.
  pinMode(6,OUTPUT); // - LEDPIN
  digitalWrite(6, HIGH);
  delay(1000);
  digitalWrite(6, LOW);
  myServo.write(164);//Anger vilken vinkel motorn ska ha

  fona.sendSMS(admin, "Gustav har vaknat");

}

/***************************************************************************
 H�r kommer den del som kommer k�ra hela tiden. S�l�nge det finns str�m s�
 kommer f�ljande kod att loopas igenom. Det �r en blandning mellan data som
 skickas till den seriella motorn f�r att kunna dubbelkolla v�rden, samt
 saker som ska utf�ras vid specifika h�ndelser.  
***************************************************************************/

void loop() {
    int humprocent = ((humlvl-185)*0.119);
    int watprocent = watlvl*0.129;
    int humlow; //Snustorrt = 1023 riktigt bl�tt = 185
    //vatten fr�n 0 till 775
    
    humlvl = analogRead(humPin); //Ber�ttar att v�rdet fr�n fuktm�taren sparas i humlvl.
    watlvl = analogRead(watPin); //Samma som ovan, fast till vattenm�taren its�llet.
    Serial.print("Humiditylvl: ");
    Serial.print(humlvl);
    Serial.print(" Waterlvl: ");
    Serial.print(watlvl);
    Serial.print(" Water %: ");
    Serial.println(watprocent);
    

    
    while (humlvl > 100) //Basv�rde var 76 vid exprimentstart
    {
      openvalve();
    }
    if (watlvl<500 && watlvl > 50)
    {
      lowwater();
    }

    
    
   while (watlvl<50)
    {
      //nowater();
         
      if (counter > 29)
      {
          fona.sendSMS(admin, "Hall�?.. �r du d�r?.. maten �r snart slut :( ");
          counter = 0;
      }
      if (counter > 29 && humlvl > 100)
      {
         fona.sendSMS(admin, "I'm drying.. dryyyiiiiing... :'( "); 
         counter = 0;     
      }
       else
    
      for(int i = 0; i < 10; i++)
      {
          digitalWrite(6, HIGH);
          delay(500);
          counter++;
          Serial.print("Repetition: ");
          Serial.println(counter);
          digitalWrite(6, LOW);
          delay(500);
      }
      delay(5000);
    }
 

    delay(2000);


}


/***************************************************************************
 Nedan kommer de olika funktionerna som jag anv�nder mig utav f�r att inte
 loop ska bli sv�rl�st.  
***************************************************************************/

void openvalve()
{
      myServo.write(valvopen);
      delay(1500);
      myServo.write(valvclosed);
      fona.sendSMS(admin, "It's raining men!(?)");
      delay((minut*15)); //S�tter delay p� 15 minuter, s� vattnet hinner sjunka ner i jorden.      
}

void lowwater()
{
    int counter;
    if (counter > 50)
    {
        fona.sendSMS(admin, "Maten �r snart slut");
    }

    for(int i; i < 5; i++)
        digitalWrite(6, HIGH);
        delay(1000);
        digitalWrite(6, LOW);
        counter++;
    delay(5000);
}

/*
void nowater()
{
    int counter;
    if (counter > 50)
    {
        fona.sendSMS(admin, "Hall�?.. �r du d�r?.. maten �r snart slut :( ");
    }
    if (counter > 50 && humlvl > 100)
    {
        fona.sendSMS(admin, "I'm drying.. dryyyiiiiing... :'( ");      
    }
    else
    
    for(int i; i < 10; i++)
        digitalWrite(6, HIGH);
        delay(1000);
        digitalWrite(6, LOW);
        counter++;
    delay(5000);
}

*/



