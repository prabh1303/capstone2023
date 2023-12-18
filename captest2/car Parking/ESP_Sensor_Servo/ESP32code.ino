#include <LiquidCrystal.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>

#define SSID "M32"
#define PASS "admin1234"
#define API_KEY "15"
#define SERVER "https://api.codingprojects.cloud//"

LiquidCrystal lcd(23,22, 21, 19,18,5);  // rs, en, d4, d5, d6, d7
HTTPClient http;

Servo servo_ent, servo_ext;

void display3val(int d) {
  unsigned int temparray[3], temp;
  for (temp = 3; temp > 0; temp--) {
    temparray[temp - 1] = d % 10;
    d = d / 10;
  }
  for (temp = 0; temp < 3; temp++) {
    lcd.write(temparray[temp] + 48);
  }
}

#define SERVO_ENT 16
#define SERVO_EXT 17
#define BUZZER 15
#define s1 13
#define s2 26
#define s3 14
#define s4 27

int Val1 = 0, Val2 = 0;
int v1 = 0, v2 = 0, v3 = 0, v4 = 0, Vs = 0;
int Fl1 = 0, Fl2 = 0, Fl3 = 0, Fl4 = 0;
int Flag=1;

void setup() {
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("SMART CAR ");
  lcd.setCursor(0, 1);
  lcd.print("PARKING SYSTEM");
  delay(3000);
  pinMode(s1, INPUT_PULLUP);
  pinMode(s2, INPUT_PULLUP);
  pinMode(s3, INPUT_PULLUP);
  pinMode(s4, INPUT_PULLUP);

  pinMode(BUZZER, OUTPUT);

  servo_ent.attach(SERVO_ENT);
  servo_ent.write(90);
  servo_ext.attach(SERVO_EXT);
  servo_ext.write(0);
  
  lcd.clear();
  Lprint(0, 0, 0, 0);
  Serial.begin(9600);
  initWiFi();
  delay(10000);
  send_to_server(0,0,0,0,0,0,0,0);
  delay(10000);
  
 
}


void loop() {

  v1 = 1 - digitalRead(s1);
  v2 = 1 - digitalRead(s2);
  v3 = 1 - digitalRead(s3);
  v4 = 1 - digitalRead(s4);


  if ((v1 == 1) & (v2 == 1) & (v3 == 1) & (v4 == 1)) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("PARKING FULL");
      Beep(10, 150);
      servo_ent.attach(SERVO_ENT);
      servo_ent.write(180);
      Flag=1;
      delay(3000);
      lcd.clear();

    } else {
      Beep(1, 200);
      if (Flag==1) {
      servo_ent.attach(SERVO_ENT);
      servo_ent.write(90);
      Flag=2;
      }
      delay(1000);
    }
  Lprint(v1, v2, v3, v4);
  Fl1=EntryExt( v1, Fl1,1);
  Fl2=EntryExt( v2, Fl2,2);
  Fl3=EntryExt( v3, Fl3,3);
  Fl4=EntryExt( v4, Fl4,4);
  get_from_server();
  delay(2000);
}

int EntryExt(int Sn,int Flg,int typ){
  if (Sn == 1) {
    if (Flg==0){
      Flg=1;
      if (typ==1){Serial.println("E1");send_to_server1("field1","1");delay(2000);send_to_server1("field7","1");delay(2000);}
      else if (typ==2){Serial.println("E2");send_to_server1("field2","1");delay(2000);send_to_server1("field8","1");delay(2000);}
      else if (typ==3){Serial.println("E3");send_to_server1("field3","1");delay(2000);send_to_server1("field9","1");delay(2000);}
      else if (typ==4){Serial.println("E4");send_to_server1("field4","1");delay(2000);send_to_server1("field10","1");delay(2000);}
            
    }
  }
  else if (Sn == 0){
    if (Flg==1){
    Flg=0;
      if (typ==1){Serial.println("X1");send_to_server1("field1","0");delay(2000);}
      else if (typ==2){Serial.println("X2");send_to_server1("field2","0");delay(2000);}
      else if (typ==3){Serial.println("X3");send_to_server1("field3","0");delay(2000);}
      else if (typ==4){Serial.println("X4");send_to_server1("field4","0");delay(2000);}
    } 
  } 
  return Flg;
}


