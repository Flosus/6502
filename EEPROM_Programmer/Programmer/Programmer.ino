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

bool WRITE_MODE = true;

// Number of words (bytes)
//int MAX_WORD = 32767;
int MAX_WORD = 8;

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

void set_adress(int a) {
  digitalWrite(PIN_ADRESS[0],  (a&1)?HIGH:LOW    );
  digitalWrite(PIN_ADRESS[1],  (a&2)?HIGH:LOW    );
  digitalWrite(PIN_ADRESS[2],  (a&4)?HIGH:LOW    );
  digitalWrite(PIN_ADRESS[3],  (a&8)?HIGH:LOW    );
  digitalWrite(PIN_ADRESS[4],  (a&16)?HIGH:LOW   );
  digitalWrite(PIN_ADRESS[5],  (a&32)?HIGH:LOW   );
  digitalWrite(PIN_ADRESS[6],  (a&64)?HIGH:LOW   );
  digitalWrite(PIN_ADRESS[7],  (a&128)?HIGH:LOW  );
  digitalWrite(PIN_ADRESS[8],  (a&256)?HIGH:LOW  );
  digitalWrite(PIN_ADRESS[9],  (a&512)?HIGH:LOW  );
  digitalWrite(PIN_ADRESS[10], (a&1024)?HIGH:LOW );
  digitalWrite(PIN_ADRESS[11], (a&2048)?HIGH:LOW );
  digitalWrite(PIN_ADRESS[12], (a&4096)?HIGH:LOW );
  digitalWrite(PIN_ADRESS[13], (a&8192)?HIGH:LOW );
  digitalWrite(PIN_ADRESS[14], (a&16384)?HIGH:LOW);
}

void set_data(byte b) {
  digitalWrite(PIN_DATA[0], (b&1)?HIGH:LOW  );
  digitalWrite(PIN_DATA[1], (b&2)?HIGH:LOW  );
  digitalWrite(PIN_DATA[2], (b&4)?HIGH:LOW  );
  digitalWrite(PIN_DATA[3], (b&8)?HIGH:LOW  );
  digitalWrite(PIN_DATA[4], (b&16)?HIGH:LOW );
  digitalWrite(PIN_DATA[5], (b&32)?HIGH:LOW );
  digitalWrite(PIN_DATA[6], (b&64)?HIGH:LOW );
  digitalWrite(PIN_DATA[7], (b&128)?HIGH:LOW);
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

byte read_data() {
  byte data = 0;
  Serial.print("BIN: ");
  for (int i = 0; i < dataPinCount; i++) {
    int dataRaw = digitalRead(PIN_DATA[i]);
    Serial.print(dataRaw);
    data = (data << 1) + dataRaw;
  }
  Serial.print(" HEX: ");
  return data;
}

byte read(int adress) {
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
  enable_data_pins(READ_MODE);
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
  set_adress(adress);
  set_data(data);
  set_chip_enabled(LOW);
  set_write_enabled(LOW);
  delayMicroseconds(k_uTime_ReadPulse_uS);
  set_write_enabled(HIGH);
  set_chip_enabled(HIGH);
}

void write() {
  set_write_enabled(HIGH);
  set_output_enabled(HIGH);
  enable_data_pins(WRITE_MODE);
  Serial.println("Writing 170");
  int val = 170;
  for (int w = 0; w <= MAX_WORD; w++) {
    SerialPrintBinary(w);
    Serial.print(": ");
    write(w, val);
    Serial.print(val, HEX);
    Serial.println();
  }
}

void setup() {
  Serial.begin(9600);
  enable_pins();
  Serial.println("Finished setup");
  digitalWrite(PIN_LED_GREEN, HIGH);
  read_all();
  write();
  read_all();
}

void loop() {

}
