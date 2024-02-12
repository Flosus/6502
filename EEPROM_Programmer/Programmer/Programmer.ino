/*
Define constants
*/
const int PIN_LED_RED = 13;
const int PIN_LED_GREEN = 7;

const int PIN_OUTPUT_ENABLED = 22;
const int PIN_WRITE_ENABLED = 12;
const int PIN_CHIP_ENABLED = 26;

const int PIN_DATA[] = { 19, 20, 21, 36, 34, 32, 30, 28 };
const int dataPinCount = 8;
const int PIN_ADRESS[] = { 18, 17, 16, 15, 14, 2, 3, 4, 10, 9, 24, 8, 5, 11, 6 };
const int adressPinCount = 15;

bool READ_MODE = true;

// Number of words (bytes)
//int MAX_WORD = 32767;
int MAX_WORD = 255;

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

void enable_and_set_val(int pin,  int mode, int val) {
  pinMode(pin, mode);
  digitalWrite(pin, val);
  Serial.print("Enabling Pin: ");
  Serial.println(pin);
}

void enable_data_pins(bool readMode) {
  Serial.println("Setup data Pins");
  for (int i = 0; i < dataPinCount; i++) {
    enable_and_set_val(PIN_DATA[i], readMode ? INPUT : OUTPUT, LOW);
  }
}

void enable_pins() {
  Serial.println("Setup special Pins");
  enable_and_set_val(PIN_LED_RED, OUTPUT, LOW);
  enable_and_set_val(PIN_LED_GREEN, OUTPUT, LOW);
  enable_and_set_val(PIN_OUTPUT_ENABLED, OUTPUT, HIGH);
  enable_and_set_val(PIN_WRITE_ENABLED, OUTPUT, HIGH);
  enable_and_set_val(PIN_CHIP_ENABLED, OUTPUT, HIGH);
  Serial.println("Setup adress Pins");
  for (int i = 0; i < adressPinCount; i++) {
    enable_and_set_val(PIN_ADRESS[i], OUTPUT, LOW);
  }
}

void set_adress(int adress) {

  digitalWrite(PIN_OUTPUT_ENABLED, true);
}

void set_data(byte data) {

}

byte read_data() {
  return 0;
}

byte read(int adress) {
  set_adress(adress);
  byte data = read_data();
  return data;
}

void read_all() {
  digitalWrite(PIN_LED_RED, HIGH);
  enable_data_pins(READ_MODE);
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


void setup() {
  Serial.begin(9600);
  enable_pins();
  Serial.println("Finished setup");
  digitalWrite(PIN_LED_GREEN, HIGH);
  read_all();
}

void loop() {

}
