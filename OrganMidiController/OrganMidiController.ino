// The purpose of this program is to control the shift registers on the breadboards to obtain keystates, convert that information to MIDI, and send it via MIDI cable to the synthesizer.

#include "MIDIUSB.h" // Used to send MIDI messages to the computer

// PIN CONFIG
// Note: it is assumed that the MIDI is being sent out the serial interface so therefore does not need a pin, as 'Serial.begin()' opens pin TX to be used as a serial output.
int ClockPin = 8; // To tell the shift registers we are ready for the next bit.
int SwitchPin = 9; // Used to switch the shift registers between collecting input data and outputting serial data.

// (data inputs)
int ExpressionPedalPin = A0; // Expression pedal

int GreatPin = 11;
int SwellPin = 12;
int PedalPin = 10;
int StopsPin = 13;

// VARIABLE CONFIG
int medianAnalogReadData[30]; // For MedianAnalogRead to store temporary data.
int ExpressionPedalCalibrationRange[2] = { 730, 1000 }; // Lowest, then highest values in range of analog readings from expression pedal. It may need recalibrated so potentially add an option for that.
int DesiredExpressionPedalMIDIRange[2] = { 0, 127}; // Output volume range, within MIDI spec (0 - 127)

const int expressionWaitTime = 10; // Only send expression pedal MIDI messages this many milliseconds after the previous one.
const int expressionSimilarityThreshold = 3; // How close a reading needs to be to be considered equal to the current state.

const int UsbMidiBaudRate = 115200; //default 31250;

const int maxNumberOfInputs = 61; // Used so we stop pulsing the clock pin and reading data when we run out of inputs.

const int greatMap[61] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 48, 20, 21, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 19, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 22 }; // Re-ordering of wires. I tried to wire this manual in order and made a few mistakes.
const int swellMap[61] = { 40, 39, 50, 52, 42, 58, 46, 49, 48, 41, 44, 38, 60, 54, 51, 56, 36, 35, 37, 30, 31, 32, 34, 26, 27, 29, 33, 28, 23, 21, 47, 25, 43, 0, 24, 45, 55, 57, 22, 11, 6, 19, 20, 59, 17, 16, 15, 14, 7, 13, 12, 53, 8, 4, 3, 5, 1, 10, 2, 9, 18 };
const int pedalMap[32] = { 24, 25, 3, 8, 19, 11, 17, 27, 2, 14, 6, 7, 23, 1, 21, 10, 15, 13, 29, 18, 20, 9, 4, 0, 30, 16, 12, 22, 31, 26, 5, 28 }; // Randomly ordered
const int stopsMap[42] = { 27, 4, 24, 5, 36, 32, 23, 0, 33, 29, 20, 35, 30, 41, 19, 40, 16, 37, 25, 34, 14, 17, 15, 28, 3, 31, 18, 11, 22, 6, 21, 26, 13, 8, 12, 38, 39, 7, 1, 9, 10, 2 };

const int keyboardOffset = 36; // How much to add to the midi note value to ensure, for example, that note 0 on the keyboard will be sent as Low C and not some other note.
const int pedalOffset = 36;

const int pedalMIDIChannel = 1;
const int greatMIDIChannel = 2;
const int swellMIDIChannel = 3;
const int stopsMIDIChannel = 4;
const int expressionMIDIChannel = 5;

int tempIndex = 0;

