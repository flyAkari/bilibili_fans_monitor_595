/**********************************************************************
 * 项目：bilibili粉丝数监视器(74595版)开发中
 * 硬件：适用于NodeMCU ESP8266 + 2片74HC595 + 8位共阳数码管
 * 功能：连接WiFi后获取指定用户的哔哩哔哩实时粉丝数并在8位数码管上居中显示
 * 作者：flyAkari 会飞的阿卡林 bilibili UID:751219
 * 日期：2018/11/21
 **********************************************************************/
 //本代码未经硬件测试
 //硬件连接详情见电路图：
 //显示模块 --- ESP8266
 //  VCC   --- 3V(3.3V)
 //  GND   --- G (GND)
 //  DIO   --- (GPIO4)
 //  SCLK  --- (GPIO5)
 //  RCLK  --- (GPIO3)

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <Ticker.h>
//---------------修改此处""内的信息--------------------
const char* ssid     = "Akari";       //WiFi名
const char* password = "uid751219";   //WiFi密码
String biliuid       = "751219";      //bilibili UID
//----------------------------------------------------
DynamicJsonDocument jsonBuffer(200);
WiFiClient client;
Ticker blinker;

#define mySCLK 2         //设置数据输入时钟线
#define myRCLK 3         //设置输出存储器锁存时钟线
#define myDIO 4          //设置串行数据输入

enum ALIGN{              //数显对齐方式
    LEFT, CENTER, RIGHT
};
unsigned char LED_CA[] = {       //共阳七段数码管码表
 //  0    1    2    3    4    5    6    7    8    9    
  0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,
 //  A    b    C    d    E    F    -    P   []    H
  0x88,0x83,0xC6,0xA1,0x86,0x8E,0xBF,0x8C,0xFF,0x89,
 //  L    o    c    y    u    q    r
  0xC7,0xA3,0xA7,0x91,0xE3,0x98,0xAF
  /*A=10,b=11,C=12,d=13,E=14,F=15,-=16,P=17,blank=18,H=19,L=20,o=21,c=22,y=23,u=24,q=25,r=26*/
};
unsigned char LED[8];       //LED显示内容缓存
int SCLK = mySCLK;   //SH_CP, 上升沿时数据移位
int RCLK = myRCLK;   //ST_CP, 上升沿时数据锁存
int DIO = myDIO;     //DS, 串行数据输入端

void DisplayError()
{
  LED[0] = 18;LED[1] = 18;LED[2] = 14;LED[3] = 26;
  LED[4] = 26;LED[5] = 21;LED[6] = 26;LED[7] = 18;
}

void EditNumber(int number, ALIGN align)
{
  int flag = 0;
  if(number>99999999||number<-9999999){
    DisplayError();
    return;
  }
  if(number<0){
    flag = 1;
    number = abs(number);
  }
  int x = 1;
  int tmp = number;
  for (x = 1; tmp /= 10; x++);
  if(align == LEFT)
  {
    if(flag == 0){
      for(int i = 7; i > x - 1; i--)
      {
          LED[i] = 18;
     }
     for(int i = x - 1; i >= 0; i--)
      {
        int character = number % 10;
        LED[i] = character;
        number /= 10;
      }
    }
    else{
      for(int i = 7; i > x; i--)
      {
        LED[i] = 18;
      }
      for(int i = x; i >= 1; i--)
      {
        int character = number % 10;
        LED[i] = character;
        number /= 10;
      }
      LED[0] = 16;
    }
  }
  if(align == CENTER)
  {
    for(int i = 7; i >= 0; i--)
    {
      if(i < (4 - x / 2) || i >= ((x + 9) / 2))
      {
        LED[i] = 18;
      }
      else{
        int character = number % 10;
        LED[i] = character;
        number /= 10;
      }
    }
    if(flag == 1) LED[3-x/2] = 16;
  }
  if(align == RIGHT)
  {
    for(int i = 7; i > 7 - x; i--)
    {
      int character = number % 10;
      LED[i] = character;
      number /= 10;
    }
    for(int i = 7 - x; i >= 0; i--)
    {
        LED[i] = 18;
    }
    if(flag == 1) LED[7-x] = 16;
  }
}

void displayNumber(int number)
{
  EditNumber(number, CENTER);
}

void LED_Display()
{
    unsigned char bit = 0x01;
    for(int i = 0; i < 8; i++)
    {
        LED_OUT(LED_CA[LED[i]]);
        LED_OUT(bit);
        bit <<= 1;
        digitalWrite(RCLK, LOW);
        digitalWrite(RCLK, HIGH);
    }
}

void LED_OUT(unsigned char x)
{
    unsigned char i;
    for(i = 8; i > 0; i--){
        if(x & B10000000){
            digitalWrite(DIO, HIGH);
        }else{
            digitalWrite(DIO, LOW);
        }
        x<<=1;
        digitalWrite(SCLK, LOW);
        digitalWrite(SCLK, HIGH);
    }
}

void setup() //程序将从这里开始
{
    pinMode(SCLK, OUTPUT);
    pinMode(RCLK, OUTPUT);
    pinMode(DIO, OUTPUT);
    blinker.attach_ms(5,LED_Display); //定时器中断
}
void loop()  //接着在这里循环
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin("http://api.bilibili.com/x/relation/stat?vmid=" + biliuid);
    auto httpCode = http.GET();
    Serial.println(httpCode);
    if (httpCode > 0) {
      String resBuff = http.getString();
      DeserializationError error = deserializeJson(jsonBuffer, resBuff);
      if (error) {
        Serial.println("json error");
        while (1);
      }
      JsonObject root = jsonBuffer.as<JsonObject>();
      long code = root["code"];
      Serial.println(code);
      long fans = root["data"]["follower"];
      Serial.println(fans);
      displayNumber(fans);
      delay(1000);
    }
  }
}
