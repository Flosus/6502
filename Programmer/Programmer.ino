/*
Define constants
*/
#define PIN_LED_RED 12
#define PIN_LED_GREEN 13
#define PIN_OUTPUT_ENABLED 6
#define PIN_WRITE_ENABLED 11
#define PIN_CHIP_ENABLED 4

const int PIN_DATA[] = { 27, 28, 29, 16, 15, 14, 2, 3 };
#define dataPinCount 8
const int PIN_ADDRESS[] = { 26, 25, 24, 23, 22, 21, 20, 19, 9, 8, 5, 7, 18, 10, 17 };
#define addressPinCount 15

// Number of words (bytes)
#define MAX_WORD 32767

#define writeCycleWait 10
#define DATA_BUFFER_SIZE 69
#define COMMAND_WRITE "W"
#define COMMAND_READ "R"

/*
Setup and Setup helper functions
*/

void setDataPins(int mode) {
  for (int i = 0; i < dataPinCount; i++) {
    pinMode(PIN_DATA[i], mode);
  }
}

void setChipEnabled(int val) {
  digitalWrite(PIN_CHIP_ENABLED, val);
}

void setWriteEnabled(int val) {
  digitalWrite(PIN_WRITE_ENABLED, val);
}

void setOutputEnabled(int val) {
  digitalWrite(PIN_OUTPUT_ENABLED, val);
}

void enablePins() {
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_OUTPUT_ENABLED, OUTPUT);
  pinMode(PIN_WRITE_ENABLED, OUTPUT);
  pinMode(PIN_CHIP_ENABLED, OUTPUT);
  for (int i = 0; i < addressPinCount; i++) {
    pinMode(PIN_ADDRESS[i], OUTPUT);
  }
}

void setAddress(int a) {
  byte tmp = a;
  for (int i = 0; i < addressPinCount; i++) {
    bool enabled = tmp&1;
    tmp = (tmp >> 1);
    digitalWrite(PIN_ADDRESS[i], enabled ? HIGH : LOW);
  }
}

void setData(byte b) {
  byte tmp = b;
  for (int i = 0; i < dataPinCount; i++) {
    bool enabled = tmp&1;
    tmp = (tmp >> 1);
    digitalWrite(PIN_DATA[i], enabled ? HIGH : LOW);
  }
}

byte readData() {
  byte data = 0;
  for (int i = dataPinCount - 1; i >= 0; i--) {
    int dataRaw = digitalRead(PIN_DATA[i]);
    data = (data << 1) + dataRaw;
  }
  return data;
}

byte read(int address) {
  setDataPins(INPUT);
  setAddress(address);
  setChipEnabled(LOW);
  delayMicroseconds(writeCycleWait);
  byte data = readData();
  setChipEnabled(HIGH);
  return data;
}

void read(byte buffer[], int startAddress, int readSize) {
  digitalWrite(PIN_LED_RED, HIGH);
  setWriteEnabled(HIGH);
  setOutputEnabled(LOW);
  for (int w = 0; w < readSize; w++) {
    buffer[w] = read(startAddress + w);
  }
  digitalWrite(PIN_LED_RED, LOW);
}

void write(int address, byte data) {
  setWriteEnabled(HIGH);
  setOutputEnabled(HIGH);
  delayMicroseconds(writeCycleWait);
  setDataPins(OUTPUT);
  setOutputEnabled(HIGH);
  setAddress(address);
  setChipEnabled(LOW);
  setWriteEnabled(LOW);
  delayMicroseconds(1);
  setData(data);
  delayMicroseconds(1);
  setWriteEnabled(HIGH);
  setChipEnabled(HIGH);
  setOutputEnabled(LOW);
  delayMicroseconds(1);
}

void write(byte * data, int startAddress, int size) {
  for (int i = 0; i < size; i++) {
    write(startAddress + i, data + i);
  }
}

void setup() {
  Serial.begin(9600);
  printString("SETUP");
  enablePins();
  printString("SETUP_FINISHED");
}

byte dataBuffer[DATA_BUFFER_SIZE];

String command = "_";
int commandAddress = -1;
int commandSize = -1;
byte * data = NULL;
bool isRunningCommand = false;
bool isExpectingData = false;

void loop() {
  while (!Serial) {}
  printString("READY");
  while (Serial.available() == 0) {}
  int read = Serial.readBytesUntil('\n', dataBuffer, DATA_BUFFER_SIZE);
  char * inputChars =  (char*) malloc(read * (sizeof(char)));
  strncpy(inputChars, dataBuffer, read);
  String input = String(inputChars);

  /* Inputs:
    * CMD_START
    * CMD_RESET
    * CMD_R
    * CMD_W
    * CMD_A<Address>
    * CMD_S<Size>
    * CMD_D<DATA>
  */
  if (isRunningCommand) {
    printString("BUSY");
  } else if (input.startsWith("CMD_START")) {
    cmdStart();
  } else if (input.startsWith("CMD_RESET")) {
    cmdReset();
  } else if (input.startsWith("CMD_R")) {
    command = COMMAND_READ;
  } else if (input.startsWith("CMD_W")) {
    command = COMMAND_WRITE;
  } else if (input.startsWith("CMD_A")) {
    commandAddress = input.substring(5).toInt();
  } else if (input.startsWith("CMD_S")) {
    commandSize = input.substring(5).toInt();
  } else if (input.startsWith("CMD_D")) {
    data = dataBuffer + 5; 
  } else {
    printString("INVALID_INPUT_'" + input + "'");
  }
  printString("ECHO#" + input);
}

void printString(String msg) {
  Serial.print("RESP" + msg + "END");
}

void cmdStart() {
  if (!isCommandValid()) {
    printString("INCOMPLETE_COMMAND_" + command + "_" + commandAddress + "_" + commandSize);
  }
  if (command == COMMAND_READ) {
    read(dataBuffer, commandAddress, commandSize);
    Serial.print("BIN");
    Serial.write(dataBuffer, commandSize);
    Serial.print("END");
    delayMicroseconds(1);
  } else if (command == COMMAND_WRITE) {
    
  }
  cmdReset();
}

void cmdReset() {
  command = "_";
  commandAddress = -1;
  commandSize = -1;
  data = NULL;
  isRunningCommand = false;
  memset(dataBuffer, 0, sizeof(dataBuffer));
  printString("RESET");
}

bool isCommandValid() {
  return (command == "R" || command == "W" && data != NULL)
    && commandAddress < MAX_WORD
    && commandSize > 0;
}
