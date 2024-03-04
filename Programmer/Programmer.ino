/*
Define constants
*/
#define PIN_LED_RED 12
#define PIN_LED_GREEN 13
#define PIN_LED_IS_READING 32
#define PIN_LED_IS_WRITING 33
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
#define MESSAGE_BUFFER_SIZE 64
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
  pinMode(PIN_LED_IS_READING, OUTPUT);
  pinMode(PIN_LED_IS_WRITING, OUTPUT);
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
  setWriteEnabled(HIGH);
  setChipEnabled(HIGH);
  setOutputEnabled(HIGH);
  setDataPins(INPUT);
  setAddress(address);
  // delay because idk
  delayMicroseconds(6500);
  setChipEnabled(LOW);
  setOutputEnabled(LOW);
  byte data = readData();
  setChipEnabled(HIGH);
  setOutputEnabled(HIGH);
  return data;
}

void read(byte buffer[], int startAddress, int readSize) {
  setWriteEnabled(HIGH);
  setOutputEnabled(LOW);
  for (int w = 0; w < readSize; w++) {
    buffer[w] = read(startAddress + w);
  }
}

void write(int address, byte data) {
  setDataPins(OUTPUT);
  setOutputEnabled(LOW);
  setWriteEnabled(HIGH);
  setChipEnabled(HIGH);
  setOutputEnabled(HIGH);
  setAddress(address);
  setChipEnabled(LOW);
  setWriteEnabled(LOW);
  setData(data);
  setWriteEnabled(HIGH);
  setChipEnabled(HIGH);
  setOutputEnabled(LOW);
}

void writeEnsured(int address, byte data) {
  int tries = 10;
  bool success = false;
  while (!success && tries > 0) {
    write(address, data);
    byte readData = read(address);
    success = readData == data;
    tries--;
    if (!success) {
      Serial.print("ERROR WHILE WRITING");
      Serial.print(data);
      Serial.print("@");
      Serial.print(address);
      Serial.print(" != ");
      Serial.print(readData);
      Serial.println();
    }
  }
}

void write(byte * data, int startAddress, int size) {
  for (int i = 0; i < size; i++) {
    byte * dataBytePointer = data + i;
    byte dataByte = &(dataBytePointer);
    write(startAddress + i, dataByte);
    delayMicroseconds(1);
  }
}

void setup() {
  Serial.begin(9600);
  enablePins();
  sendStr("setup_end");
}

byte dataBuffer[DATA_BUFFER_SIZE];
byte messageBuffer[MESSAGE_BUFFER_SIZE];

String command = "_";
int commandAddress = -1;
int commandSize = -1;
byte * data = NULL;
bool isRunningCommand = false;
bool isExpectingData = false;

void loop() {
  while (!Serial) {}
  digitalWrite(PIN_LED_GREEN, HIGH);
  while (Serial.available() == 0) {}
  sendStr("rdy");
  memset(dataBuffer, 0, sizeof(dataBuffer));
  memset(messageBuffer, 0, sizeof(MESSAGE_BUFFER_SIZE));
  int read = Serial.readBytesUntil('\n', dataBuffer, DATA_BUFFER_SIZE);
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_RED, HIGH);
  char * inputChars =  (char*) malloc((read * (sizeof(char)) + 1));
  strncpy(inputChars, dataBuffer, read);
  inputChars[read] = NULL;
  String input = String(inputChars);
  if (isRunningCommand) {
    sendStr("BUSY");
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
  } else if (input.startsWith("ECHO")) {
    String echoText = input.substring(4);
    sendStr(echoText);
  } else {
    String str = "INVALID_INPUT_'" + input + "'";
    sendStr(str);
  }
  free(inputChars);
  digitalWrite(PIN_LED_RED, LOW);
}

bool startsWith(char * input) {

}

void sendStr(String msg) {
  memset(messageBuffer, 0, sizeof(MESSAGE_BUFFER_SIZE));
  msg.toCharArray(messageBuffer, MESSAGE_BUFFER_SIZE);
  sendData(messageBuffer, msg.length());
}

void sendData(char * msg, int length) {
  for (int i = 0; i < MESSAGE_BUFFER_SIZE; i++) {
    if (i < length) {
      Serial.print(msg[i]);
    } else {
      Serial.print(0);
    }
  }
}

void cmdStart() {
  if (!isCommandValid()) {
    sendStr("INCOMPLETE_COMMAND_[" + command + "@" + commandAddress + "+" + commandSize + "]");
  }
  if (command == COMMAND_READ) {
    read(dataBuffer, commandAddress, commandSize);
    sendStr("BIN_START");
    sendData(dataBuffer, commandSize);
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
  sendStr("RESET");
}

void fill(byte *data, int size, byte value){
  for (int i = 0; i < size; i++) {
    data[i] = value;
  }
}

bool isCommandValid() {
  return (command == "R" || command == "W" && data != NULL)
    && commandAddress < MAX_WORD
    && commandSize > 0;
}
