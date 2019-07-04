byte CD4543Pin[4] = {36, 53, 39, 37}; //ABCD Pins form chip CD4543BE
byte displayPins[3][4] =
{
  {43, 40, 42, 41}, //ScreenA
  {44, 47, 46, 45}, //ScreenB
  {48, 51, 50, 49}  //ScreenC
};

byte bcd[11][4] = //Binary Coded Decimal
{ //a  b  c  d
  { 1, 1, 1, 1}, // = 0
  { 0, 1, 1, 1}, // = 1
  { 1, 0, 1, 1}, // = 2
  { 0, 0, 1, 1}, // = 3
  { 1, 1, 0, 1}, // = 4
  { 0, 1, 0, 1}, // = 5
  { 1, 0, 0, 1}, // = 6
  { 0, 0, 0, 1}, // = 7
  { 1, 1, 1, 0}, // = 8
  { 0, 1, 1, 0}, // = 9
  { 0, 0, 0, 0}  // = 10 blank
};

// define encoder structure
typedef struct
{
  const int pinA;
  const int pinB;
  int pos;
  int prev_pos;
  int del; //delay
  int min;
  int max;
  bool limit;
} Encoder;

volatile Encoder Enc[2] =
{
  {22, 23, 0, 0, 0, 10, 999, false},
  {24, 25, 0, 0, 0, 0 , 100, false}
};

uint32_t last_millis;
uint16_t display_num;
char buffer[10];

void setup() {
  Serial.begin(115200);
  // attach interrupts
  pinMode(Enc[0].pinA, INPUT_PULLUP);
  pinMode(Enc[0].pinB, INPUT_PULLUP);
  pinMode(Enc[1].pinA, INPUT_PULLUP);
  pinMode(Enc[1].pinB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Enc[0].pinA), isrPinAEn1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(Enc[0].pinB), isrPinBEn1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(Enc[1].pinA), isrPinAEn2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(Enc[1].pinB), isrPinBEn2, CHANGE);
  set7SegPinMode(0);
  //display_num = 10000;
}

void loop() {

  if (millis() - last_millis > 15) {
    last_millis = millis();
    NumberToScreen(display_num, 0);
  }

  EncodersChanges();


  // check serial input commands
  char cmd[32]; char par[32];
  while (Serial.available() > 0) {
    if (ReadSerialCommand(cmd, par, 32)) {
      if (strcmp(cmd, "freq_max") == 0)  {
        Enc[0].max = atoi(par);
      }
      if (strcmp(cmd, "freq_min") == 0)  {
        Enc[0].min = atoi(par);
      }
      if (strcmp(cmd, "freq_set") == 0)  {
        Enc[0].pos = atoi(par);
      }
      if (strcmp(cmd, "vol_max") == 0)   {
        Enc[1].max = atoi(par);
      }
      if (strcmp(cmd, "vol_min") == 0)   {
        Enc[1].min = atoi(par);
      }
      if (strcmp(cmd, "vol_set") == 0)   {
        Enc[1].pos = atoi(par);
      }
      if (strcmp(cmd, "off") == 0) {
        display_num = 10000;
      }
      if (strcmp(cmd, "on") == 0) {
        display_num = Enc[0].pos;
      }
    }
  }
}

// ****************************************************************************
// UdooSend - Send serial message to Udoo using printf formatting.
// ****************************************************************************
void UdooSend(const char *format, ...) {
  char buffer[2048];
  va_list fparams;

  if (format) {
    va_start(fparams, format);
    vsnprintf(buffer, 2047, format, fparams);
    va_end(fparams);
  } else {
    //strcpy(buffer, "");
    memset(buffer, 0, 2048); //clear buf
  }
  Serial.write(buffer);
}


//----------------------------------------------------------------------
// Check encoders changes and send via serial
//----------------------------------------------------------------------
void EncodersChanges()
{
  if (Enc[0].prev_pos != Enc[0].pos)
  {
    UdooSend("F,%d,%d\n", Enc[0].pos, Enc[1].pos);
    display_num = Enc[0].pos;
    Enc[0].prev_pos = Enc[0].pos;
  } else {
    if (Enc[0].limit) {
      UdooSend("F,%d,%d\n", Enc[0].pos, Enc[1].pos);
      display_num = Enc[0].pos;
      Enc[0].limit = false;
    }
  }

  if (Enc[1].prev_pos != Enc[1].pos)
  {
    UdooSend("V,%d,%d\n", Enc[0].pos, Enc[1].pos);
    display_num = Enc[1].pos;
    Enc[1].prev_pos = Enc[1].pos;
  } else {
    if (Enc[1].limit) {
      UdooSend("F,%d,%d\n", Enc[0].pos, Enc[1].pos);
      display_num = Enc[1].pos;
      Enc[1].limit = false;
    }
  }
}


