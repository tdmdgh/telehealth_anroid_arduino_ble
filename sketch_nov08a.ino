//11.06
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

//OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET  -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//variables for temperature sensor
int TempPin = A3;
int temp;
bool temp1;

//variables for light sensor
int bluePin = 9;
int greenPin = 10;
int redPin = 11;
int lightsensor = A2;
int light;

//variables for warning signal
bool warning = true;
//bool warning = false;
int warningincoming;

//variables for graph
#define UpperThreshold 560
#define LowerThreshold 530
int a;
int b;
int lasta;
int lastb=0;
int LastTime=0;
int ThisTime;
bool BPMTiming=false;
bool BeatComplete=false;
int bpm;

//timer
unsigned long prevtime = 0;
unsigned long starttime;
long timer;

//bitmap
const unsigned char bitmap [] PROGMEM=
{
  0xff, 0xc0, 0xc9, 0xc0, 0x80, 0xc0, 0x06, 0x40, 0x06, 0x40, 0x00, 0x40, 0x80, 0xc0, 0xc1, 0xc0, 
  0xe3, 0xc0, 0xf7, 0xc0
};
const int epd_bitmap_allArray_LEN = 1;
const unsigned char* epd_bitmap_allArray[1] = {
  bitmap
};


// Wi-Fi & ThingSpeak
#include "WiFiEsp.h"
#include "ThingSpeak.h" 

char ssid[] = "Jecspot";   
char pass[] = "dmschd12";        
WiFiEspClient  client;

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
SoftwareSerial Serial1(1, 2); // RX, TX
#define ESP_BAUDRATE  19200
#else
#define ESP_BAUDRATE  115200
#endif

unsigned long myChannelNumber = 1887337;
const char * myWriteAPIKey = "92VNJ1EV1CFADLYE";

int blueTx = 12;
int blueRx = 13;
SoftwareSerial mySerial(blueTx, blueRx);

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  mySerial.begin(9600);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  //light sensor
  pinMode(bluePin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(lightsensor, INPUT);
  display.clearDisplay();
  warning = true;//default

  while(!Serial){
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }  

  setEspBaudRate(ESP_BAUDRATE);
  //Serial1.begin(9600);

  // initialize ESP module
  WiFi.init(&Serial1);

  ThingSpeak.begin(client);  // Initialize ThingSpeak
  
}
void loop() {
  if(warning){
    digitalWrite(bluePin, LOW);
    digitalWrite(greenPin, LOW);
    temp1 = tempcheck();
    if(temp1) {
      warning = warn1min();
    }
    if((!temp1)&&(warning)){
      display.clearDisplay();
      warnnormal ();
      temp1 = tempcheck();
      warning = false;
      if(temp1){
        warning = true;
       }
    }
   }
  if(!warning){
    display.clearDisplay();
    nowarn();
    warning = true;
  }

  if(WiFi.status() != WL_CONNECTED) {
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);
    }
  }

  int t_bpm = measurebpm();
  int t_temp = tempdetect();
  if ((t_temp>30 && t_temp<35) || (t_temp>40 && t_temp<45)) {
    ThingSpeak.writeField(myChannelNumber, 1, t_bpm, myWriteAPIKey);
    Serial.println("Temperature sent to tempearature channel");
    delay(1000);
  }
  if ((t_bpm >0 && t_bpm<50) || (t_bpm>120 && t_bpm<300)) {
    ThingSpeak.writeField(myChannelNumber, 2, t_temp, myWriteAPIKey);
    Serial.println("Pulse sent to pulse channel");
    delay(1000);
  }
  
}//end of loop

//이상신호보내주기
bool tempcheck() {
    bool la;
    bpm = measurebpm ();
    temp = tempdetect ();
    if((temp>30 && temp<35)|| (temp>40 && temp<45)){
      la = true;
      return la;
    }
    if((bpm>0 && bpm<50)|| (bpm>120 && bpm<300)){
      la = true;
      return la;
    }
    la = false;
    return la;
}
//drawbitmap
void onBeatDetected(){
  display.drawBitmap(0, 0, bitmap, 10, 10, WHITE);
  display.display();
  return;
}

//measurebpm
int measurebpm(){
  int bp;
  int value=analogRead(0);
  bp=80-(value/16);
  return bp;
}

//measure temp
int tempdetect (){
  int t;
  t = analogRead(TempPin);
  t = t+1;
  return t;
}

void changeLED (){
  light = analogRead(lightsensor);
  if(light>50){
    analogWrite(redPin, light);
    analogWrite(greenPin, light);
    digitalWrite(bluePin, LOW);
    delay(100);
  }
  else if(light<50 || light == 50){
    analogWrite(bluePin, light);
    analogWrite(redPin, light);
    digitalWrite(greenPin, LOW);
    delay(100);
  }
  return;
}

