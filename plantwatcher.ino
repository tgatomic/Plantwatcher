/***************************************************************************
 Det här programmet håller koll på en växts fuktighet i jorden och öppnar
 en ventil för vatten att komma igenom om jorden blir för torr. 
 
 Om vattennivån blir för låg, kommer programmet att skicka ett SMS och varna
 om att det snart måste fyllas på. 
 ***************************************************************************/

#include<Servo.h>
#include<SoftwareSerial.h>
#include <Adafruit_FONA.h>


/***************************************************************************
 Här kommer inställningar för GPRS modulen så att den vet vilka in och
 utgångar som används på i kommunikationen mellan Arduino och GPRS modulen.
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
 Här kommer inställningarna för jordfuktsmötaren, vattennivåmätaren samt
 till motorn som ska öppna ventilen för vattnet. Initierar variabler för
 de olika nivåerna för att lättare kunna arbeta med dem, samt sätta max
 och minivärden då motorn inte har varit helt konsekvent (har skiftat max
 vinkel med ett par grader beroendes på belastning).
***************************************************************************/


Servo myServo; //Namnet på motorn
int const humPin = A0;
int const watPin = A1 ;

int watlvl; //Variabel för vattenmätaren
int humlvl; //Variabel för jordfuktsmätaren

int valvopen = 0; //gradantal på motor för öppen
int valvclosed = 164; //gradantal på motor för stängd

int sek = 1000;
int minut = 60000;

int counter = 0;

//LED på pin 3!!!



/***************************************************************************
 Här startar programmet med själva setup/installationen av alla moduler och 
 registrerar var allt är kopplat, om det ska ha ström och om det är in eller
 utgående (i vissa fall).

 Startar seriellmonitorn som gör att vi kan felsöka samt kommunicera mellan
 Arduino UNO och GPRS modulen. 
***************************************************************************/

void setup() 
{
  Serial.begin(115200); //seriel för monitorn
  fonaSS.begin(4800);   //Hastigheten som vi kommunicerar med GPRS modulen


  Serial.println(F("Väcker Gustav!"));

  //Startar om GPRS modulen och väntar på en OK signal tillbaka.
  if (! fona.begin(fonaSS)) 
    {            
      Serial.println(F("Kan inte starta GPRS"));
      while (1);
    }
  fona.unlockSIM(PIN);
  delay(2000);
  Serial.println(F("GPRS har startat!"));


  myServo.attach(5); //beskriver att motorn finns på port 5.
  pinMode(6,OUTPUT); // - LEDPIN
  digitalWrite(6, HIGH);
  delay(1000);
  digitalWrite(6, LOW);
  myServo.write(164);//Anger vilken vinkel motorn ska ha

  fona.sendSMS(admin, "Gustav har vaknat");

}

/***************************************************************************
 Här kommer den del som kommer köra hela tiden. Sålänge det finns ström så
 kommer följande kod att loopas igenom. Det är en blandning mellan data som
 skickas till den seriella motorn för att kunna dubbelkolla värden, samt
 saker som ska utföras vid specifika händelser.  
***************************************************************************/

void loop() {
    int humprocent = ((humlvl-185)*0.119);
    int watprocent = watlvl*0.129;
    int humlow; //Snustorrt = 1023 riktigt blött = 185
    //vatten från 0 till 775
    
    humlvl = analogRead(humPin); //Berättar att värdet från fuktmätaren sparas i humlvl.
    watlvl = analogRead(watPin); //Samma som ovan, fast till vattenmätaren itsället.
    Serial.print("Humiditylvl: ");
    Serial.print(humlvl);
    Serial.print(" Waterlvl: ");
    Serial.print(watlvl);
    Serial.print(" Water %: ");
    Serial.println(watprocent);
    

    
    while (humlvl > 100) //Basvärde var 76 vid exprimentstart
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
          fona.sendSMS(admin, "Hallå?.. Är du där?.. maten är snart slut :( ");
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
 Nedan kommer de olika funktionerna som jag använder mig utav för att inte
 loop ska bli svårläst.  
***************************************************************************/

void openvalve()
{
      myServo.write(valvopen);
      delay(1500);
      myServo.write(valvclosed);
      fona.sendSMS(admin, "It's raining men!(?)");
      delay((minut*15)); //Sätter delay på 15 minuter, så vattnet hinner sjunka ner i jorden.      
}

void lowwater()
{
    int counter;
    if (counter > 50)
    {
        fona.sendSMS(admin, "Maten är snart slut");
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
        fona.sendSMS(admin, "Hallå?.. Är du där?.. maten är snart slut :( ");
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



