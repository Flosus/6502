/*
Define constants
*/
const int PIN_LED_RED = 12;
const int PIN_LED_GREEN = 13;

const int PIN_OUTPUT_ENABLED = 6;
const int PIN_WRITE_ENABLED = 11;
const int PIN_CHIP_ENABLED = 4;

const int PIN_DATA[] = { 27, 28, 29, 16, 15, 14, 2, 3 };
const int dataPinCount = 8;
const int PIN_ADDRESS[] = { 26, 25, 24, 23, 22, 21, 20, 19, 9, 8, 5, 7, 18, 10, 17 };
const int addressPinCount = 15;

bool READ_MODE = true;
bool WRITE_MODE = true;

// Number of words (bytes)
//int MAX_WORD = 32767;
int MAX_WORD = 64;

static const long int writeCycleWait = 10;

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

void readAll() {
  digitalWrite(PIN_LED_RED, HIGH);
  setWriteEnabled(HIGH);
  setOutputEnabled(LOW);
  Serial.println("Starting to read all bytes");
  for (int w = 0; w <= MAX_WORD; w++) {
    serialPrintBinary(w);
    Serial.print(": ");
    byte data = read(w);
    Serial.print(data, HEX);
    Serial.println();
  }
  Serial.println("Finished to read all bytes");
  digitalWrite(PIN_LED_RED, LOW);
}

void write(int address, byte data) {
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

void write() {
  setWriteEnabled(HIGH);
  setOutputEnabled(HIGH);
  //Serial.println("Writing 170");
  int val = 0;
  for (int w = 0; w <= MAX_WORD; w++) {
    serialPrintBinary(w);
    //Serial.print(": ");
    write(w, w);
    //Serial.print(val, HEX);
    Serial.println();
    delayMicroseconds(writeCycleWait);
  }
}

void setup() {
  Serial.begin(9600);
  enablePins();
  Serial.println("Finished setup");
  digitalWrite(PIN_LED_GREEN, HIGH);
  readAll();
  //write(0b0000000000001001, 55);
  //write(0b0000000000001010, 0xAB);
  //write(0b0000000000010110, 0xAC);
  write();
  readAll();
  digitalWrite(PIN_LED_GREEN, LOW);
}

void loop() {

}