void warningled () {
    digitalWrite(redPin, HIGH);
    delay(10);
    digitalWrite(redPin, LOW);
    return;
}
long stoptimer (){
  unsigned long ss = 0;
  return ss;
}

bool warn1min (){
  extern volatile unsigned long timer0_millis;
  warning = true;
  display.clearDisplay();
  temp1 = tempcheck();
    if(temp1){
      mySerial.write("W");  
    }
    else if(!temp1){
      mySerial.write("N");
    return true;
    }
  while(warning){
    display.setCursor(17,20);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println("!Warning!");
    display.display();
    display.setCursor(20, 30);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println("Contact Doctor");   
    display.display();

    while(!(timer >= 60000)){
      starttime = millis();
      timer = starttime - prevtime;
      bpm = measurebpm ();
      temp = tempdetect ();
      warningled ();
      display.setCursor(0,40);
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.print("BPM: ");
      display.println(bpm);
      display.display();
      display.setCursor(0,50);
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.print("TEMP: ");
      display.println(temp);
      display.display();
      display.setCursor(50,50);
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.println(" C");
      display.display(); 
      //fill with black for update
      display.fillRect(23,40,19,10,BLACK);
      display.display();
      display.fillRect(30,50,24,10,BLACK);
      display.display();  
      //keep communicating with bluetooth
      //(int)temp -> (char[]) temp
      char t_str1_w1 [5];
      itoa(temp,t_str1_w1,10);
      char t_str2_w1 [6];
      t_str2_w1[0] = 'T';
      int t_index_w1 = 1;
      while(t_str1_w1[t_index_w1-1] !='\0'){
        t_str2_w1[t_index_w1] =t_str1_w1[t_index_w1-1];
        t_index_w1++;
      }
      mySerial.write(t_str2_w1);
      delay(10);

      //(int)bpm -> (char[]) bpm
      char p_str1_w1 [5];
      itoa(bpm,p_str1_w1,10);
      char p_str2_w1 [6];
      p_str2_w1[0] = 'P';
      int p_index_w1 = 1;
      while(p_str1_w1[p_index_w1-1] !='\0'){
        p_str2_w1[p_index_w1] =p_str1_w1[p_index_w1-1];
        p_index_w1++;
      }
      mySerial.write(p_str2_w1);
      delay(10);

      warning = warningdetectt();
      if(!warning){
        return false;
      }
    }//while timer
    timer = stoptimer();
    noInterrupts ();
    timer0_millis = 0;
    interrupts ();
   }//while warning
   
}

