//#define FASTLED_INTERRUPT_RETRY_COUNT 1
#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

#define NUM_LEDS 64
#define DATA_PIN  D8
#define BLINK_INT 500

//1 LU
//2 LD
//3 LE
//4 RU
//5 RD
//6 RE
//7 ST

#define BTN_PIN_1 D1
#define BTN_PIN_2 D2
#define BTN_PIN_3 D4
#define BTN_PIN_4 D6
#define BTN_PIN_5 D5
#define BTN_PIN_6 D7
#define BTN_PIN_7 D3

#define LED_PIN D0

#define DEFAULT_FRAME 80
#define DEBOUNCE 150
#define INCLTHR 50
#define INCLRESETTHR 40

CRGB leds[NUM_LEDS];
ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
DNSServer dnsServer;
IPAddress apIP(192, 168, 4, 1);

unsigned int frame = DEFAULT_FRAME;
byte state;
byte nextState;
byte gameChoosen;
byte currentPos;
boolean player;
boolean blynk;
byte board[7][6];
byte winningCells[4][2];
short counter;
bool autoMode = true;
byte autoTarget;
bool autoJustFound;

byte wsClt0 = 255;
byte wsClt1 = 255;
int8_t wsPos0;
int8_t wsPos1;
byte wsPress0;
byte wsPress1;
bool tilting0;
bool tilting1;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght);

void setup() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5,100);
  FastLED.setBrightness(40);

  pinMode(BTN_PIN_1, INPUT_PULLUP);
  pinMode(BTN_PIN_2, INPUT_PULLUP);
  pinMode(BTN_PIN_3, INPUT_PULLUP);
  pinMode(BTN_PIN_4, INPUT_PULLUP);
  pinMode(BTN_PIN_5, INPUT_PULLUP);
  pinMode(BTN_PIN_6, INPUT_PULLUP);
  pinMode(BTN_PIN_7, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  attachInterrupt(digitalPinToInterrupt(BTN_PIN_1), interrHandler1, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN_PIN_2), interrHandler2, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN_PIN_3), interrHandler3, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN_PIN_4), interrHandler4, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN_PIN_5), interrHandler5, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN_PIN_6), interrHandler6, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN_PIN_7), interrHandler7, FALLING);

  random16_add_entropy( random(10000000) );

  WiFi.disconnect(true);
  WiFi.softAP("GameConsole", "suxsemgame");

  digitalWrite(LED_PIN, HIGH);
  Serial.begin(500000);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();

  dnsServer.start(53, "clients3.google.com", apIP); //captive portal detection

  logo();
}

void handleRoot(){
  server.send_P(200, "text/html; charset=utf-8", indexhtml);
}

void handleNotFound(){
  server.send_P(200, "text/html; charset=utf-8", redirecthtml);
}

volatile bool btnPressedAr[8] = {false};
volatile bool btnPressedArInt[8] = {false};
void interrHandler1() { btnPressedArInt[1] = true; }
void interrHandler2() { btnPressedArInt[2] = true; }
void interrHandler3() { btnPressedArInt[3] = true; }
void interrHandler4() { btnPressedArInt[4] = true; }
void interrHandler5() { btnPressedArInt[5] = true; }
void interrHandler6() { btnPressedArInt[6] = true; }
void interrHandler7() { btnPressedArInt[7] = true; }

unsigned long lastFrame = 0;

void loop() {
  webSocket.loop();
  dnsServer.processNextRequest();
  server.handleClient();
  checkDebounce();
  statusLed();
  if (millis() - lastFrame > frame) {
    draw();
    resetButtons();
    lastFrame = millis();
  }
}

void draw() {
  serviceDraw();
  if (state < 20) { //connect4
    c4Draw();
  } else if (state < 40) {//ttt
    tttDraw();
  } else if (state < 60) {//pong
    pongDraw();
  }
}

short getPoint(byte x, byte y) {
  return 63 - (y*8 + x);
}

