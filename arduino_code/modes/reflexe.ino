using namespace std;
long randNumber;
#include "Adafruit_NeoPixel.h";
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <PulseSensorPlayground.h>     // Inclut la bibliothèque PulseSensorPlayground.  

// Variables
const int PulseWire = 0;       // Fil VIOLET du capteur de pouls connecté à la BROCHE ANALOGIQUE 0
const int LED = LED_BUILTIN;          // La LED Arduino intégrée, près de la BROCHE 13.
int Threshold = 550;           // Détermine le signal à "compter comme un battement" et celui à ignorer.
PulseSensorPlayground pulseSensor;  

// Define colors and leds
#define ROUGE 0
#define VERT 3
#define BLEU 2
#define JAUNE 1
#define PIXEL_COUNT 8
#define PIXEL_PIN 5// a definir

// --- Définition des notes ---
#define NOTE_C4  262
#define NOTE_E4  330
#define NOTE_G4  392
#define NOTE_C5  523
#define NOTE_A3  220
#define NOTE_E5  659
#define NOTE_G5  784
#define NOTE_D4  294
#define NOTE_F4  349
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_D5  587
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_G3  196
#define NOTE_B3  247

#define BUZZER_PIN 9

// Définition des broches des multiplexeurs
const int dataPin = 2;   // Q7 : Sortie des données du registre
const int clockPin= 3;  // CP : Signal d'horloge
const int latchPin= 4;  // PL : Signal de verrouillage
const int clockPinEnable = 8;

unsigned long pinValues;
unsigned long oldPinValues;

const int numBits = 16;   // 8 boutons, 1 registre
String data_screen = "";
String data_putty = "PUTTY,ESSAI,TEMPS,";
String data_bpm = "BPM,";
Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
LiquidCrystal_I2C ecran(0x27, 16, 2);
SoftwareSerial liaisonSerie(10, 11);

// Variables pour mémoriser l'état précédent
byte lastButtonState = 0;


int SEC = 0;
int MILLI = 0;
unsigned long timer;

int compteur = 0;
String data_temps = "";
int number_of_turn = 10;



