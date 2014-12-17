/*
 The program was written to drive a 4 to seven segment displays using CD4543BE.
 http://www.jameco.com/Jameco/Products/ProdDS/819163.pdf
 THE SCREEN CIRCUIT IS CREATED IN NORMAL CLOSE LOGIC (that mean 1 is LOW 2 HIGH)
 
 And reading two rotary encoders 
 http://www.mouser.com/ds/2/54/ENC1J-26993.pdf
 
 as well a (switch) button  
 https://cdn-reichelt.de/documents/datenblatt/C200/11-SERIE_8000-D.pdf
 
 The sketch is reading the values from the encoders and the state of the button and then printed those values to serial monitor. 
 Also drive the displays in order to show a number from 0 to 9999, the number is the value  of the encoder. 
 
 For better understanding of rotary encoders read also 
 http://en.wikipedia.org/wiki/Gray_code
 
pin to 

36	4 display
39	3 display
38	2 display
37	1 display

32	A of CD4543BE
34	B of CD4543BE
35	C of CD4543BE
33	D of CD4543BE

3	 encoderA for frequency 
2	 encoderB for frequency

6	encoderA Volume
5	encoderB Volume

4	button

there are also 4 pin going to GND.
 		

 
Author: Zisoglou Sakis 13/11/2014
 */
const int led      =16;
int incomingByte   =0;
const int MaxChars =5;
char      strValue[MaxChars+1];
int       ind      =0;
int       number,number2   =0; //the number we print

//Declaration of Arduino pins used as CD4543BE inputs
const int A=32; //47;
const int B=34; //48; 
const int C=35; //49;
const int D=33; //46;

//Declaration of Arduino pins used as 7-segment displays inputs
const int screen1=37; //51; sel 1-4
const int screen2=38; //52; sel 1-3
const int screen3=39; //53; sel 1-2
const int screen4=36; //50; sel 1-1

//frequency 
int encoder0PinA     = 3;
int encoder0PinB     = 4;//2;
int encoder0Pos      = 10;
int mul              = 5;
int encoder0PinALast = LOW;
int n                = LOW;

//Volume
int encoderPinA     = 6; 
int encoderPinB     = 5;
int encoderPos      = 50;
int encoderPinALast = LOW;
int v               = LOW;

//button
const int buttonPin         = 7; //4; the number of the pushbutton pin
int       buttonState       = HIGH;  // variable for reading the pushbutton status
int       buttonlaststate   = HIGH;
int       buttonPushCounter = 1;
int       start             = 0;   //the state of the button that will print 

//flag for serial comunication
boolean   flag   =false; 

// will store last time was updated
long    previousMillis,previousMillis2,previousMillis3 = 0;  
// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long    interval       = 5;    // interval at which to do in (milliseconds)
int     count          = 0;     // initialize count variable

//On the screen for
int       Ontime       = 25*1000;
int       onv          =1000;

boolean overtime,printV       = false;

void setup() {
  pinMode (encoder0PinA,INPUT);
  pinMode (encoder0PinB,INPUT);
  pinMode (encoderPinA, INPUT);
  pinMode (encoderPinB, INPUT);
  pinMode (buttonPin,   INPUT);     

  pinMode(A, OUTPUT); //LSB
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(D, OUTPUT); //MSB

  pinMode(screen1, OUTPUT);
  pinMode(screen2, OUTPUT);
  pinMode(screen3, OUTPUT);
  pinMode(screen4, OUTPUT);

  pinMode(led, OUTPUT);
  digitalWrite(led,LOW);


  blank();
  Serial.begin(9600);

  Serial.print(encoder0Pos);
  Serial.print(",");
  Serial.print(encoderPos);
  Serial.print(",");
  Serial.println(start);

}

void loop() {
  // here is where you'd put code that needs to be running all the time.
  if (Serial.available() > 0) {
  // read the incoming byte:
  incomingByte = Serial.read();
  }
  printTheValue();    //Send to serial monitor
  button();           //read the button state
  number  =encoder();   //read the first  encoder for F and set the munber 
  number2=encoderV();         //read the second encoder for V 
  ledon();
  // number=receiveTheValue();
  // check if the difference between the current time and last time
  unsigned long currentMillis = millis();
  if((currentMillis - previousMillis > interval)) {
    // save the last time you high
    previousMillis = currentMillis; 
    

  
    if(start==1 && incomingByte!=49)
    {
		if((printV==false)  &&(currentMillis - previousMillis3 > onv ))
		{
	          printNumber(number);//Print the number of encoder to the displays
	        }
		else
		{
                   printNumber(number2);
		}

    }
    else
    {
      blank();
      //Serial.println("blank");
    }
  }//interval

if(printV==true)
 { 
   previousMillis3 = currentMillis;
   printV=false;
   
 }
  
  //Turn off the speaker after 16sec if nothing hapen 
  
//  if(flag==true)
//  {
//    previousMillis2 = currentMillis;
//  }
//  if(currentMillis - previousMillis2 > Ontime)
//  {
//    overtime=true;
//  }


}//loop

