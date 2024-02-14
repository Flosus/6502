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
const int PIN_ADRESS[] = { 26, 25, 24, 23, 22, 21, 20, 19, 9, 8, 5, 7, 18, 10, 17 };
const int adressPinCount = 15;

bool READ_MODE = true;
bool WRITE_MODE = true;

// Number of words (bytes)
//int MAX_WORD = 32767;
int MAX_WORD = 64;

static const long int k_uTime_ReadPulse_uS = 10;

/*
Setup and Setup helper functions
*/

void SerialPrintBinary(int data) {
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

void set_chip_enabled(int val) {
  digitalWrite(PIN_CHIP_ENABLED, val);
}

void set_write_enabled(int val) {
  digitalWrite(PIN_WRITE_ENABLED, val);
}

void set_output_enabled(int val) {
  digitalWrite(PIN_OUTPUT_ENABLED, val);
}

void enable_pins() {
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_OUTPUT_ENABLED, OUTPUT);
  pinMode(PIN_WRITE_ENABLED, OUTPUT);
  pinMode(PIN_CHIP_ENABLED, OUTPUT);
  Serial.println("Setup adress Pins");
  for (int i = 0; i < adressPinCount; i++) {
    pinMode(PIN_ADRESS[i], OUTPUT);
  }
}

void set_adress(int a) {
  byte tmp = a;
  for (int i = 0; i < adressPinCount; i++) {
    bool enabled = tmp&1;
    tmp = (tmp >> 1);
    digitalWrite(PIN_ADRESS[i], enabled ? HIGH : LOW);
  }
}

void set_data(byte b) {
  byte tmp = b;
  for (int i = 0; i < dataPinCount; i++) {
    bool enabled = tmp&1;
    tmp = (tmp >> 1);
    digitalWrite(PIN_DATA[i], enabled ? HIGH : LOW);
  }
}

byte read_data() {
  byte data = 0;
  for (int i = dataPinCount - 1; i >= 0; i--) {
    int dataRaw = digitalRead(PIN_DATA[i]);
    data = (data << 1) + dataRaw;
  }
  return data;
}

byte read(int adress) {
  setDataPins(INPUT);
  set_adress(adress);
  set_chip_enabled(LOW);
  delayMicroseconds(k_uTime_ReadPulse_uS);
  byte data = read_data();
  set_chip_enabled(HIGH);
  return data;
}

void read_all() {
  digitalWrite(PIN_LED_RED, HIGH);
  set_write_enabled(HIGH);
  set_output_enabled(LOW);
  Serial.println("Starting to read all bytes");
  for (int w = 0; w <= MAX_WORD; w++) {
    SerialPrintBinary(w);
    Serial.print(": ");
    byte data = read(w);
    Serial.print(data, HEX);
    Serial.println();
  }
  Serial.println("Finished to read all bytes");
  digitalWrite(PIN_LED_RED, LOW);
}

void write(int adress, byte data) {
  setDataPins(OUTPUT);
  set_output_enabled(HIGH);
  set_adress(adress);
  set_chip_enabled(LOW);
  set_write_enabled(LOW);
  delayMicroseconds(1);
  set_data(data);
  delayMicroseconds(1);
  set_write_enabled(HIGH);
  set_chip_enabled(HIGH);
  set_output_enabled(LOW);
  delayMicroseconds(1);
}

void write() {
  set_write_enabled(HIGH);
  set_output_enabled(HIGH);
  //Serial.println("Writing 170");
  int val = 0;
  for (int w = 0; w <= MAX_WORD; w++) {
    SerialPrintBinary(w);
    //Serial.print(": ");
    write(w, w);
    //Serial.print(val, HEX);
    Serial.println();
    delayMicroseconds(k_uTime_ReadPulse_uS);
  }
}

void setup() {
  Serial.begin(9600);
  enable_pins();
  Serial.println("Finished setup");
  digitalWrite(PIN_LED_GREEN, HIGH);
  read_all();
  //write(0b0000000000001001, 55);
  //write(0b0000000000001010, 0xAB);
  //write(0b0000000000010110, 0xAC);
  write();
  read_all();
  digitalWrite(PIN_LED_GREEN, LOW);
}

void loop() {

}
