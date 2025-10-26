#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_NeoPixel.h>

// Configuration des broches et constantes
#define PIXEL_PIN 5
#define PIXEL_COUNT 8
#define BUZZER_PIN 9
#define ROUGE 0
#define VERT 3
#define BLEU 2
#define JAUNE 1




// Variables globales
enum GameMode { IDLE, REFLEXE, MEMOIRE, PRECISION };
GameMode currentMode = IDLE;
GameMode requestedMode = IDLE;
// Ajoutez ces variables globales
float tempsEssais[10]; // Pour stocker les temps de chaque essaie




LiquidCrystal_I2C ecran(0x27, 16, 2);
Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);




// Broches des multiplexeurs
const int dataPin = 2;
const int clockPin = 3;
const int latchPin = 4;
const int clockPinEnable = 8;




unsigned long pinValues;
unsigned long oldPinValues;
const int numBits = 16;




// Variables pour les modes
// Mode réflexe
int compteurReflexe = 0;
String data_putty_reflexe = "";
const int number_of_turn_reflexe = 10;
int etatReflexe = 0;
int zoneReflexe = -1;
unsigned long tempsDebutReflexe = 0;




// Mode mémoire
int compteurExterne = 0;
String data_putty_memoire = "";
const int number_of_turn_memoire = 3;
int sequence[10];
int tailleSequence = 2;
int etatMemoire = 0;
int sequencePosMemoire = 0;
unsigned long tempsDebutMemoire = 0;




// Mode précision
int compteurPrecision = 0;
String data_putty_precision = "";
const int number_of_turn_precision = 10;
int positions[4][2] = {{5, 0}, {8, 0}, {5, 1}, {8, 1}};
int etatPrecision = 0;
int boutonPrecision = -1;
int boutonIndexPrecision = -1;
unsigned long tempsDebutPrecision = 0;




// Caractères personnalisés
byte boutonVide[] = {0b00000, 0b00100, 0b01010, 0b01010, 0b01010, 0b00100, 0b00000, 0b00000};
byte boutonPlein[] = {0b00000, 0b00100, 0b01110, 0b01110, 0b01110, 0b00100, 0b00000, 0b00000};




void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(A1));
 
  ecran.init();
  ecran.backlight();
  ecran.createChar(0, boutonVide);
  ecran.createChar(1, boutonPlein);
 
  strip.begin();
  strip.show();
  strip.setBrightness(5);
 
  pinMode(dataPin, INPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPinEnable, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
 
  ecran.clear();
  ecran.print("Pret a recevoir");
  ecran.setCursor(0, 1);
  ecran.print("des commandes");
}




unsigned long read_shift_register() {
  long bitVal;
  unsigned long octetRead = 0;




  digitalWrite(clockPinEnable, HIGH);
  digitalWrite(latchPin, LOW);
  delayMicroseconds(5);
  digitalWrite(latchPin, HIGH);
  digitalWrite(clockPinEnable, LOW);




  for(int i = 0; i < numBits; i++) {
    bitVal = digitalRead(dataPin);
    octetRead |= (bitVal << ((numBits-1) - i));
    digitalWrite(clockPin, HIGH);
    delayMicroseconds(5);
    digitalWrite(clockPin, LOW);
  }




  return octetRead;
}




int value(int number) {
  switch(number) {
    case 1: return 0; case 2: return 1; case 4: return 2; case 8: return 3;
    case 16: return 4; case 32: return 5; case 64: return 6; case 128: return 7;
    case 256: return 8; case 512: return 9; case 1024: return 10; case 2048: return 11;
    case 4096: return 12; case 8192: return 13; case 16384: return 14; case 32768: return 15;
    default: return -1;
  }
}




uint32_t codeColor(int zone) {
  switch (zone) {
    case 0: return strip.Color(255, 0, 0);    // Rouge
    case 1: return strip.Color(125, 0, 125);  // Jaune
    case 2: return strip.Color(0, 255, 0);    // Bleu
    case 3: return strip.Color(0, 0, 255);    // Vert
    default: return strip.Color(128, 128, 0);     // Noir
  }
}




