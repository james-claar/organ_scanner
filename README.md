# organ_scanner
Some Arduino code to scan my organ's keyboards and send them to the computer via USB MIDI.

KeyboardMapGenerator is used temporarily to find which order the keys are in. When used, the user presses each key from left to right and the program spits out the keymap.

OrganMidiController is the code that normally runs on the Arduino Leonardo. It handles the keyboards, the expression pedal (volume), the foot pedals, and the stops, and reads them using daisy chains of shift registers. The Leonardo pretends to be a MIDI device, and sends any note value changes to the computer, which is running the virtual pipe organ software Grandorgue. This code would work with Hauptwerk (another virtual pipe organ software) as well.