//Declaration for numbers of 7-segment displays 
//abcd to 0-9 (read the datasheet of CD4543BE)
void segmentWrite(int num1)
{
  if (num1 == 0) //write 0000
  {
    digitalWrite(A, HIGH);
    digitalWrite(B, HIGH);
    digitalWrite(C, HIGH);
    digitalWrite(D, HIGH);
  }

  if (num1 == 1) //write 0001
  {
    digitalWrite(A, LOW);
    digitalWrite(B, HIGH);
    digitalWrite(C, HIGH);
    digitalWrite(D, HIGH);
  }

  if (num1 == 2) //write 0010
  {
    digitalWrite(A, HIGH);
    digitalWrite(B, LOW);
    digitalWrite(C, HIGH);
    digitalWrite(D, HIGH);
  }

  if (num1 == 3) //write 0011
  {
    digitalWrite(A, LOW);
    digitalWrite(B, LOW);
    digitalWrite(C, HIGH);
    digitalWrite(D, HIGH);
  }

  if (num1 == 4) //write 0100
  {
    digitalWrite(A, HIGH);
    digitalWrite(B, HIGH);
    digitalWrite(C, LOW);
    digitalWrite(D, HIGH);
  }

  if (num1 == 5) //write 0101
  {
    digitalWrite(A, LOW);
    digitalWrite(B, HIGH);
    digitalWrite(C, LOW);
    digitalWrite(D, HIGH);
  }

  if (num1 == 6) //write 0110
  {
    digitalWrite(A, HIGH);
    digitalWrite(B, LOW);
    digitalWrite(C, LOW);
    digitalWrite(D, HIGH);
  }

  if (num1 == 7) //write 0111
  {
    digitalWrite(A, LOW);
    digitalWrite(B, LOW);
    digitalWrite(C, LOW);
    digitalWrite(D, HIGH);
  }

  if (num1 == 8) //write 1000
  {
    digitalWrite(A, HIGH);
    digitalWrite(B, HIGH);
    digitalWrite(C, HIGH);
    digitalWrite(D, LOW);
  }

  if (num1 == 9) //write 1001
  {
    digitalWrite(A, LOW);
    digitalWrite(B, HIGH);
    digitalWrite(C, HIGH);
    digitalWrite(D, LOW);
  }
}

void printNumber(int num)
{
  if(num<10)
  {
    //write the digit num to the screen dis display

    digitalWrite(screen4, LOW);
    segmentWrite(0);                digitalWrite(screen4, HIGH);
    digitalWrite(screen3, LOW);
    segmentWrite(0);                digitalWrite(screen3, HIGH);
    digitalWrite(screen2, LOW);
    segmentWrite(0);                digitalWrite(screen2, HIGH);
    digitalWrite(screen1, LOW);
    segmentWrite(num);              digitalWrite(screen1, HIGH);
  }
  else if(num<100)
  {
    digitalWrite(screen4, LOW);
    segmentWrite(0);               digitalWrite(screen4, HIGH);
    digitalWrite(screen3, LOW);
    segmentWrite(0);               digitalWrite(screen3, HIGH);
    digitalWrite(screen1, LOW);  
    segmentWrite(num%10);          digitalWrite(screen1, HIGH);
    digitalWrite(screen2, LOW);  
    segmentWrite(num/10);          digitalWrite(screen2, HIGH); 
  }
  else if(num<1000)
  {
    digitalWrite(screen4, LOW);
    segmentWrite(0);              digitalWrite(screen4, HIGH);
    digitalWrite(screen3, LOW);
    segmentWrite(num/100);        digitalWrite(screen3, HIGH);
    digitalWrite(screen1, LOW);
    segmentWrite(num%10);         digitalWrite(screen1, HIGH);
    digitalWrite(screen2, LOW);  
    segmentWrite((num%100)/10);   digitalWrite(screen2, HIGH);

  }
  else
  {
    digitalWrite(screen4, LOW); //1
    segmentWrite(num/1000);      digitalWrite(screen4, HIGH);
    digitalWrite(screen3, LOW);
    segmentWrite((num%1000)/100);digitalWrite(screen3, HIGH);
    digitalWrite(screen2, LOW);  //2
    segmentWrite((num%100)/10);  digitalWrite(screen2, HIGH);
    digitalWrite(screen1, LOW); //3 chip
    segmentWrite(num%10);        digitalWrite(screen1, HIGH);
  }
  clean();               //Clean the screens 
}//prinNumber