void displayColor(uint32_t color) {
  for (int i = 0; i < PIXEL_COUNT; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}




void displayEndColor() {
  for (int i = 0; i < PIXEL_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(200, 200, 200));
  }
  strip.show();
}




void playStartSound() {
  tone(BUZZER_PIN, 262, 500); delay(500); noTone(BUZZER_PIN); delay(300);
  tone(BUZZER_PIN, 330, 750); delay(500); noTone(BUZZER_PIN); delay(300);
  tone(BUZZER_PIN, 392, 750); delay(1000); noTone(BUZZER_PIN); delay(300);
  tone(BUZZER_PIN, 523, 750); delay(1500); noTone(BUZZER_PIN);
}




void playStopSound() {
  tone(BUZZER_PIN, 523, 300); delay(300);
  tone(BUZZER_PIN, 392, 300); delay(300);
  tone(BUZZER_PIN, 330, 500); delay(500); noTone(BUZZER_PIN);
}




void playSuccessSound() {
  tone(BUZZER_PIN, 523, 150); delay(170);
  tone(BUZZER_PIN, 659, 150); delay(170);
  tone(BUZZER_PIN, 784, 400); delay(400); noTone(BUZZER_PIN);
}




void playClickSound() {
  tone(BUZZER_PIN, 494, 80); delay(100); noTone(BUZZER_PIN);
}




String timeNeeded(unsigned long time_before) {
  unsigned long diff = millis() - time_before;
  int SEC = diff/1000;
  int MILLI = diff % 1000;
  return String(SEC) + ":" + String(MILLI);
}




int whichAnswer() {
  int answer = -1;
  pinValues = read_shift_register();




  if(pinValues != oldPinValues && value(pinValues) != -1) {
    answer = value(pinValues);
    delay(200);
    oldPinValues = pinValues;
    playClickSound();
  }
  return answer;
}




void cleanupReflexeMode() {
  displayEndColor();
  ecran.clear();
  data_putty_reflexe = "";
  compteurReflexe = 0;
  etatReflexe = 0;
  zoneReflexe = -1;
  tempsDebutReflexe = 0;
}




void cleanupMemoireMode() {
  displayEndColor();
  ecran.clear();
  data_putty_memoire = "";
  compteurExterne = 0;
  tailleSequence = 2;
  etatMemoire = 0;
  sequencePosMemoire = 0;
  tempsDebutMemoire = 0;
}




void cleanupPrecisionMode() {
  displayEndColor();
  ecran.clear();
  data_putty_precision = "";
  compteurPrecision = 0;
  etatPrecision = 0;
  boutonPrecision = -1;
  boutonIndexPrecision = -1;
  tempsDebutPrecision = 0;
}




void displayScreenReflexe(int zone, int compteur) {
  ecran.clear();
  ecran.setCursor(0, 0);
  ecran.print("COULEUR:");
  switch (zone) {
    case ROUGE: ecran.print("ROUGE"); break;
    case VERT: ecran.print("VERT"); break;
    case BLEU: ecran.print("BLEU"); break;
    case JAUNE: ecran.print("JAUNE"); break;
  }
  ecran.setCursor(0, 1);
  ecran.print("Essai:" + String(compteur));
}




bool correctAnswerReflexe(int zone) {
  int answer = whichAnswer();
  if(answer == -1) return false;
 
  if(answer == 7 || answer == 6 || answer == 2 || answer == 3) answer = ROUGE;
  if(answer == 8 || answer == 9 || answer == 15 || answer == 14) answer = VERT;
  if(answer == 11 || answer == 10 || answer == 12 || answer == 13) answer = BLEU;
  if(answer == 4 || answer == 5 || answer == 0 || answer == 1) answer = JAUNE;
 
  return (answer == zone);
}




