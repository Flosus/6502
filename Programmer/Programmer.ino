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

/*
Setup and Setup helper functions
*/

void serialPrintBinary(int data) {
  String binary = "";
  for (int i = 0; i < 16; i++)
  {
    bool b = data & 1;
    binary = (b ? "1" : "0") + binary;
    data = data >> 1;
  }
  Serial.print(binary);
}

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
  Serial.println("Setup address Pins");
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

byte * read(int startAddress, int readSize) {
  digitalWrite(PIN_LED_RED, HIGH);
  setWriteEnabled(HIGH);
  setOutputEnabled(LOW);
  byte * result;
  result = (byte*)malloc(readSize*(sizeof(byte)));
  for (int w = 0; w < readSize; w++) {
    result[w] = read(startAddress + w);
  }
  digitalWrite(PIN_LED_RED, LOW);
  return result;
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
  Serial.println("SETUP");
  enablePins();
  Serial.println("SETUP_FINISHED");
}

void loop() {
  while (!Serial) {}
  while (Serial.available() == 0) {}
  String command = Serial.readStringUntil('\n');
  if (command == "READ") {
    int address = 0; //Serial.readStringUntil('\n').toInt();
    int size = 100; // Serial.readStringUntil('\n').toInt();
    byte * readData = read(address, size);
    Serial.write(readData, size);
  } else if (command == "WRITE") {
    int address = Serial.readStringUntil('\n').toInt();
    int size = Serial.readStringUntil('\n').toInt();
    byte * dataBuffer = (byte*)malloc(size*(sizeof(byte)));
    int pointerEnd = Serial.readBytes(dataBuffer, size);
    write(dataBuffer, address, size);
  }
  else {
    Serial.println("UNKNOWN_COMMAND: " + command);
  }
}