void serviceDraw() {
  switch(state) {
  case 200: //intro
    leds[getPoint(pgm_read_byte_near(suxsemLogoX+counter), pgm_read_byte_near(suxsemLogoY+counter))] = CHSV(random8(),255,255);
    if (counter>2 && counter<7) {
      leds[getPoint(3-(counter-3), 7)] = CRGB::Red;
      leds[getPoint(4+(counter-3), 7)] = CRGB::Red;
    }
    counter++;
    if (counter == 14) {
      counter = 0;
      state = 201;
    }
    break;
  case 201: //intro 2
    leds[getPoint(pgm_read_byte_near(suxsemLogoX+counter), pgm_read_byte_near(suxsemLogoY+counter))] = CRGB::Black;
    if (counter > 8 && counter < 13) {
      leds[getPoint(counter-9, 7)] = CRGB::Black;
      leds[getPoint(7-(counter-9), 7)] = CRGB::Black;
    }
    counter++;
    if (counter == 14) {
      fade(210);
    }
    break;
  case 202: //fade
    if (counter > 0) {
      for (byte i=0; i<(8-2*(counter-1)); i++) {
        leds[getPoint(counter+i-1, counter-1)] = CRGB::Black;
        leds[getPoint(counter-1, counter+i-1)] = CRGB::Black;
        leds[getPoint(7-(counter+i-1), 7-(counter-1))] = CRGB::Black;
        leds[getPoint(7-(counter-1), 7-(counter+i-1))] = CRGB::Black;
      }
    }
    for (byte i=0; i<(8-2*counter); i++) {
      leds[getPoint(counter+i, counter)] = CRGB::White;
      leds[getPoint(counter, counter+i)] = CRGB::White;
      leds[getPoint(7-(counter+i), 7-counter)] = CRGB::White;
      leds[getPoint(7-counter, 7-(counter+i))] = CRGB::White;
    }
    counter++;
    if (counter == 4) {
      counter = 2;
      state = 203;
    }
    break;
  case 203: //fade 2
    for (byte i=0; i<(8-2*(counter+1)); i++) {
      leds[getPoint(counter+i+1, counter+1)] = CRGB::Black;
      leds[getPoint(counter+1, counter+i+1)] = CRGB::Black;
      leds[getPoint(7-(counter+i+1), 7-(counter+1))] = CRGB::Black;
      leds[getPoint(7-(counter+1), 7-(counter+i+1))] = CRGB::Black;
    }
    for (byte i=0; i<(8-2*counter); i++) {
      leds[getPoint(counter+i, counter)] = CRGB::White;
      leds[getPoint(counter, counter+i)] = CRGB::White;
      leds[getPoint(7-(counter+i), 7-counter)] = CRGB::White;
      leds[getPoint(7-counter, 7-(counter+i))] = CRGB::White;
    }
    if (counter == 0) {
      counter = 0;
      state = 204;
    } else {
      counter--;
    }
    break;
  case 204: //fade 3
    if (counter == 0)
      FastLED.clear();
    if (counter == 2) {
      state = nextState;
    } else {
      counter++;
    }
    break;
  case 210: // select connect4
    for (byte i=0; i<4; i++)
      leds[getPoint(pgm_read_byte_near(connect4RedX+i), pgm_read_byte_near(connect4RedY+i))] = CRGB::Red;
    for (byte i=0; i<6; i++)
      leds[getPoint(pgm_read_byte_near(connect4BlueX+i), pgm_read_byte_near(connect4BlueY+i))] = CRGB::Blue;
    if (btnPressed(false, 3) || tiltPressed(false, 3, true)) {
      FastLED.clear();
      gameChoosen = 10;
      state = 220;
    }
    if (btnPressed(false, 1) || tiltPressed(false, 1, true)) {
      FastLED.clear();
      state = 212;
    }
    if (btnPressed(false, 2) || tiltPressed(false, 2, true)) {
      FastLED.clear();
      state = 211;
    }
    break;
  case 211: // select ttt
    for (byte i=0; i<8; i++)
      leds[getPoint(pgm_read_byte_near(tttRedX+i), pgm_read_byte_near(tttRedY+i))] = CRGB::Red;
    for (byte i=0; i<5; i++)
      leds[getPoint(pgm_read_byte_near(tttGreenX+i), pgm_read_byte_near(tttGreenY+i))] = CRGB::Green;
    if (btnPressed(false, 3) || tiltPressed(false, 3, true)) {
      FastLED.clear();
      gameChoosen = 21;
      state = 220;
    }
    if (btnPressed(false, 1) || tiltPressed(false, 1, true)){
      FastLED.clear();
      state = 210;
    }
    if (btnPressed(false, 2) || tiltPressed(false, 2, true)) {
      FastLED.clear();
      state = 212;
    }
    break;
  case 212: // select pong
    for (byte i=0; i<3; i++) {
      leds[getPoint(pgm_read_byte_near(pong4GreenX+i), pgm_read_byte_near(pong4GreenY+i))] = CRGB::YellowGreen;
      leds[getPoint(pgm_read_byte_near(pong4PinkX+i), pgm_read_byte_near(pong4PinkY+i))] = CRGB::PaleVioletRed;
    }
    for (byte i=0; i<4; i++)
      leds[getPoint(pgm_read_byte_near(pong4YellowX+i), pgm_read_byte_near(pong4YellowY+i))] = CRGB::Yellow;
    if (btnPressed(false, 3) || tiltPressed(false, 3, true)) {
      FastLED.clear();
      gameChoosen = 41;
      state = 220;
    }
    if (btnPressed(false, 1) || tiltPressed(false, 1, true)){
      FastLED.clear();
      state = 211;
    }
    if (btnPressed(false, 2) || tiltPressed(false, 2, true)) {
      FastLED.clear();
      state = 210;
    }
    break;
  case 220: // select manual
    for (byte i=0; i<20; i++)
      leds[getPoint(pgm_read_byte_near(humanYellowX+i), pgm_read_byte_near(humanYellowY+i))] = CRGB::Yellow;
    for (byte i=0; i<6; i++)
      leds[getPoint(pgm_read_byte_near(humanRedX+i), pgm_read_byte_near(humanRedY+i))] = CRGB::Red;
    for (byte i=0; i<2; i++)
      leds[getPoint(pgm_read_byte_near(humanBlueX+i), pgm_read_byte_near(humanBlueY+i))] = CRGB::Blue;
    for (byte i=0; i<6; i++)
      leds[getPoint(pgm_read_byte_near(humanBrownX+i), pgm_read_byte_near(humanBrownY+i))] = CRGB::Brown;
    if (btnPressed(false, 1) || tiltPressed(false, 1, true)) {
      FastLED.clear();
      state = 221;
    }
    if (btnPressed(false, 2) || tiltPressed(false, 2, true)) {
      FastLED.clear();
      state = 221;
    }
    if (btnPressed(false, 3) || tiltPressed(false, 3, true)) {
      autoMode = false;
      fade(gameChoosen);
    }
    break;
  case 221: // select auto
    for (byte i=0; i<22; i++)
      leds[getPoint(pgm_read_byte_near(pcGreenX+i), pgm_read_byte_near(pcGreenY+i))] = CRGB::Green;
    for (byte i=0; i<6; i++)
      leds[getPoint(pgm_read_byte_near(pcRedX+i), pgm_read_byte_near(pcRedY+i))] = CRGB::Red;
    if (btnPressed(false, 1) || tiltPressed(false, 1, true)) {
      FastLED.clear();
      state = 220;
    }
    if (btnPressed(false, 2) || tiltPressed(false, 2, true)) {
      FastLED.clear();
      state = 220;
    }
    if (btnPressed(false, 3) || tiltPressed(false, 3, true)) {
      autoMode = true;
      fade(gameChoosen);
    }
    break;

  }

  if (btnPressed(false, 7)) {
    frame = DEFAULT_FRAME;
    fade(210);
  }
  
  FastLED.show();
}