void Lprint(int v1, int v2, int v3, int v4) {
  lcd.setCursor(0, 0);
  lcd.print("S1:");
  if (v1 == 0) {
    lcd.setCursor(3, 0);
    lcd.print("EMTY");
  } else {
    lcd.setCursor(3, 0);
    lcd.print("PRKD");
  }

  lcd.setCursor(8, 0);
  lcd.print("S2:");
  if (v2 == 0) {
    lcd.setCursor(12, 0);
    lcd.print("EMTY");
  } else {
    lcd.setCursor(12, 0);
    lcd.print("PRKD");
  }

  lcd.setCursor(0, 1);
  lcd.print("S3:");
  if (v3 == 0) {
    lcd.setCursor(3, 1);
    lcd.print("EMTY");
  } else {
    lcd.setCursor(3, 1);
    lcd.print("PRKD");
  }



  lcd.setCursor(8, 1);
  lcd.print("S4:");
  if (v4 == 0) {
    lcd.setCursor(12, 1);
    lcd.print("EMTY");
  } else {
    lcd.setCursor(12, 1);
    lcd.print("PRKD");
  }
}


void Beep(int times, int D) {
  while (times--) {
    digitalWrite(BUZZER, HIGH);
    delay(D);
    digitalWrite(BUZZER, LOW);
    delay(D);
  }
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASS);
  unsigned long s_time = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - s_time > 5000)
      break;
    delay(500);
  }
}

void send_to_server(int field1, int field2, int field3,int field4,int field7,int field8,int field9,int field10) {
  if (WiFi.status() == WL_CONNECTED) {
    String url = String(SERVER) + "set_values.php?id=" + API_KEY + "&field1=" + String(field1) + "&field2=" + String(field2) + "&field3=" + String(field3)+ "&field4=" + String(field4)+ "&field5=" + "C"+ "&field7=" + String(field7)+"&field8=" + String(field8)+"&field9=" + String(field9)+"&field10=" + String(field10);
    http.begin(url);
    int code = http.GET();
    if (code == HTTP_CODE_MOVED_PERMANENTLY) {
    String newUrl = http.header("Location");
    
    // Make a new request to the redirected URL
    http.begin(newUrl);
    code = http.GET();
  }

    if (code == 200) {
      String payload = http.getString();
      http.end();
    } else {
    }
    http.end();
  }else {
    initWiFi();
  } 
}


void get_from_server() {
  if (WiFi.status() == WL_CONNECTED) {
    String url = String(SERVER) + "get_values.php?id=" + API_KEY;
    http.begin(url);
    int code = http.GET();
    if (code == 200) {
      String payload = http.getString();
      JSONVar json_object = JSON.parse(payload);
      if (JSON.typeof(json_object) == "undefined") {
      }
      char marray[5];
      strcpy(marray, json_object["field5"]);
      Val1 = stoi(marray);
      if (Val1==31){
        lcd.setCursor(0, 2);
        lcd.print("ENTRY GATE: OPEN  ");
        servo_ext.attach(SERVO_EXT);
        servo_ext.write(90);
        delay(4000); 
        lcd.setCursor(0, 2);
        lcd.print("ENTRY GATE: CLOSE");
        servo_ext.attach(SERVO_EXT);
        servo_ext.write(0);
        send_to_server1("field5","C");
      }
    }
    http.end();
  } else {
    initWiFi();
  }
  
}


void send_to_server1(String fieldn,String field1) {
  if (WiFi.status() == WL_CONNECTED) {
    String url = String(SERVER) + "set_values.php?id=" + API_KEY + "&" + fieldn + "=" + field1 ;
    http.begin(url);
    int code = http.GET();
    if (code == 200) {
      String payload = http.getString();
      http.end();
    } else {
    }
    http.end();
  } else {
    initWiFi();
  }
}

int stoi(char *string) {
  int num = 0;
  for (int i = 0; string[i] != '\0'; i++) {
    num = num * 10 + (string[i] - 48);
  }
  return num;
}