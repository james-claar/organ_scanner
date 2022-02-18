// The purpose of this program is to generate a key map for parallel/bussed keyboards.

const int ClockPin = 8;
const int SwitchPin = 9;

const int DataInputPin = 13;

const int BuzzerPin = 11; // Used for beeping when a key is detected

const int keyboardSize = 42; // Number of keys on your keyboard
const int buzzerPitch = 1976; // High B


int numberOfPressedKeys = 0;
int pressedKey = 0; // Value of key that was pressed.
bool keyboardState[keyboardSize]; // Raw keyboard state as wired.

int generatedMap[keyboardSize]; // Index map for keyboard.

bool incomingBit = false; // Temporary storage of incoming shift register data

void setup() {
  Serial.begin(9600);
  
  pinMode(ClockPin, OUTPUT);
  pinMode(SwitchPin, OUTPUT);

  pinMode(DataInputPin, INPUT);

  pinMode(BuzzerPin, OUTPUT);
  
  Serial.println("Up and running!");
}

void loop() {
  setupGeneratedMap(); // Set every value in generatedMap to -1, the default value.
  
  Serial.println("Ready to map.");

  for (int i=0; i<=(keyboardSize - 1); i++) {
    while(true) { // Keep trying to add a new key until we do it successfully. This loop is to stop the program from moving on if and when the user presses the same key twice.
      while (numberOfPressedKeys != 0) { // Wait until no keys are held.
        tone(BuzzerPin, buzzerPitch); // During this loop, at least one key is held. Turn on the buzzer.
        updateKeyboardState();
        delay(1);
      }
      while (numberOfPressedKeys != 1) { // Wait until one key is held.
        noTone(BuzzerPin); // During this loop, no keys are pressed. Turn off the buzzer.
        updateKeyboardState();
        delay(1);
      }
      
      // At this point, one key is pressed. Even if the user stops pushing the key, it should still be in the list at this point.
      pressedKey = getKeyboardIndex();
      
      if (generatedMap[pressedKey] != -1) { // We've pressed this key before.
        continue;
      } else { // Else, get out of this infinite loop.
        break;
      }
    }
    
    generatedMap[pressedKey] = i; // Load this key into the map.
    
    Serial.print("New key "); // Notify the user.
    Serial.print(i);
    Serial.print(", ");
    Serial.println(pressedKey);
  }

  Serial.print("\n\n\n\n\n\n\nGenerated Map:\n");
  printGeneratedMap();
}

void updateKeyboardState() {
  int inputNo = 0;

  numberOfPressedKeys = 0;

  digitalWrite(SwitchPin, HIGH); // Tell the registers to collect parallel data
  delayMicroseconds(20); // Allow the registers time to collect parallel data
  digitalWrite(SwitchPin, LOW); // Tell the registers to start sending serial data
  
  for (int i=0; i<=(keyboardSize-1); i++) {
    digitalWrite(ClockPin, 0);
    if (inputNo >= keyboardSize) { // Previous loop just read the final key, end the function now
      return;
    }
    
    digitalWrite(ClockPin, 0);
    delayMicroseconds(2);
    
    incomingBit = (digitalRead(DataInputPin) == HIGH);
    
    keyboardState[inputNo] = incomingBit;

    if (incomingBit == true) { // If this key is pressed, add it to the running total of pressed keys.
      numberOfPressedKeys++;
    }
    
    inputNo++;
    digitalWrite(ClockPin, 1);
  }
}

int getKeyboardIndex() { // Assumes one key is pressed.
  for (int i=0; i<=(keyboardSize-1); i++) {
    if (keyboardState[i]) {
      return i;
    }
  }

  Serial.println("WARNING: VALUE NOT FOUND IN ARRAY. PLEASE UPDATE YOUR CODE.");
}

void setupGeneratedMap() {
  for (int i=0; i<=(keyboardSize-1); i++) {
    generatedMap[i] = -1;
  }
}

void printGeneratedMap() {
  Serial.print("{ ");
  
  for (int i=0; i<=(keyboardSize-1); i++) {
    Serial.print(generatedMap[i]);
    if (i <= (keyboardSize-2)) { // Not the last element
      Serial.print(", ");
    }
  }
  
  Serial.print(" }");
}