//----------------------------------------------------------------------
// 7-Seg screen, Write a number
//----------------------------------------------------------------------
void NumberToScreen(uint16_t num, uint8_t scr) {//scr can be 0,1,2



  if (num < 10) {
    digitalWrite(displayPins[scr][0], LOW); digitWrite(0);   digitalWrite(displayPins[scr][0], HIGH);
    digitalWrite(displayPins[scr][1], LOW); digitWrite(0);   digitalWrite(displayPins[scr][1], HIGH);
    digitalWrite(displayPins[scr][2], LOW); digitWrite(0);   digitalWrite(displayPins[scr][2], HIGH);
    digitalWrite(displayPins[scr][3], LOW); digitWrite(num); digitalWrite(displayPins[scr][3], HIGH);
  }
  else if (num < 100) {
    digitalWrite(displayPins[scr][0], LOW); digitWrite(0);        digitalWrite(displayPins[scr][0], HIGH);
    digitalWrite(displayPins[scr][1], LOW); digitWrite(0);        digitalWrite(displayPins[scr][1], HIGH);
    digitalWrite(displayPins[scr][3], LOW); digitWrite(num % 10); digitalWrite(displayPins[scr][3], HIGH);
    digitalWrite(displayPins[scr][2], LOW); digitWrite(num / 10); digitalWrite(displayPins[scr][2], HIGH);
  }
  else if (num < 1000) {
    digitalWrite(displayPins[scr][0], LOW); digitWrite(0);                digitalWrite(displayPins[scr][0], HIGH);
    digitalWrite(displayPins[scr][1], LOW); digitWrite(num / 100);        digitalWrite(displayPins[scr][1], HIGH);
    digitalWrite(displayPins[scr][3], LOW); digitWrite(num % 10);         digitalWrite(displayPins[scr][3], HIGH);
    digitalWrite(displayPins[scr][2], LOW); digitWrite((num % 100) / 10); digitalWrite(displayPins[scr][2], HIGH);
  }
  else if (num < 10000) {
    digitalWrite(displayPins[scr][0], LOW); digitWrite(num / 1000);         digitalWrite(displayPins[scr][0], HIGH);
    digitalWrite(displayPins[scr][1], LOW); digitWrite((num % 1000) / 100); digitalWrite(displayPins[scr][1], HIGH);
    digitalWrite(displayPins[scr][2], LOW); digitWrite((num % 100) / 10);   digitalWrite(displayPins[scr][2], HIGH);
    digitalWrite(displayPins[scr][3], LOW); digitWrite(num % 10);           digitalWrite(displayPins[scr][3], HIGH);
  }
  else {
    digitalWrite(displayPins[scr][0], LOW); digitWrite(10); digitalWrite(displayPins[scr][0], HIGH);
    digitalWrite(displayPins[scr][1], LOW); digitWrite(10); digitalWrite(displayPins[scr][1], HIGH);
    digitalWrite(displayPins[scr][2], LOW); digitWrite(10); digitalWrite(displayPins[scr][2], HIGH);
    digitalWrite(displayPins[scr][3], LOW); digitWrite(10); digitalWrite(displayPins[scr][3], HIGH);
  }

}
//----------------------------------------------------------------------
// 7-Seg display, Write a digit
//----------------------------------------------------------------------
void digitWrite(uint8_t digit)
{
  //write the number for sevenSegment from 2D array
  for (byte i = 0; i < 4; ++i)
  {
    digitalWrite(CD4543Pin[i], bcd[digit][i]);
  }
}

//----------------------------------------------------------------------
// 7-seg setup pinMode
//----------------------------------------------------------------------
void set7SegPinMode(int screen) {
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(CD4543Pin[i], OUTPUT);
    pinMode(displayPins[screen][i], OUTPUT);
  }
}