void processReflexeMode() {
  switch(etatReflexe) {
    case 0: // Initialisation
      if (currentMode != REFLEXE) return;
      compteurReflexe = 0;
      data_putty_reflexe = "PUTTY,ESSAI,TEMPS,";
      playStartSound();
      etatReflexe = 1;
      break;
     
    case 1: // Nouvel essai
      if (currentMode != REFLEXE) {
        cleanupReflexeMode();
        return;
      }
     
      if(compteurReflexe < number_of_turn_reflexe) {
        zoneReflexe = random(0, 4);
        displayColor(codeColor(zoneReflexe));
        displayScreenReflexe(zoneReflexe, compteurReflexe+1);
        tempsDebutReflexe = millis();
        etatReflexe = 2;
      } else {
        etatReflexe = 3; // Fin du jeu
      }
      break;
     
    case 2: // Attente réponse
      if (currentMode != REFLEXE) {
        cleanupReflexeMode();
        return;
      }
     
      if(correctAnswerReflexe(zoneReflexe)) {
  String temps = timeNeeded(tempsDebutReflexe);
  tempsEssais[compteurReflexe] = (millis() - tempsDebutReflexe) / 1000.0; // Stocke en secondes
  data_putty_reflexe += String(compteurReflexe+1) + "," + temps + ",";
  compteurReflexe++;
  etatReflexe = 1;
}
      break;
     
    case 3: // Fin du mode
  if (currentMode != REFLEXE) {
    cleanupReflexeMode();
    return;
  }


  playStopSound();
  ecran.clear();
  ecran.print("Fin Mode Reflexe");


  // Format correct pour le parsing
  Serial.print("REFLEXE_RESULTS:");
  Serial.print("score="); Serial.print(compteurReflexe);
  Serial.print(",temps_moyen="); Serial.print(calculateAverageTime());
  Serial.print(",details=");
  for(int i = 0; i < compteurReflexe; i++) {
    Serial.print("essai"); Serial.print(i+1); Serial.print("=");
    Serial.print(tempsEssais[i]);
    if(i < compteurReflexe-1) Serial.print(","); // Utiliser des virgules au lieu de ;
  }
  Serial.println();


  currentMode = IDLE;
  etatReflexe = 0;
  break;
  }
}




void generateSequence() {
  for(int i = 0; i < tailleSequence; i++) {
    sequence[i] = random(0, 4);
  }
}




void displaySequence() {
  ecran.clear();
  ecran.print("Memorisez!");
  displayColor(strip.Color(25, 25, 25));
  delay(750);
 
  for(int i = 0; i < tailleSequence; i++) {
    if (currentMode != MEMOIRE) return;
   
    ecran.clear();
    displayEndColor();
    ecran.setCursor(0, 0);
    ecran.print("n:" + String(i+1));
   
    switch(sequence[i]) {
      case 0: ecran.setCursor(0,1); ecran.print("ROUGE"); displayColor(codeColor(ROUGE)); break;
      case 1: ecran.setCursor(0,1); ecran.print("VERT"); displayColor(codeColor(VERT)); break;
      case 2: ecran.setCursor(0,1); ecran.print("BLEU"); displayColor(codeColor(BLEU)); break;
      case 3: ecran.setCursor(0,1); ecran.print("JAUNE"); displayColor(codeColor(JAUNE)); break;
    }
    delay(1000);
  }
}




int correctAnswerMemoire(int zone, int answer) {
  if(answer == -1) return 3;
 
  if(answer == 6 || answer == 7 || answer == 2 || answer == 3) answer = ROUGE;
  if(answer == 8 || answer == 9 || answer == 15 || answer == 14) answer = VERT;
  if(answer == 11 || answer == 10 || answer == 12 || answer == 13) answer = BLEU;
  if(answer == 4 || answer == 5 || answer == 0 || answer == 1) answer = JAUNE;
 
  return (answer == zone) ? 1 : 2;
}