bool greatData[61] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // 61 Keys
bool swellData[61] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // 61 Keys
bool pedalData[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // 32 Keys
bool stopsData[42] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // 42 Stops

bool deltaGreat[61] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // 61 Keys
bool deltaSwell[61] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // 61 Keys
bool deltaPedal[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // 32 Keys
bool deltaStops[42] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // 42 Stops


// DATA VAR SETUP

int reading; // For storing the Expression Pedal state
float tempMidiExpression;
int midiExpression;
int lastSentExpression = -9999; // Arbitrary number that won't interfere with our volume (normal range is 0-127)

int medianAnalogReadDataLen = sizeof(medianAnalogReadData) / sizeof(medianAnalogReadData[0]);

unsigned long MIDIExpressionTimeCheck = millis(); // Time since expression pedal value was sent, used to avoid flooding host with expression pedal changes
int tempDifference;

void setup() {
  Serial.begin(UsbMidiBaudRate);
  pinMode(ExpressionPedalPin, INPUT);

  pinMode(ClockPin, OUTPUT);
  pinMode(SwitchPin, OUTPUT);

  pinMode(GreatPin, INPUT);
  pinMode(SwellPin, INPUT);
  pinMode(PedalPin, INPUT);
  pinMode(StopsPin, INPUT);
}

void loop() {
  getKeysData();
  sendMidiMessages();
  updateExpression();
  
  delay(1); // Scan slow enough to avoid key bouncing, while still being responsive.
}

void midiControlChange(byte channel, byte value) {
  byte arg2 = (0xC0 | channel-1);
  midiEventPacket_t message = {0x09, arg2, value};

  MidiUSB.sendMIDI(message);
    
  MidiUSB.flush();
}

void queueMidiNote(byte channel, byte note, byte velocity) {
  byte arg2 = (0x90 | channel-1);
  midiEventPacket_t noteOn = {0x09, arg2, note, velocity};
    
  MidiUSB.sendMIDI(noteOn);
}

// Utility function used by selectionSort, swaps two ints
void swap(int* xp, int* yp) 
{ 
   int temp = *xp; 
   *xp = *yp; 
   *yp = temp; 
}

// Function to perform Selection Sort 
void selectionSort(int arr[], int n) {
  int i, j, min_idx; 

  // One by one move boundary of unsorted subarray 
  for (i = 0; i < n - 1; i++) {

    // Find the minimum element in unsorted array 
    min_idx = i; 
    for (j = i + 1; j < n; j++) {
      if (arr[j] < arr[min_idx]) {
        min_idx = j;
      }
    }

    // Swap the found minimum element 
    // with the first element 
    swap(&arr[min_idx], &arr[i]); 
  }
}

int medianAnalogRead(int pin, int tries) {
  //Note: do not read more times than meadianAnalogReadData can store. It will attempt to write to a non-existing position in the array.
  for (int i = 0; i < tries; i++) {
    reading = analogRead(pin);
    medianAnalogReadData[i] = reading;
    
    delayMicroseconds(10); // Arbitrary delay
  }

  selectionSort(medianAnalogReadData, tries); // Sort data from lowest to highest
  
  return medianAnalogReadData[(int)tries/2]; // Pick median datapoint from list and return it
}

void updateExpression() {
  midiExpression = getExpressionPedalState();

  sendFilteredExpression(midiExpression);
}

int getExpressionPedalState() {
  reading = medianAnalogRead(ExpressionPedalPin, 5); // Get an accurate representation of the position of the expression pedal. We don't want any volume spikes if a read or two is wonky!
  
  tempMidiExpression = map(reading, ExpressionPedalCalibrationRange[1], ExpressionPedalCalibrationRange[0], DesiredExpressionPedalMIDIRange[0], DesiredExpressionPedalMIDIRange[1]); // Map the pedal's calibration range to the desired range within standard MIDI volume range.
  
  if (tempMidiExpression <= 0+expressionSimilarityThreshold) {
    tempMidiExpression = 0;
  }
  if (tempMidiExpression >= 127-expressionSimilarityThreshold) {
    tempMidiExpression = 127;
  }
  
  return tempMidiExpression;
}

void sendFilteredExpression(int value) {
  tempDifference = abs(value - lastSentExpression); // Difference between value and current reading

  if (millis() > MIDIExpressionTimeCheck + expressionWaitTime and tempDifference > expressionSimilarityThreshold) {
    midiControlChange(1, value);
    lastSentExpression = value;
    MIDIExpressionTimeCheck = millis();
  }
}


void getKeysData() {
  bool greatTemp = 0;
  bool swellTemp = 0;
  bool pedalTemp = 0;
  bool stopsTemp = 0;

  //we will be holding the clock pin high 8 times (0,..,7) at the
  //end of each time through the for loop
  
  //at the begining of each loop when we set the clock low, it will
  //be doing the necessary low to high drop to cause the shift
  //register's DataPin to change state based on the value
  //of the next bit in its serial information flow.

  digitalWrite(SwitchPin, HIGH); // Tell the registers to collect parallel data
  delayMicroseconds(20); // Allow the registers time to collect parallel data
  digitalWrite(SwitchPin, LOW); // Tell the registers to start sending serial data
  
  for (int currentInput = 0; currentInput <= (maxNumberOfInputs-1); currentInput++) {
    digitalWrite(ClockPin, 0);
    delayMicroseconds(2);
    
    greatTemp = (digitalRead(GreatPin) == HIGH); // Read all the incoming data streams from the keys and stops, one key at a time.
    swellTemp = (digitalRead(SwellPin) == HIGH);
    pedalTemp = (digitalRead(PedalPin) == HIGH);
    stopsTemp = (digitalRead(StopsPin) == HIGH);
    
    if (currentInput <= (32 - 1)) { // We haven't run out of pedal keys (subtracting 1 because we are counting from zero.)
      tempIndex = pedalMap[currentInput];
      deltaPedal[tempIndex] = (pedalData[tempIndex] ^ pedalTemp); // Perform bitwise XOR (True if the value has changed, false if it has not) to see if the note has changed, and write it to the delta list.
      pedalData[tempIndex] = pedalTemp;
    }
    
    if (currentInput <= (42 - 1)) { // We haven't run out of stops
      tempIndex = stopsMap[currentInput];
      deltaStops[tempIndex] = (stopsData[tempIndex] ^ stopsTemp);
      stopsData[tempIndex] = stopsTemp;
    }
    
    if (tempIndex <= (61 - 1)) { // We haven't run out of keys on either manual
      tempIndex = greatMap[currentInput];
      deltaGreat[tempIndex] = (greatData[tempIndex] ^ greatTemp);
      greatData[tempIndex] = greatTemp;
    
      tempIndex = swellMap[currentInput];
      deltaSwell[tempIndex] = (swellData[tempIndex] ^ swellTemp);
      swellData[tempIndex] = swellTemp;
    }
    
    digitalWrite(ClockPin, 1);

    // Write the information to the lists and see if it has changed.
  }
}

void sendMidiMessages() {
  for (int currentNote = 0; currentNote <= (maxNumberOfInputs-1); currentNote++) {
    if (currentNote <= (32 - 1)) { // We haven't run out of pedal keys (subtracting 1 because we are counting from zero.)
      if (deltaPedal[currentNote]) {
        if (pedalData[currentNote]) { // Note was just pressed
          queueMidiNote(pedalMIDIChannel, currentNote + pedalOffset, 63); 
        } else { // Note was just released
          queueMidiNote(pedalMIDIChannel, currentNote + pedalOffset, 0);
        }
      }
    }

    if (currentNote <= (61 - 1)) { // We haven't run out of keys on the keyboards
      if (deltaGreat[currentNote]) {
        if (greatData[currentNote]) { // Note was just pressed
          queueMidiNote(greatMIDIChannel, currentNote + keyboardOffset, 63); 
        } else { // Note was just released
          queueMidiNote(greatMIDIChannel, currentNote + keyboardOffset, 0);
        }
      }

      if (deltaSwell[currentNote]) {
        if (swellData[currentNote]) { // Note was just pressed
          queueMidiNote(swellMIDIChannel, currentNote + keyboardOffset, 63); 
        } else { // Note was just released
          queueMidiNote(swellMIDIChannel, currentNote + keyboardOffset, 0);
        }
      }
    }

    if (currentNote <= (42 - 1)) {
      if (deltaStops[currentNote]) {
        if (stopsData[currentNote]) { // Note was just pressed
          queueMidiNote(stopsMIDIChannel, currentNote, 63);
        } else { // Note was just released
          queueMidiNote(stopsMIDIChannel, currentNote, 0);
        }
      }
    }
  }
  
  MidiUSB.flush(); // Send the queued midi messages
}