//----------------------------------------------------------------------
// ReadSerialCommand
//----------------------------------------------------------------------
bool ReadSerialCommand(char* cmd, char* par, int maxIndex) {
  bool inPar = false;
  int index = 0;
  cmd[0] = '\0';
  par[0] = '\0';
  while (true) {
    if (Serial.available() <= 0) {
      delay(20);
      if (Serial.available() <= 0) {
        break;
      }
    }
    char sin = Serial.read();
    if (sin == '\n') {
      if (inPar) {
        par[index] = '\0';
      }
      else {
        cmd[index] = '\0';
      }
      return true;
    }
    if (sin == ' ' || sin == '\t') {
      if (!inPar) {
        cmd[index] = '\0';
        inPar = true;
        index = 0;
        continue;
      }
      else {
        if (index < maxIndex) {
          par[index++] = sin;
        }
      }
    }
    if (!inPar) {
      if (index < maxIndex) {
        cmd[index++] = sin;
      }
    }
    else {
      if (index < maxIndex) {
        par[index++] = sin;
      }
    }
  }
  return false;
}

//----------------------------------------------------------------------
// Encoders
//----------------------------------------------------------------------

// auxiliary debouncing function
void debounce(int del)
{
  int k;
  for (k = 0; k < del * 15; k++)
  {
    /* can't use delay in the ISR so need to waste some time
       perfoming operations, this uses roughly 0.1ms */
    k = k + 0.0 + 0.0 - 0.0 + 3.0 - 3.0;
  }
}

// Interrupt Service Routine on pin A for Encoder 1
void isrPinAEn1()
{
  uint8_t drB = digitalRead(Enc[0].pinB);
  /* possibly wait before reading pin A, then read it        */
  debounce(Enc[0].del);
  uint8_t drA = digitalRead(Enc[0].pinA);
  if (drA == HIGH)
  {
    if (drB == LOW)
      Enc[0].pos++;
    else
      Enc[0].pos--;
  }
  else
  {
    if (drB == HIGH)
      Enc[0].pos++;
    else
      Enc[0].pos--;
  }
  if (Enc[0].pos < Enc[0].min) { Enc[0].pos = Enc[0].min; Enc[0].limit = true; }
  if (Enc[0].pos > Enc[0].max) { Enc[0].pos = Enc[0].max; Enc[0].limit = true; }
}

// Interrupt Service Routine on pin B for Encoder 1
void isrPinBEn1()
{
  uint8_t drA = digitalRead(Enc[0].pinA);
  /* possibly wait before reading pin B, then read it        */
  debounce(Enc[0].del);
  uint8_t drB = digitalRead(Enc[0].pinB);
  if (drB == HIGH)
  {
    if (drA == HIGH)
      Enc[0].pos++;
    else
      Enc[0].pos--;
  }
  else
  {
    if (drA == LOW)
      Enc[0].pos++;
    else
      Enc[0].pos--;
  }
  if (Enc[0].pos < Enc[0].min) { Enc[0].pos = Enc[0].min; Enc[0].limit = true; }
  if (Enc[0].pos > Enc[0].max) { Enc[0].pos = Enc[0].max; Enc[0].limit = true; }
}

// Interrupt Service Routine on pin A for Encoder 2
void isrPinAEn2()
{
  uint8_t drB = digitalRead(Enc[1].pinB);
  /* possibly wait before reading pin A, then read it        */
  debounce(Enc[1].del);
  uint8_t drA = digitalRead(Enc[1].pinA);
  if (drA == HIGH)
  {
    if (drB == LOW)
      Enc[1].pos++;
    else
      Enc[1].pos--;
  }
  else
  {
    if (drB == HIGH)
      Enc[1].pos++;
    else
      Enc[1].pos--;
  }
  if (Enc[1].pos < Enc[1].min) { Enc[1].pos = Enc[1].min; Enc[1].limit = true; }
  if (Enc[1].pos > Enc[1].max) { Enc[1].pos = Enc[1].max; Enc[1].limit = true; }
}

// Interrupt Service Routine on pin B for Encoder 2
void isrPinBEn2()
{
  uint8_t drA = digitalRead(Enc[1].pinA);
  /* possibly wait before reading pin B, then read it        */
  debounce(Enc[1].del);
  uint8_t drB = digitalRead(Enc[1].pinB);

  if (drB == HIGH)
  {
    if (drA == HIGH)
      Enc[1].pos++;
    else
      Enc[1].pos--;
  }
  else
  {
    if (drA == LOW)
      Enc[1].pos++;
    else
      Enc[1].pos--;
  }
  if (Enc[1].pos < Enc[1].min) { Enc[1].pos = Enc[1].min; Enc[1].limit = true; }
  if (Enc[1].pos > Enc[1].max) { Enc[1].pos = Enc[1].max; Enc[1].limit = true; }
}