//warning = on, condition = normal
void warnnormal (){
  warning = true;
  int j =0;
  int a = 60;
  b= measurebpm();
  lastb=b;
  
  while(warning){
    temp = tempdetect();
    bpm = measurebpm();
    changeLED();
    if(a>128){
      display.clearDisplay();
      a=60;
      lasta=a;
      int j = 0;
      lastb=34;
    }
    if(j==0){
      a=60;
      lasta=60;
      lastb=34;
    }
    int value = analogRead(0);
    b = 70-(value/16);
    display.drawLine(lasta,lastb,a,b,WHITE);
    display.display(); 
    onBeatDetected(); 
    lastb=b;
    lasta=a; 
    display.setCursor(0,40);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.print("BPM: ");
    display.println(bpm);
    display.display();
    display.setCursor(0,50);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.print("TEMP: ");
    display.println(temp);
    display.display();
    display.setCursor(48,50);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println(" C");
    display.display();
      
    display.setCursor(0,20);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println("Warning: ");
    display.display();
    display.setCursor(50,20);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println("O");
    display.display();
    
          
    //fill with black for update
    display.fillRect(23,40,19,10,BLACK); //bpm
    display.display();
    display.fillRect(30,50,24,10,BLACK); //temp
    display.display();
    display.fillRect(0,0,10,10,BLACK); //bitmap
    display.display();

    //(int)temp -> (char[]) temp
    char t_str1_w [5];
    itoa(temp,t_str1_w,10);
    char t_str2_w [6];
    t_str2_w[0] = 'T';
    int t_index_w = 1;
    while(t_str1_w[t_index_w-1] !='\0'){
      t_str2_w[t_index_w] =t_str1_w[t_index_w-1];
      t_index_w++;
    }
    mySerial.write(t_str2_w);
    delay(10);

    //(int)bpm -> (char[]) bpm
    char p_str1_w [5];
    itoa(bpm,p_str1_w,10);
    char p_str2_w [6];
    p_str2_w[0] = 'P';
    int p_index_w = 1;
    while(p_str1_w[p_index_w-1] !='\0'){
      p_str2_w[p_index_w] =p_str1_w[p_index_w-1];
      p_index_w++;
    }
    mySerial.write(p_str2_w);
    delay(10);

    
    a++;
    j++;
    warning = warningdetectt(); 
    if(!warning){
        return;
    } 
      temp1 = tempcheck();
      if(temp1){
        return;
      }
      
  }//while loop
  
}
//warning = off
void nowarn (){
  warning = false;
  int j =0;
  int a = 60;
  b= measurebpm();
  lastb=b;
  
  while(!warning){
    temp = tempdetect();
    bpm = measurebpm();
    changeLED();
    if(a>128){
      display.clearDisplay();
      a=60;
      lasta=a;
      lastb=34;
      int j = 0;
    }
    if(j==0){
      a=60;
      lasta=60;
      lastb=34;
    }
    int value = analogRead(0);
    b = 70-(value/16);
    display.drawLine(lasta,lastb,a,b,WHITE);
    display.display(); 
    onBeatDetected(); 
    lastb=b;
    lasta=a; 
    display.setCursor(0,40);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.print("BPM: ");
    display.println(bpm);
    display.display();
    display.setCursor(0,50);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.print("TEMP: ");
    display.println(temp);
    display.display();
    display.setCursor(48,50);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println(" C");
    display.display();
      
    display.setCursor(0,20);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println("Warning: ");
    display.display();
    display.setCursor(50,20);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println("X");
    display.display();
    
          
    //fill with black for update
    display.fillRect(23,40,19,10,BLACK); //bpm
    display.display();
    display.fillRect(30,50,24,10,BLACK); //temp
    display.display();

    display.fillRect(0,0,10,10,BLACK); //bitmap
    display.display();


    //(int)temp -> (char[]) temp
    char t_str1 [5];
    itoa(temp,t_str1,10);
    char t_str2 [6];
    t_str2[0] = 'T';
    int t_index = 1;
    while(t_str1[t_index-1] !='\0'){
      t_str2[t_index] =t_str1[t_index-1];
      t_index++;
    }
    mySerial.write(t_str2);
    delay(10);

    //(int)bpm -> (char[]) bpm
    char p_str1 [5];
    itoa(bpm,p_str1,10);
    char p_str2 [6];
    p_str2[0] = 'P';
    int p_index = 1;
    while(p_str1[p_index-1] !='\0'){
      p_str2[p_index] =p_str1[p_index-1];
      p_index++;
    }
    mySerial.write(p_str2);
    delay(10);
    
    a++;
    j++;
    warning = warningdetectf(); 
    if(warning){
     return;
    }
  }//while loop
}
char* inttostring(int num){
    int length = snprintf(NULL, 0, "%d", num);
    char* temptoString = malloc(length+1);
    snprintf(temptoString, length+1, "%d", num);
    char* T = "T";
    strcat(T, temptoString);
    return T;
}
//warning detect when warning = on
bool warningdetectt () {
  bool result1;
  long test1=0;
  long compare1 = 129;
  long compare2 = 130;
//  if(mySerial.available(){
//    mySerial.read();
//  }
  if(mySerial.available ()){
    long test1 = mySerial.read();
//    long test1 = Serial.parseInt();
    if(test1 == compare1){
    result1 = true;
    return result1;
  }
  else if(test1 == compare2){
    result1 = false;
    return result1;
  }
 }
  result1 = true;
  return result1;
  
}
//warning detect when warning = off
bool warningdetectf () {
  bool result2;
  long test2=0;
  long compare1 = 129;
  long compare2 = 130;
  if(mySerial.available ()){
    test2 = mySerial.read();
    Serial.println(test2);
    if(test2 == compare1){
      result2 = true;
      return result2;
    }
    else if(test2 == compare2){
      result2 = false;
      return result2;
    } 
   }
  result2 = false;
  return result2;
}

// setting esp baud rate for Wi-Fi module
void setEspBaudRate(unsigned long baudrate){
  long rates[6] = {115200,74880,57600,38400,19200,9600};

  //Serial.print("Setting ESP8266 baudrate to ");
  //Serial.print(baudrate);
  //Serial.println("...");

  for(int i = 0; i < 6; i++){
    Serial1.begin(rates[i]);
    //delay(100);
    //Serial1.print("AT+UART_DEF=");
    //Serial1.print(baudrate);
    //Serial1.print(",8,1,0,0\r\n");
    //delay(100);  
  }
    
  Serial1.begin(baudrate);
}

//void tostring(char str[], int num)
//{
//    int i, rem, len = 0, n;
// 
//    n = num;
//    while (n != 0)
//    {
//        len++;
//        n /= 10;
//    }
//    for (i = 0; i < len; i++)
//    {
//        rem = num % 10;
//        num = num / 10;
//        str[len - (i + 1)] = rem + '0';
//    }
//    str[len] = '\0';
//}
/*
char* toArray(int number)
{
    int n = log10(number) + 1;
    int i;
    char numberArray[] = calloc(n, sizeof(char));
    for (i = n-1; i >= 0; --i, number /= 10)
    {
        numberArray[i] = (number % 10) + '0';
    }
    return numberArray;
}*/
