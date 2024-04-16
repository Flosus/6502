/*
Define constants
*/
#define PIN_LED_RED 12
#define PIN_LED_GREEN 13
#define PIN_LED_IS_READING 32
#define PIN_LED_IS_WRITING 33
#define PIN_CHIP_ENABLED 28
#define PIN_WRITE_ENABLED 29
#define PIN_OUTPUT_ENABLED 30

const int PIN_DATA[] = { 46, 47, 48, 49, 50, 51, 52, 53 };
#define dataPinCount 8
const int PIN_ADDRESS[] = { 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45 };
#define addressPinCount 15

// Number of words (bytes)
#define MAX_WORD 32767

#define writeCycleWait 10
//#define DATA_BUFFER_SIZE 64 + 5
#define DATA_BUFFER_SIZE 1024 + 5
#define MESSAGE_BUFFER_SIZE 64
#define COMMAND_READ "R"
#define COMMAND_READ_ALL "RA"
#define COMMAND_WRITE "W"
#define COMMAND_WRITE_ALL "WA"

const int DATA0[] = {
  0xa2, 0xff, 0x9a, 0xa9, 0xff, 0x8d, 0x02, 0x60, 0xa9, 0xe0, 0x8d, 0x03, 0x60, 0xa9, 0x38, 0x20,
  0x65, 0x80, 0xa9, 0x0e, 0x20, 0x65, 0x80, 0xa9, 0x06, 0x20, 0x65, 0x80, 0xa9, 0x01, 0x20, 0x65,
  0x80, 0xa9, 0x48, 0x20, 0x78, 0x80, 0xa9, 0x65, 0x20, 0x78, 0x80, 0xa9, 0x6c, 0x20, 0x78, 0x80,
  0xa9, 0x6c, 0x20, 0x78, 0x80, 0xa9, 0x6f, 0x20, 0x78, 0x80, 0xa9, 0x2c, 0x20, 0x78, 0x80, 0xa9,
  0x20, 0x20, 0x78, 0x80, 0xa9, 0x77, 0x20, 0x78, 0x80, 0xa9, 0x6f, 0x20, 0x78, 0x80, 0xa9, 0x72,
  0x20, 0x78, 0x80, 0xa9, 0x6c, 0x20, 0x78, 0x80, 0xa9, 0x64, 0x20, 0x78, 0x80, 0xa9, 0x21, 0x20,
  0x78, 0x80, 0x4c, 0x62, 0x80, 0x8d, 0x20, 0x60, 0xa9, 0x20, 0x8d, 0x01, 0x60, 0xa9, 0x80, 0x8d,
  0x01, 0x60, 0xa9, 0x20, 0x8d, 0x01, 0x60, 0x60, 0x8d, 0x20, 0x60, 0xa9, 0x20, 0x8d, 0x01, 0x60,
  0xa9, 0xa0, 0x8d, 0x01, 0x60, 0xa9, 0x20, 0x8d, 0x01, 0x60, 0x60 };

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

void setAddress(unsigned int a) {
  digitalWrite(PIN_ADDRESS[0], a & 1 ? HIGH : LOW);
  digitalWrite(PIN_ADDRESS[1], a & 2 ? HIGH : LOW);
  digitalWrite(PIN_ADDRESS[2], a & 4 ? HIGH : LOW);
  digitalWrite(PIN_ADDRESS[3], a & 8 ? HIGH : LOW);
  digitalWrite(PIN_ADDRESS[4], a & 16 ? HIGH : LOW);
  digitalWrite(PIN_ADDRESS[5], a & 32 ? HIGH : LOW);
  digitalWrite(PIN_ADDRESS[6], a & 64 ? HIGH : LOW);
  digitalWrite(PIN_ADDRESS[7], a & 128 ? HIGH : LOW);
  digitalWrite(PIN_ADDRESS[8], a & 256 ? HIGH : LOW);
  digitalWrite(PIN_ADDRESS[9], a & 512 ? HIGH : LOW);
  digitalWrite(PIN_ADDRESS[10], a & 1024 ? HIGH : LOW);
  digitalWrite(PIN_ADDRESS[11], a & 2048 ? HIGH : LOW);
  digitalWrite(PIN_ADDRESS[12], a & 4096 ? HIGH : LOW);
  digitalWrite(PIN_ADDRESS[13], a & 8192 ? HIGH : LOW);
  digitalWrite(PIN_ADDRESS[14], a & 16384 ? HIGH : LOW);
  delay(6);
}

