/*
  Connection checker for testing circuits.

  Allows pins to be defined along with the connections between them.
  Checks for missing and bridged connections.

  Connect to Arduino with 115200 baud serial, press enter to test.
*/
#define RED_LED 12
#define GREEN_LED 11

// Define the pin names for printing in the debug console.
String pinNames[] = {
  "PROC1_SENS6",      // 0
  "PROC2_SENS4",      // 1
  "PROC3_SENS2",      // 2
  "PROC4_3V",         // 3
  "PROC5_SENS1",      // 4
  "PROC6_SENS3",      // 5
  "PROC7_SENS5",      // 6
  "PROC8_SENS7",      // 7

  "CHAIN1_SENS7",     // 8
  "CHAIN2_SENS5",     // 9
  "CHAIN3_SENS3",     // 10
  "CHAIN4_3V",        // 11
  "CHAIN5_SENS2",     // 12
  "CHAIN6_SENS4",     // 13
  "CHAIN7_SENS6",     // 14
  "CHAIN8_NC"         // 15
  };

// Define which Arduino pin the test pin is connected to. -1 means not connected (-1) and just used for organisation
int pins[] = {
  // To Processor
  -1, // NC           // 0
  2,                  // 1
  3,                  // 2
  4,                  // 3
  -1, // NC           // 4
  -1, // NC           // 5
  -1, // NC           // 6
  -1, // NC           // 7

  // To Chain
  -1, // NC           // 8
  -1, // NC           // 9
  -1, // NC           // 10
  5,                  // 11
  6,                  // 12
  7,                  // 13
  -1, // NC           // 14
  -1  // NC           // 15
  }; 

// Define which pins need to be connected together. Uses the array index from the pins array
int connections[][2] = {
  { 3, 11 },  // PROC4_3V     -   CHAIN4_3V
  { 1, 13 },  // PROC2_SENS4  -   CHAIN6_SENS4
  { 2, 12 }   // PROC3_SENS2  -   CHAIN5_SENS2
  };

int pinCount = sizeof(pins) / sizeof(int);

// the setup function runs once when you press reset or power the board
void setup() {
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  // Initialize all pins to output HIGH
  for(int i = 0; i < sizeof(pins) / sizeof(int); ++i) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], HIGH);
  }

  // Start serial communication
  Serial.begin(115200);
  Serial.println("Tester ready.");
}

// the loop function runs over and over again forever
// Checks two pins at a time (A and B), checks for both missing and present connections
void loop() {
  // Turn LEDs off
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  bool error = false;
  
  for(                                      // Iterate over all the pins
    int pinAIndex = 0;                      // Start at pin at index 0
    pinAIndex < pinCount; // Stop at the last pin.
    ++pinAIndex) {                          // Increment pinIndex by 1 each loop

    String pinAName = pinNames[pinAIndex];  // Get pin name
    int pinAValue = pins[pinAIndex];        // Get the arduino pin that the test A pin is on
    if(pinAValue == -1) continue;           // Skip pins marked not connected (-1).
    
    pinMode(pinAValue, OUTPUT);             // Set pin A to be output
    digitalWrite(pinAValue, LOW);           // Output 1 on pin A
    
    
    for(                                      // Iterate over all the pins again
      int pinBIndex = 0;                      // Start at pin at index 0
      pinBIndex < pinCount; // Stop at the last pin.
      ++pinBIndex) {
      
      int pinBValue = pins[pinBIndex];        // Get the arduino pin that the test B pin is on
      String pinBName = pinNames[pinBIndex];  // Get pin name
      if(
        pinBValue == -1 || 
        pinAIndex == pinBIndex)
          continue;         // Skip pins marked not connected (-1) OR when pin A is the same as pin B OR checked pins
      
      pinMode(pinBValue, INPUT_PULLUP);             // Set pin B to be input
      bool isConnected = !digitalRead(pinBValue);   // Read value on pin B
      bool connectionExpected = isConnectionExpected(pinAIndex, pinBIndex); // Gets the expected connection state of pins A and B
      Serial.print("Testing: " + pinAName + "\t-\t" + pinBName + ":\t");

      // Compare test results with expected
      if(isConnected == connectionExpected)   // Success
        Serial.println("\tSuccess");
      else if(connectionExpected) {           // Connection was expected
        error = true;
        Serial.println("\tFAIL! Missing");
      } else {                                // Connection was not expected
        error = true;
        Serial.println("\tFAIL! Bridged");
      }
    }

    digitalWrite(pinAValue, HIGH);            // Output 0 on pin A, resets for the next test
  }

  // Set output LEDs
  digitalWrite(RED_LED, error);
  digitalWrite(GREEN_LED, !error);
  if(error)
    Serial.println("Test Completed with errors.");
  else
    Serial.println("Test Completed successfully.");
    
  while(!Serial.available()){} // Wait for serial input
  while(Serial.available()){Serial.read();} // Clear the serial buffer so that the previous while loop waits for more input
  Serial.println("\n\n----------------------------------------------------------\n\n");
}

// Uses connections array to check if a pin A and pin B are supposed to be connected
bool isConnectionExpected(int indexA, int indexB) {
  for(int i = 0; i < sizeof(connections) / (sizeof(int) * 2); ++i) {
    int* con = connections[i];
    
    // Return true for any connection that has A and B in it
    if(
      (con[0] == indexA && con[1] == indexB) || 
      (con[0] == indexB && con[1] == indexA)) 
      return true;
  }

  // No connection found
  return false;
}
