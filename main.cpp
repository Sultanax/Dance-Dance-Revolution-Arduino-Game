int pos = 0;
int speaker = 9;
int length = 52; 
//Array for the beats
int beats[] = {6, 3, 3, 8, 1, 2, 1, 2, 1, 2, 1, 3, 3, 6, 6, 1,
               6, 3, 3, 2, 1, 2, 1, 6, 1,
               3, 3, 3, 3, 2, 1, 2, 1, 2, 1, 2, 1,
              2, 1, 6, 3, 2, 1, 9, 1,
               6, 3, 3, 3, 3, 3, 3, 6
              };
//Notes for the song, a space represents a rest
char notes[] = "bfgddrefgabEDhC EDaCbbab CgfgbagaDCafbbbfbbb bCRDDDC";
//Tempo
int tempo = 100;
static unsigned long lastMusicTime = 0;
static unsigned int musicIndex = 0;

#include <LiquidCrystal.h>
#define PIN_BUTTON 2
#define UP_BUTTON 13
#define DOWN_BUTTON 10
#define LEFT_BUTTON 8
#define RIGHT_BUTTON 7
#define PIN_AUTOPLAY 1
#define PIN_READWRITE 10
#define PIN_CONTRAST 12


#define SPRITE_RUN1 1
#define SPRITE_RUN2 2
#define SPRITE_JUMP_UPPER '.'         // Use the '.' character for the head
#define SPRITE_TERRAIN_EMPTY ' ' 
#define SPRITE_UP 3
#define SPRITE_DOWN 4
#define SPRITE_LEFT 5
#define SPRITE_RIGHT 6

#define HERO_HORIZONTAL_POSITION 1    // Horizontal position of hero on screen

#define TERRAIN_WIDTH 16
#define TERRAIN_EMPTY 0
#define TERRAIN_LOWER_BLOCK 1

#define HERO_POSITION_OFF 0          // Hero is invisible
#define HERO_POSITION_RUN_LOWER_1 1  // Hero is running on lower row (pose 1)
#define HERO_POSITION_RUN_LOWER_2 2  //                              (pose 2)

LiquidCrystal lcd(12,11,5,4,3,6);
static char terrainLower[TERRAIN_WIDTH + 1];
static bool buttonPushed = false;
static bool upButtonPushed = false;

void initializeGraphics(){
  static byte graphics[] = {
    // Run position 1
    B01100,
    B01100,
    B00000,
    B01110,
    B11100,
    B01100,
    B11010,
    B10011,
    // Run position 2
    B01100,
    B01100,
    B00000,
    B01100,
    B01100,
    B01100,
    B01100,
    B01110,
    // Up
    B00100,
    B01110,
    B10101,
    B00100,
    B00100,
    B00100,
    B00000,
    B00000,
    // Down
    B00000,
    B00100,
    B00100,
    B10101,
    B01110,
    B00100,
    B00000,
    B00000,
    // Left
    B00010,
    B00110,
    B01110,
    B11111,
    B01110,
    B00110,
    B00010,
    B00000,
    // Right
    B01000,
    B01100,
    B01110,
    B11111,
    B01110,
    B01100,
    B01000,
    B00000
  };
  int i;
  // Skip using character 0, this allows lcd.print() to be used to
  // quickly draw multiple characters
  for (i = 0; i < 8; ++i) {
    lcd.createChar(i + 1, &graphics[i * 8]);
  }
  for (i = 0; i < TERRAIN_WIDTH; ++i) {
    terrainLower[i] = SPRITE_TERRAIN_EMPTY;
  }
}

// Slide the terrain to the left in half-character increments
void advanceTerrain(char* terrain, byte newTerrain) {
  for (int i = 0; i < TERRAIN_WIDTH; ++i) {
    terrain[i] = (i == TERRAIN_WIDTH - 1) ? newTerrain : terrain[i + 1];
  }
}

bool drawHero(byte position, char* terrainLower, unsigned int score) {
  bool collide = false;
  char lowerSave = terrainLower[HERO_HORIZONTAL_POSITION];
  byte lower;
  switch (position) {
    case HERO_POSITION_OFF:
      lower = SPRITE_TERRAIN_EMPTY;
      break;
    case HERO_POSITION_RUN_LOWER_1:
      lower = SPRITE_RUN1;
      break;
    case HERO_POSITION_RUN_LOWER_2:
      lower = SPRITE_RUN2;
      break;
  }
  if (lower != ' ') {
    terrainLower[HERO_HORIZONTAL_POSITION] = lower;

    if (lowerSave == SPRITE_TERRAIN_EMPTY) {
        collide = false;
    } else if (lowerSave == SPRITE_UP && digitalRead(UP_BUTTON) == LOW) {
        collide = false;
    } else if (lowerSave == SPRITE_DOWN && digitalRead(DOWN_BUTTON) == LOW) {
        collide = false;
    } else if (lowerSave == SPRITE_LEFT && digitalRead(LEFT_BUTTON) == LOW) {
        collide = false;
    } else if (lowerSave == SPRITE_RIGHT && digitalRead(RIGHT_BUTTON) == LOW) {
        collide = false;
    } else {
        collide = true;
    }
  }
  
  byte digits = (score > 9999) ? 5 : (score > 999) ? 4 : (score > 99) ? 3 : (score > 9) ? 2 : 1;
  // Draw the scene
  terrainLower[TERRAIN_WIDTH] = '\0';
  lcd.setCursor(0,1);
  lcd.print(terrainLower);
  lcd.setCursor(16 - digits,0);
  lcd.print(score);
  terrainLower[HERO_HORIZONTAL_POSITION] = lowerSave;
  return collide;
}