void processMemoireMode() {
  switch(etatMemoire) {
    case 0: // Initialisation
      if (currentMode != MEMOIRE) return;
      compteurExterne = 0;
      tailleSequence = 2;
      data_putty_memoire = "PUTTY,SEQUENCE,ESSAI,TEMPS,REUSSITE,";
      playStartSound();
      etatMemoire = 1;
      break;
     
    case 1: // Nouvelle séquence
      if (currentMode != MEMOIRE) {
        cleanupMemoireMode();
        return;
      }
     
      if(compteurExterne < number_of_turn_memoire) {
        generateSequence();
        displaySequence();
        sequencePosMemoire = 0;
        data_putty_memoire += String(compteurExterne+1) + ",";
        etatMemoire = 2;
      } else {
        etatMemoire = 5; // Fin du jeu
      }
      break;
     
    case 2: // Attente réponse
      if (currentMode != MEMOIRE) {
        cleanupMemoireMode();
        return;
      }
     
      if(sequencePosMemoire < tailleSequence) {
        ecran.clear();
        ecran.print("Votre tour!");
        tempsDebutMemoire = millis();
        etatMemoire = 3;
      } else {
        tailleSequence += 2;
        compteurExterne++;
        etatMemoire = 4;
      }
      break;
     
    case 3: // Vérification réponse
      if (currentMode != MEMOIRE) {
        cleanupMemoireMode();
        return;
      }
     
      {
        int answer = whichAnswer();
        if(answer != -1) {
          int result = correctAnswerMemoire(sequence[sequencePosMemoire], answer);
          String temps = timeNeeded(tempsDebutMemoire);
          data_putty_memoire += temps + "," + (result == 1 ? "REUSSITE," : "ECHEC,");
         
          if(result == 1) playSuccessSound();
          sequencePosMemoire++;
          etatMemoire = 2;
        }
      }
      break;
     
    case 4: // Entre séquences
      if (currentMode != MEMOIRE) {
        cleanupMemoireMode();
        return;
      }
     
      ecran.clear();
      ecran.print("Sequence finie");
      delay(1500);
      etatMemoire = 1;
      break;
     
    case 5: // Fin du mode
      if (currentMode != MEMOIRE) {
        cleanupMemoireMode();
        return;
      }
     
      playStopSound();
      ecran.clear();
      ecran.print("Fin Mode Memoire");
      Serial.println("MEMOIRE_DATA:" + data_putty_memoire);
      currentMode = IDLE;
      etatMemoire = 0;
      break;
  }
}




void displayPrecisionScreen(int boutonIndex) {
  ecran.clear();
  if(boutonIndex == 
  /*for(int i = 0; i < 4; i++) {
    ecran.setCursor(positions[i][0], positions[i][1]);
    ecran.write(i == boutonIndex ? byte(1) : byte(0));*/
  }
}




int zoneOfButton(int bouton) {
  if(bouton == 2 || bouton == 3 || bouton == 6 || bouton == 7) return ROUGE;
  if(bouton == 8 || bouton == 9 || bouton == 15 || bouton == 14) return VERT;
  if(bouton == 11 || bouton == 13 || bouton == 12 || bouton == 10) return BLEU;
  if(bouton == 0 || bouton == 1 || bouton == 4 || bouton == 5) return JAUNE;
  return -1;
}




bool correctAnswerPrecision(int bouton) {
  int answer = whichAnswer();
  return (answer == bouton);
}