void setup() { //pour initialiser les paramètres, les entrées et les sorties
   Serial.begin(9600);
   randomSeed(analogRead(A1));
   ecran.init();
   ecran.backlight();
   liaisonSerie.begin(9600);
   timer = millis();
   pinMode(PIXEL_PIN, INPUT_PULLUP);
   strip.begin(); // Initialize NeoPixel strip object (REQUIRED)
   pinMode(dataPin, INPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(latchPin, OUTPUT);
    pinMode(clockPinEnable, OUTPUT);
   strip.show();
   strip.setBrightness(5); // Ajuste la luminosité (0 à 255)
   displayColor(strip.Color(200, 200, 200));
   pinMode(BUZZER_PIN, OUTPUT);
   pulseSensor.analogInput(PulseWire);  
   pulseSensor.blinkOnPulse(LED);       // Fait clignoter automatiquement la LED d'Arduino au rythme cardiaque.
   pulseSensor.setThreshold(Threshold);  

}
//Fonctions pour le son
void playStartCountdown() {
   // "3"
   tone(BUZZER_PIN, NOTE_C4, 750);  // 1 seconde
   delay(1000);
   noTone(BUZZER_PIN);
   delay(300); // petite pause
   // "2"
   tone(BUZZER_PIN, NOTE_E4, 750);
   delay(1000);
   noTone(BUZZER_PIN);
   delay(300);
   // "1"
   tone(BUZZER_PIN, NOTE_G4, 750);
   delay(1000);
   noTone(BUZZER_PIN);
   delay(300);
   // "Partez !"
   tone(BUZZER_PIN, NOTE_C5, 1000);  // un peu plus long
   delay(1500);
   noTone(BUZZER_PIN);
}
void playStopSound() {
   // Son STOP : descente de trois notes
   tone(BUZZER_PIN, NOTE_C5, 300);  // note aiguë
   delay(300);
   tone(BUZZER_PIN, NOTE_G4, 300);  // plus grave
   delay(300);
   tone(BUZZER_PIN, NOTE_E4, 500);  // encore plus grave
   delay(500);
   noTone(BUZZER_PIN);              // Arrête le son
}
void playSuccessSound() {
   // Petit jingle : C5 → E5 → G5
   tone(BUZZER_PIN, NOTE_C5, 150);
   delay(170);
   tone(BUZZER_PIN, NOTE_E5, 150);
   delay(170);
   tone(BUZZER_PIN, NOTE_G5, 400);
   delay(400);
   noTone(BUZZER_PIN);
}
void playStartSound() {
   tone(BUZZER_PIN, NOTE_C4, 500); // "3"
   delay(500);
   noTone(BUZZER_PIN);
   delay(300);
   tone(BUZZER_PIN, NOTE_E4, 750); // "2"
   delay(500);
   noTone(BUZZER_PIN);
   delay(300);
   tone(BUZZER_PIN, NOTE_G4, 750); // "1"
   delay(1000);
   noTone(BUZZER_PIN);
   delay(300);
   tone(BUZZER_PIN, NOTE_C5, 750); // "Partez !"
   delay(1500);
   noTone(BUZZER_PIN);
}
void playDefeatSound() {
   tone(BUZZER_PIN, NOTE_B4, 500);
   delay(500);
   noTone(BUZZER_PIN);
}
void playClickSound() {
   tone(BUZZER_PIN, NOTE_B4, 80);
   delay(100);
   noTone(BUZZER_PIN);
}
//Fonctions nécessaires pour l'étude du registre
unsigned long read_shift_register()
{
    long bitVal;
    unsigned long octetRead = 0;

    digitalWrite(clockPinEnable, HIGH);
    digitalWrite(latchPin, LOW);
    delayMicroseconds(5);
    digitalWrite(latchPin, HIGH);
    digitalWrite(clockPinEnable, LOW);

    for(int i = 0; i < numBits; i++)
    {
        bitVal = digitalRead(dataPin);
        octetRead |= (bitVal << ((numBits-1) - i));
        digitalWrite(clockPin, HIGH);
        delayMicroseconds(5);
        digitalWrite(clockPin, LOW);
    }

    return(octetRead);
}
int value(int number){
  switch(number){
    case 1 : return 0;
    case 2 : return 1;
    case 4 : return 2;
    case 8 : return 3;
    case 16 : return 4;
    case 32 : return 5;
    case 64 : return 6;
    case 128 : return 7;
    case 256 : return 8;
    case 512 : return 9;
    case 1024 : return 10;
    case 2048 : return 11;
    case 4096 : return 12;
    case 8192 : return 13;
    case 16384 : return 14;
    case 32768 : return 15;
    default : return -1;
  }
}
// Fonctions pour le choix de la couleur et son affichage
int chooseColor() {
   return random(0, 4);
}
uint32_t codeColor(int zone){
   switch (zone) {
       case 0: return strip.Color(255, 0, 0);    // Rouge vif
       case 1: return strip.Color(125, 0, 125);    // Jaune vif
       case 2: return strip.Color(0, 255, 0);    // Bleu vif
       case 3: return strip.Color(0, 0, 255);  // Vert vif
       default: return strip.Color(0, 0, 0);     // Noir (éteint) par sécurité
   }
}
void displayColor(uint32_t color){
   for (int i = 0; i < PIXEL_COUNT; i++) {
        strip.setPixelColor(i, color);
    }
    strip.show();
}
void displayEndColor(){
   int j = 0;
   for (int i = 0; i < PIXEL_COUNT; i++) {
      strip.setPixelColor(i, strip.Color(200, 200, 200));
  }
  strip.show();
}
//affichage écran et serial
void displayScreen(int zone, int compteur){ // affichage score
   ecran.clear();
   ecran.setCursor(0, 0);
   ecran.print("COULEUR :");
   switch (zone) {
       case ROUGE: ecran.print("ROUGE"); break;
       case VERT: ecran.print("VERT"); break;
       case BLEU: ecran.print("BLEU"); break;
       case JAUNE: ecran.print("JAUNE"); break;
   }
   ecran.setCursor(0, 1);
   ecran.print("Essai n : "+String(compteur));
}
void displaySerial(int zone, int compteur){ // affichage score
   //Serial.println("\nEssai n°: "+String(compteur));
   switch (zone) {
       case ROUGE: Serial.println("ROUGE"); break;
       case VERT: Serial.println("VERT"); break;
       case BLEU: Serial.println("BLEU"); break;
       case JAUNE: Serial.println("JAUNE"); break;

   }
}

int whichAnswer(){
   int answer = -1;
   pinValues = read_shift_register();

    if(pinValues != oldPinValues)
    {   if(value(pinValues)!=-1){
        answer = value(pinValues);
        delay(200);}
        oldPinValues = pinValues;
    }// Petite pause pour éviter les rebonds
    if(answer != -1){playClickSound();}

   return answer;
}
//fonction pour l'attente de réponse
boolean correctAnswer(int zone){ //permet de vérifier que le bouton appuyé  correspond à la zone demandée
   int answer = whichAnswer();
   //Serial.println("WHICH ANSWER" + answer);
   if(answer == 0 | answer ==  1 | answer == 2 | answer == 3){answer = ROUGE;}
   if(answer == 8 | answer ==  9 | answer == 15 | answer == 14){answer = VERT;}
   if(answer == 11 | answer ==  10 | answer == 12 | answer == 13){answer = BLEU;}
   if(answer == 4 | answer ==  5 | answer == 7 | answer == 6){answer = JAUNE;}
   if(answer == -1){return false;}
   switch (zone) {
      case 0: if(answer == zone){return true;}
      case 1:  if(answer == zone){return true;}
      case 2:  if(answer == zone){return true;}
      case 3:  if(answer == zone){return true;}
      default: return false;
  }
  return false;
}
//fonctions pour le chrono et le stockage du temps
String timeNeeded(int time_before){ //mesure le temps pris avant d’appuyer sur un bouton
   int time_now = millis();
   int diff = time_now - time_before;
   SEC = diff/1000;
   MILLI = diff - SEC*1000;
   return String(SEC) + ":" + String(MILLI);
}
void displayTime(String time) { //affiche si la réponse est bonne ou pas
   ecran.clear();
   ecran.setCursor(0, 0);
   ecran.print("temps :");
   ecran.print(time);
   //Serial.println("temps n°"+String(compteur)+" = "+time+"\n");
   delay(1000);
}
void newData(String time){ //enregistre les nouvelles données dans un tableau
   data_screen = data_screen + String(compteur)+" : "+time + "\n";
   data_putty = data_putty + String(compteur) + ","+time+",";
}

void loop(){ //boucle du jeu principal, où rassembler les fonctions afin d’obtenir le mode 1 
   if(compteur==0){playStartSound();}
   while (compteur < number_of_turn) {
    if (pulseSensor.sawStartOfBeat()) {
      int myBPM = pulseSensor.getBeatsPerMinute();
      data_bpm = data_bpm + String(myBPM) + ",";
      }
      compteur+=1;
      boolean pressed = false; 
      int zone = chooseColor();
      uint32_t couleur = codeColor(zone);
      displayColor(couleur);
      displayScreen(zone, compteur);
      //displaySerial(zone, compteur);
      int time_before = millis();
      delay(100);
      while(pressed == false){
         if (correctAnswer(zone)==true){pressed = true;}
      }
      String time = timeNeeded(time_before);
      SEC = 0;
      MILLI = 0;
      displayTime(time);
      newData(time);    
      ecran.clear();
      ecran.print("Temps mis = ");
      ecran.print(String(SEC) + " : " + String(MILLI));
   }
   //if(compteur==number_of_turn){playStopSound();ecran.clear();ecran.print("Fin du jeu");Serial.println("\n****\n"+data_screen+"****");compteur+=1;}
   if(compteur==number_of_turn){playStopSound();ecran.clear();ecran.print("Fin du jeu");Serial.println(data_putty);Serial.print(data_bpm);compteur+=1;}

}