void setData(byte b) {
  digitalWrite(PIN_ADDRESS[0], b & 1 ? HIGH : LOW);
  digitalWrite(PIN_ADDRESS[1], b & 2 ? HIGH : LOW);
  digitalWrite(PIN_ADDRESS[2], b & 4 ? HIGH : LOW);
  digitalWrite(PIN_ADDRESS[3], b & 8 ? HIGH : LOW);
  digitalWrite(PIN_ADDRESS[4], b & 16 ? HIGH : LOW);
  digitalWrite(PIN_ADDRESS[5], b & 32 ? HIGH : LOW);
  digitalWrite(PIN_ADDRESS[6], b & 64 ? HIGH : LOW);
  digitalWrite(PIN_ADDRESS[7], b & 128 ? HIGH : LOW);
}

byte readData() {
  byte data = 0;
  for (int i = dataPinCount - 1; i >= 0; i--) {
    int dataRaw = digitalRead(PIN_DATA[i]);
    data = (data << 1) + dataRaw;
  }
  return data;
}

byte read(unsigned int address) {
  setData(0);
  setAddress(address);
  setWriteEnabled(HIGH);
  setChipEnabled(HIGH);
  setOutputEnabled(HIGH);
  setDataPins(INPUT_PULLUP);
  delayMicroseconds(1);
  
  setChipEnabled(LOW);
  setOutputEnabled(LOW);
  delayMicroseconds(4);
  byte data = readData();
  delay(00000);
  setChipEnabled(HIGH);
  setOutputEnabled(HIGH);
  return data;
}

void read(byte buffer[], unsigned int startAddress, int readSize) {
  setWriteEnabled(HIGH);
  setOutputEnabled(LOW);
  for (int w = 0; w < readSize; w++) {
    buffer[w] = read(startAddress + w);
  }
}

void write(unsigned int address, byte data) {
  setAddress(address);
  setDataPins(OUTPUT);
  setOutputEnabled(LOW);
  setWriteEnabled(HIGH);
  setChipEnabled(HIGH);
  delayMicroseconds(1);

  setOutputEnabled(HIGH);
  setData(data);
  setChipEnabled(LOW);
  delayMicroseconds(1);

  setWriteEnabled(LOW);
  delayMicroseconds(1);

  setWriteEnabled(HIGH);
  delayMicroseconds(1);

  setChipEnabled(HIGH);
  setOutputEnabled(LOW);
  delayMicroseconds(1);
  //Serial.print("Write: ");
  //Serial.println(data, BIN);
  delay(0000);
  setData(0);
}

void writeEnsured(unsigned int address, byte data) {
  int tries = 10;
  bool success = false;
  while (!success && tries > 0) {
    delayMicroseconds(10);
    write(address, data);
    delayMicroseconds(10);
    byte readData = read(address);
    success = readData == data;
    tries--;
  }
  if (!success) {
    Serial.print("ERROR WHILE WRITING: ");
    Serial.print(data);
    Serial.print("@");
    Serial.print(address);
    byte readData = read(address);
    Serial.print(": ");
    Serial.print(readData);
    Serial.println();
  } else {
  }
}

void writeEnsured(byte * data, int startAddress, int size) {
  for (int i = 0; i < size; i++) {
    writeEnsured(startAddress + i, data[i]);
    delayMicroseconds(10);
  }
}