void processPrecisionMode() {
  switch(etatPrecision) {
    case 0: // Initialisation
      if (currentMode != PRECISION) return;
      compteurPrecision = 0;
      data_putty_precision = "PUTTY,ESSAI,TEMPS,";
      playStartSound();
      etatPrecision = 1;
      break;
     
    case 1: // Nouvel essai
      if (currentMode != PRECISION) {
        cleanupPrecisionMode();
        return;
      }
     
      if(compteurPrecision < number_of_turn_precision) {
        boutonPrecision = random(0, 16);
        boutonIndexPrecision = boutonPrecision % 4;
        int zone = zoneOfButton(boutonPrecision);
        displayColor(codeColor(zone));
        displayPrecisionScreen(boutonIndexPrecision);
        tempsDebutPrecision = millis();
        etatPrecision = 2;
      } else {
        etatPrecision = 3; // Fin du jeu
      }
      break;
     
    case 2: // Attente réponse
      if (currentMode != PRECISION) {
        cleanupPrecisionMode();
        return;
      }
     
      if(correctAnswerPrecision(boutonPrecision)) {
        String temps = timeNeeded(tempsDebutPrecision);
        data_putty_precision += String(compteurPrecision+1) + "," + temps + ",";
        compteurPrecision++;
        etatPrecision = 1;
      }
      break;
     
    case 3: // Fin du mode
      if (currentMode != PRECISION) {
        cleanupPrecisionMode();
        return;
      }
     
      playStopSound();
      ecran.clear();
      ecran.print("Fin Mode Precision");
      Serial.println("PRECISION_DATA:" + data_putty_precision);
      currentMode = IDLE;
      etatPrecision = 0;
      break;
  }
}




float calculateAverageTime() {
  float somme = 0.0;
  int nombre = 0;




  for (int i = 0; i < number_of_turn_reflexe; i++) {
    if (tempsEssais[i] > 0) {
      somme += tempsEssais[i];
      nombre++;
    }
  }




  if (nombre == 0) return 0.0;
  return somme / nombre;
}








void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
   
    if (command == "start_reflexe") {
      if (currentMode != IDLE && currentMode != REFLEXE) {
        switch(currentMode) {
          case MEMOIRE: cleanupMemoireMode(); break;
          case PRECISION: cleanupPrecisionMode(); break;
          default: break;
        }
      }
      requestedMode = REFLEXE;
      currentMode = IDLE;
      Serial.println("Mode Reflexe demande");
    }
    else if (command == "start_memoire") {
      if (currentMode != IDLE && currentMode != MEMOIRE) {
        switch(currentMode) {
          case REFLEXE: cleanupReflexeMode(); break;
          case PRECISION: cleanupPrecisionMode(); break;
          default: break;
        }
      }
      requestedMode = MEMOIRE;
      currentMode = IDLE;
      Serial.println("Mode Memoire demande");
    }
    else if (command == "start_precision") {
      if (currentMode != IDLE && currentMode != PRECISION) {
        switch(currentMode) {
          case REFLEXE: cleanupReflexeMode(); break;
          case MEMOIRE: cleanupMemoireMode(); break;
          default: break;
        }
      }
      requestedMode = PRECISION;
      currentMode = IDLE;
      Serial.println("Mode Precision demande");
    }
    else if (command == "stop") {
      switch(currentMode) {
        case REFLEXE: cleanupReflexeMode(); break;
        case MEMOIRE: cleanupMemoireMode(); break;
        case PRECISION: cleanupPrecisionMode(); break;
        default: break;
      }
      requestedMode = IDLE;
      currentMode = IDLE;
      ecran.clear();
      ecran.print("Mode arrete");
      delay(1000);
    }
  }




  // Transition vers un nouveau mode si demandé
  if (requestedMode != IDLE && currentMode == IDLE) {
    currentMode = requestedMode;
    requestedMode = IDLE;
    ecran.clear();
    switch(currentMode) {
      case REFLEXE: ecran.print("Demarrage Reflexe"); break;
      case MEMOIRE: ecran.print("Demarrage Memoire"); break;
      case PRECISION: ecran.print("Demarrage Precision"); break;
      default: break;
    }
    delay(1000);
  }




  // Exécution du mode courant
  switch(currentMode) {
    case REFLEXE:
      processReflexeMode();
      break;
    case MEMOIRE:
      processMemoireMode();
      break;
    case PRECISION:
      processPrecisionMode();
      break;
    default:
      // Mode idle
      break;
  }




  delay(100);
}