void fade(byte mNextState) {
  nextState = mNextState;
  state = 202;
  counter = 0;
}

void logo() {
  state = 200;
  counter = 0;
}

unsigned long lastStatusLed;
void statusLed() {
  if (wsClt0 < 255 || wsClt1 < 255) {
    if (millis() - lastStatusLed > BLINK_INT) {
      lastStatusLed = millis();
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }
  } else {
    digitalWrite(LED_PIN, HIGH);
  }
}

unsigned long lastDebounce;
void checkDebounce() {
  for (byte i=1; i<=7; i++) {
    if (btnPressedArInt[i]) {
      if (millis() - lastDebounce < DEBOUNCE) {
        btnPressedArInt[i] = false;
      } else {
        btnPressedAr[i] = true;
        lastDebounce = millis();
      }
      break;
    }
  }
}

boolean btnPressed(bool player1, byte btn) {
  if (player1)
    btn += 3;
  if (btnPressedAr[btn]) {
      btnPressedAr[btn] = false;
      return true;
  }
  return false;
}

boolean btnIsPressed(bool player1, byte btn) {
  if (player1)
    btn += 3;
  switch(btn) {
    case 1:
      return !digitalRead(BTN_PIN_1);
    break;
    case 2:
      return !digitalRead(BTN_PIN_2);
    break;
    case 3:
      return !digitalRead(BTN_PIN_3);
    break;
    case 4:
      return !digitalRead(BTN_PIN_4);
    break;
    case 5:
      return !digitalRead(BTN_PIN_5);
    break;
    case 6:
      return !digitalRead(BTN_PIN_6);
    break;
    case 7:
      return !digitalRead(BTN_PIN_7);
    break;
  }
}