// Handle the button push as an interrupt
void buttonPush() {
  buttonPushed = true;
}

void setup(){
  pinMode(PIN_READWRITE, OUTPUT);
  digitalWrite(PIN_READWRITE, LOW);
  pinMode(PIN_CONTRAST, OUTPUT);
  digitalWrite(PIN_CONTRAST, LOW);
  pinMode(PIN_BUTTON, INPUT);
  digitalWrite(PIN_BUTTON, HIGH);
  pinMode(UP_BUTTON, INPUT);
  digitalWrite(UP_BUTTON, HIGH);
  pinMode(DOWN_BUTTON, INPUT);
  digitalWrite(DOWN_BUTTON, HIGH);
  pinMode(LEFT_BUTTON, INPUT);
  digitalWrite(LEFT_BUTTON, HIGH);
  pinMode(RIGHT_BUTTON, INPUT);
  digitalWrite(RIGHT_BUTTON, HIGH);
  pinMode(PIN_AUTOPLAY, OUTPUT);
  digitalWrite(PIN_AUTOPLAY, HIGH);
  //pinMode(8, OUTPUT);  
  // Digital pin 2 maps to interrupt 0
  attachInterrupt(0/*PIN_BUTTON*/, buttonPush, FALLING);
  initializeGraphics();
  lcd.begin(16, 2);
  pinMode(speaker, OUTPUT);
}

int generateRandomArrow() {
    int randomNumber = random(4); 
    switch (randomNumber) {
      case 0:
        return SPRITE_UP;
      case 1:
        return SPRITE_DOWN;
      case 2:
        return SPRITE_LEFT;
      case 3:
        return SPRITE_RIGHT;
    }
  }

void loop(){
  static byte heroPos = HERO_POSITION_RUN_LOWER_1;
  static byte newTerrainType = TERRAIN_EMPTY;
  static byte newTerrainDuration = 1;
  static bool playing = false;
  static bool blink = false;
  static unsigned int distance = 0;
  static int lastMusicNote = -1;
  
  if (!playing) {
    drawHero((blink) ? HERO_POSITION_OFF : heroPos, terrainLower, distance >> 3);
    if (blink) {
      lcd.setCursor(0,0);
      lcd.print("Press Start");
    }
    delay(250);
    lcd.clear();
    blink = !blink;
    if (buttonPushed) {
      initializeGraphics();
      heroPos = HERO_POSITION_RUN_LOWER_1;
      playing = true;
      buttonPushed = false;
      distance = 0;
      lastMusicTime = millis();
    }
    return;
  }
  
  // Shift the terrain to the left
  advanceTerrain(terrainLower, newTerrainType == TERRAIN_LOWER_BLOCK ? generateRandomArrow() : SPRITE_TERRAIN_EMPTY);

  // Make new terrain to enter on the right
  if (--newTerrainDuration == 0) {
    if (newTerrainType == TERRAIN_EMPTY) {
      newTerrainType = TERRAIN_LOWER_BLOCK;
      newTerrainDuration = beats[musicIndex];
    } else {
      newTerrainType = TERRAIN_EMPTY;
      newTerrainDuration = 1; // Default duration for empty terrain
    }
  }

  // Synchronize note playing with terrain generation
  if (newTerrainDuration == beats[musicIndex]) {
      if (notes[musicIndex] != ' ') {
          playNote(notes[musicIndex], beats[musicIndex] * tempo);
      }
      ++musicIndex;
      if (musicIndex >= length) {
          musicIndex = 0; // Reset music index if reached end of the music array
      }
}
  
  if (buttonPushed) {
    buttonPushed = false;
  }
  
  if (drawHero(heroPos, terrainLower, distance >> 3)) {
    playing = false; // The hero collided with something. Too bad.
  } else {
    if (heroPos == HERO_POSITION_RUN_LOWER_2) {
      heroPos = HERO_POSITION_RUN_LOWER_1;
    } else {
      ++heroPos;
    }
    ++distance;
    digitalWrite(PIN_AUTOPLAY, terrainLower[HERO_HORIZONTAL_POSITION + 2] == SPRITE_TERRAIN_EMPTY ? HIGH : LOW);
  }
  delay(30);
}

void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(speaker, HIGH); //Play speaker
    delayMicroseconds(tone);
    digitalWrite(speaker, LOW); //Turn off
    delayMicroseconds(tone);
  }
}

void playNote(char note, int duration) {
   //Array for notes
  char names[] = {'c', 'd', 'e', 'r', 'f', 'g', 'a', 'b', 'h', 'C', 'R', 'D', 'E'};
  int tones[] = {1915, 1700, 1519, 1600, 1432, 1275, 1136, 1100, 1014, 956, 900, 860, 783};
  for (int i = 0; i < 8; i++) {
    if (names[i] == note) {
      playTone(tones[i], duration);
    }
  }
}

void playMusic() {
  for (int i = 0; i < length; i++) {
    if (notes[i] == ' ') {
      delay(beats[i] * tempo); // rest
    } else {
      playNote(notes[i], beats[i] * tempo);
    }
    delay(tempo / 2); 
  }
}