void writeHelloworld() {
  for (unsigned int i = 0; i <= MAX_WORD; i++) {
    if (i <= 138) {
      writeEnsured(i, DATA0[i]);
    } else if (i == 0x7ffd) {
      writeEnsured(0x7ffd, 0x80);
    } else {
      writeEnsured(i, 0);
    }
    if (i % 256 == 0)
      Serial.println(i, HEX);
  }
  for (unsigned int i = 0; i <= MAX_WORD; i++) {
    byte data = read(i);
    if (i <= 138) {
      if (data != DATA0[i]) {
        Serial.print("Invalid data read @");
        Serial.print(i);
        Serial.print(" ");
        Serial.print(data, HEX);
        Serial.print("!=");
        Serial.print(DATA0[i], HEX);
        Serial.println();
      }
    } else if (i == 0x7ffd) {
      if (data != 0x80) {
        Serial.print("Invalid data read @");
        Serial.print(i);
        Serial.print(" ");
        Serial.print(data, HEX);
        Serial.print("!=");
        Serial.print(0x80, HEX);
        Serial.println();
      }
    } else {
      if (data != 0) {
        Serial.print("Invalid data read @");
        Serial.print(i);
        Serial.print(" ");
        Serial.print(data, HEX);
        Serial.print("!=");
        Serial.print(0, HEX);
        Serial.println();
      }
    }
    if (i % 256 == 0)
      Serial.println(i, HEX);
  }
}

void setup() {
  Serial.begin(115200);
  enablePins();
  setDataPins(OUTPUT);
  while (!Serial) {}
  //writeHelloworld();
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
  digitalWrite(PIN_LED_GREEN, HIGH);
  sendStr("rdy");
  while (Serial.available() == 0) {}
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
  } else if (input.startsWith("CMD_RA")) {
    command = COMMAND_READ_ALL;
    commandAddress = 0;
    commandSize = 256;
    cmdStart();
  } else if (input.startsWith("CMD_R")) {
    command = COMMAND_READ;
  } else if (input.startsWith("CMD_WA")) {
    command = COMMAND_WRITE_ALL;
    commandAddress = 0;
    commandSize = 256;
    cmdStart();
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

void sendStr(String msg) {
  memset(messageBuffer, 0, sizeof(MESSAGE_BUFFER_SIZE));
  msg.toCharArray(messageBuffer, MESSAGE_BUFFER_SIZE);
  sendData(messageBuffer, msg.length());
}

void sendData(char * msg, int length) {
  Serial.write(msg, length);
  for (int i = length; i < MESSAGE_BUFFER_SIZE; i++) {
    Serial.write(0);
  }
}

void cmdReset() {
  command = "_";
  commandAddress = -1;
  commandSize = -1;
  data = NULL;
  isRunningCommand = false;
  memset(dataBuffer, 0, sizeof(dataBuffer));
  memset(messageBuffer, 0, sizeof(MESSAGE_BUFFER_SIZE));
  sendStr("RESET");
}

void fill(byte *data, int size, byte value){
  for (int i = 0; i < size; i++) {
    data[i] = value;
  }
}

bool isCommandValid() {
  return (command == COMMAND_READ || command == COMMAND_READ_ALL || command == COMMAND_WRITE_ALL || command == COMMAND_WRITE && data != NULL)
    && commandAddress < MAX_WORD
    && commandSize > 0;
}

void cmdStart() {
  if (!isCommandValid()) {
    sendStr("INCOMPLETE_COMMAND_[" + command + "@" + commandAddress + "+" + commandSize + "]");
    return;
  }
  if (command == COMMAND_READ) {
    read(dataBuffer, commandAddress, commandSize);
    sendStr("BIN_START_SINGLE");
    sendData(dataBuffer, commandSize);
    sendStr("BIN_END_SINGLE");
    delayMicroseconds(1);
  } else if (command == COMMAND_READ_ALL) {
    sendStr("BIN_START_ALL");
    for (unsigned int i = 0; i < 128;) {
      unsigned int address = i * commandSize;
      read(dataBuffer, address, commandSize);
      sendData(dataBuffer, commandSize);
    }
    sendStr("BIN_END_ALL");
  } else if (command == COMMAND_WRITE_ALL) {
    for (long i = 0; i <= MAX_WORD;) {
      int bytesRead = Serial.readBytes(dataBuffer, commandSize);
      writeEnsured(dataBuffer, i, bytesRead);
      i+= bytesRead;
    }
  } else if (command == COMMAND_WRITE) {
    
  }
  cmdReset();
}