boolean tiltPressed(bool player1, byte side, boolean first) {
  bool pressed = false;
  int8_t wsPos = player1 ? wsPos1 : wsPos0;
  bool tilting = player1 ? tilting1 : tilting0;
  bool wsPress = player1 ? wsPress1 : wsPress0;
  switch(side) {
    case 1:
      if (wsPos < -INCLTHR) {
        if (!first || !tilting)
          pressed = true;
        tilting = true;
      }
    break;
    case 2:
      if (wsPos > INCLTHR) {
        if (!first || !tilting)
          pressed = true;
        tilting = true;
      }
    break;
    case 3:
      pressed = wsPress;
      wsPress = false;
    break;
  }
  if (player1) {
    tilting1 = tilting;
    wsPress1 = wsPress;
  } else {
    tilting0 = tilting;
    wsPress0 = wsPress;
  }
  return pressed;
}

void resetButtons() {
  for (int i=1; i<=7; i++) {
    btnPressedAr[i] = false;
  }
  wsPress0 = false;
  wsPress1 = false;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
  char player;
  char cmd;
  switch (type) {
    case WStype_DISCONNECTED:
      if (wsClt0 == num)
        wsClt0 = 255;
      else if (wsClt1 == num)
        wsClt1 = 255;
      break;
    case WStype_CONNECTED:
      break;
    case WStype_TEXT:
      player = payload[0];
      cmd = payload[1];
      if (player == '1') {
        wsClt0 = num;
        if (wsClt1 == wsClt0)
          wsClt1 = 255;
      } else if (player == '2') {
        wsClt1 = num;
        if (wsClt0 == wsClt1)
          wsClt0 = 255;
      }
      if (cmd == 'P') {
        if (player == '1')
          wsPress0 = true;
        else if (player == '2')
          wsPress1 = true;
      } else if (cmd == 'M') {
        int valueRcv = atoi((const char *)&payload[2]);
        if (player == '1') {
          if (valueRcv < INCLRESETTHR && valueRcv > -INCLRESETTHR)
            tilting0 = false;
          wsPos0 = valueRcv;
        } else if (player == '2') {
          if (valueRcv < INCLRESETTHR && valueRcv > -INCLRESETTHR)
            tilting1 = false;
          wsPos1 = valueRcv;
        }
      }
      break;
  }
}