void clean(){
  digitalWrite(screen1, HIGH);
  digitalWrite(screen2, HIGH);  
  digitalWrite(screen3, HIGH);
  digitalWrite(screen4, HIGH);
}

int encoder() {
  n = digitalRead(encoder0PinA);
  if ((encoder0PinALast == LOW) && (n == HIGH)) {
    if (digitalRead(encoder0PinB) == LOW) {
      encoder0Pos-=mul;
    } 
    else {
      encoder0Pos+=mul;
    }
    flag=true;
    printV=false;
    
  }
  encoder0PinALast = n;
  if(start == 0 )
  { 
    encoder0Pos=10;
    encoderPos=50;
  }
  else if(overtime==true)
   {
    encoder0Pos=0; 
    encoderPos=50;
    start=0; 
    overtime=false;
    flag=true;
  }
  else if(encoder0Pos<=10 )
  {
    encoder0Pos=10;
  }
  else if(encoder0Pos>=600)
  {
    encoder0Pos=600;
  } 
  return encoder0Pos;
} 

int encoderV(){
  v = digitalRead(encoderPinA);
  if ((encoderPinALast == LOW) && (v == HIGH)) {
    if (digitalRead(encoderPinB) == LOW) {
      encoderPos--;
    } 
    else {
      encoderPos++;
    }
    //Serial.print(encoderPos);
    flag=true;
    printV=true;
  }
  if(encoderPos<1 ){
    encoderPos=0;
  }
  else if(encoderPos>100){
    encoderPos=100;
  }
  encoderPinALast = v;
  return encoderPos;  
} 

void printTheValue(){
  if(flag==true)
  {
    Serial.print(encoder0Pos);
    Serial.print(",");
    Serial.print(encoderPos);
    Serial.print(",");
    Serial.println(start);
    flag=false;
  }
}

int receiveTheValue(){

  if(Serial.available()>0)
  {
    char ch=Serial.read();
    if(ind < MaxChars && ch >= '0' && ch<='9')
    {
      strValue[ind++]=ch;
    }
    else
    {
      strValue[ind]=0;
      number=atoi(strValue);
      ind=0;
    }
  }
  return number;

}

void sw_button(){
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH ) 
  { 
    start=1;

    if(buttonlaststate == LOW || overtime==true )    
    {
      flag=true;
    }          
  } 
  if (buttonState == LOW )
  {
    start=0; 
    if(buttonlaststate == HIGH)    
    {
      flag=true;
    } 

  }
  buttonlaststate = buttonState;

}
void button()
{
buttonState = digitalRead(buttonPin);
  if (buttonState != buttonlaststate) 
  { 
    if (buttonState == LOW) 
    {    
      // turn  on:    
      start=1;     
    }
    else 
    {
      // turn LED off:
      start=0;
    }
   flag=true;
  }
   buttonlaststate = buttonState;
}//button

void button_as_sw(){
  buttonState = digitalRead(buttonPin);
  if (buttonState != buttonlaststate) 
  { 
    if(buttonState==LOW){
      buttonPushCounter++;
      encoder0Pos=10;
      flag=true;
  } 
  else
  {
  
  }
  
}
  buttonlaststate = buttonState;
  if(buttonPushCounter % 2==0){ 
     start=1;     
     }
     else{
     start=0;
     }
}

void ledon(){  
 if(start==1){
    digitalWrite(led,HIGH);
    }
 else{
    digitalWrite(led,LOW);
   
}
}

void blank()
{
    digitalWrite(screen4, LOW);
    digitalWrite(A, LOW);
    digitalWrite(B, LOW);
    digitalWrite(C, LOW);
    digitalWrite(D, LOW);
    digitalWrite(screen4, HIGH);
    
    digitalWrite(screen3, LOW);
    digitalWrite(A, LOW);
    digitalWrite(B, LOW);
    digitalWrite(C, LOW);
    digitalWrite(D, LOW);
    digitalWrite(screen3, HIGH);
    
    digitalWrite(screen2, LOW);
    digitalWrite(A, LOW);
    digitalWrite(B, LOW);
    digitalWrite(C, LOW);
    digitalWrite(D, LOW);
    digitalWrite(screen2, HIGH);
    
    digitalWrite(screen1, LOW);
    digitalWrite(A, LOW);
    digitalWrite(B, LOW);
    digitalWrite(C, LOW);
    digitalWrite(D, LOW);
    digitalWrite(screen1, HIGH);
}
