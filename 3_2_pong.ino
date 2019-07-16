#define INCREMENT_EVERY 500

byte currentPos2;
byte ballPosX;
byte ballPosY;
byte ballDirection;
byte framesPerBall;

/*
 * 7 8 9     7 9
 * 4 5 6  => 4 6
 * 1 2 3     1 3
 */

void pongInit() {
  currentPos = 3;
  ballPosX = 1;
  ballPosY = 3;
  ballDirection = 9;
  currentPos2 = 4;
  state = 42;
  counter = 0;
  framesPerBall = 5;
}

void pongDraw() {
  CRGB randomColor;
  byte tiltTarget1;
  byte tiltTarget2;
  int8_t autoOffset1 = 0;
  int8_t autoOffset2 = 0;
  uint8_t autoRandom;
  counter++;
  switch (state) {
  case 41: //start
    pongInit();
    break;
  case 42: //playing
    tiltTarget1 = 255;
    tiltTarget2 = 255;
    if (!autoMode) {
      if (wsClt0 < 255)
        tiltTarget1 = map(wsPos0, -127, 127, 0, 6);
      if (wsClt1 < 255)
        tiltTarget2 = map(wsPos1, -127, 127, 0, 6);
    } else {
      autoRandom = random8();
      if (autoRandom < 10 && ballDirection == 1)
        autoOffset1 = 1;
      else if (autoRandom > 245 && ballDirection == 7)
        autoOffset1 = -2;
      autoRandom = random8();
      if (autoRandom < 10 && ballDirection == 3)
        autoOffset2 = 2;
      else if (autoRandom > 245 && ballDirection == 9)
        autoOffset2 = -1;
      if (ballPosX <= 3)
        tiltTarget1 = ballPosY + autoOffset1;
      if (ballPosX >= 4)
        tiltTarget2 = ballPosY - 1 + autoOffset2;
    }
    leds[getPoint(ballPosX, ballPosY)] = CRGB::Black;
    leds[getPoint(0, currentPos)] = CRGB::Black;
    leds[getPoint(0, currentPos+1)] = CRGB::Black;
    leds[getPoint(7, currentPos2)] = CRGB::Black;
    leds[getPoint(7, currentPos2+1)] = CRGB::Black;
    if (btnPressed(false, 1) || btnIsPressed(false, 1) || (tiltTarget1 < 255 && currentPos > tiltTarget1)) {
      if (currentPos > 0)
        currentPos--;
    }
    if (btnPressed(false, 2) || btnIsPressed(false, 2) || (tiltTarget1 < 255 && currentPos < tiltTarget1)) {
      if (currentPos < 6)
        currentPos++;
    }    
    if (btnPressed(true, 1) || btnIsPressed(true, 1) || (tiltTarget2 < 255 && currentPos2 > tiltTarget2)) {
      if (currentPos2 > 0)
        currentPos2--;
    }
    if (btnPressed(true, 2) || btnIsPressed(true, 2) || (tiltTarget2 < 255 && currentPos2 < tiltTarget2)) {
      if (currentPos2 < 6)
        currentPos2++;
    }
    leds[getPoint(0, currentPos)] = CRGB::YellowGreen;
    leds[getPoint(0, currentPos+1)] = CRGB::YellowGreen;
    leds[getPoint(7, currentPos2)] = CRGB::PaleVioletRed;
    leds[getPoint(7, currentPos2+1)] = CRGB::PaleVioletRed;
    if (counter % framesPerBall == 0) {
      if (!pongNextBall()) {
        state = 44;
        counter = 0;
      }
    }
    leds[getPoint(ballPosX, ballPosY)] = CRGB::Yellow;
    if (counter % INCREMENT_EVERY == 0 && framesPerBall > 1)
      framesPerBall--;
    break;
  case 44: //won
    randomColor = CHSV(random8(),255,255);
    if (!player) {
      leds[getPoint(0, currentPos)] = randomColor;
      leds[getPoint(0, currentPos + 1)] = randomColor;
    } else {
      leds[getPoint(7, currentPos2)] = randomColor;
      leds[getPoint(7, currentPos2 + 1)] = randomColor;
    }

    if (counter == 25)
      state = 45;
    
    break;    
  case 45: //won ready
    if (!player) {
      leds[getPoint(0, currentPos)] = CRGB::YellowGreen;
      leds[getPoint(0, currentPos + 1)] = CRGB::YellowGreen;
    } else {
      leds[getPoint(7, currentPos2)] = CRGB::PaleVioletRed;
      leds[getPoint(7, currentPos2 + 1)] = CRGB::PaleVioletRed;
    }
    if (btnPressed(player, 3) || tiltPressed(player, 3, true) || autoMode) {
      fade(41);
    }
    break;
  }
  
  FastLED.show();
}

bool pongNextBall() {
  bool continueOrNot = true;
  // left side  
  if (ballPosX == 1 && (ballDirection == 1 || ballDirection == 4 || ballDirection == 7)) {
    if (ballPosY == currentPos - 1 && ballDirection == 1) {
      ballDirection = 9;
    } else if (ballPosY == currentPos - 1 && ballPosY == 0 && ballDirection == 7) {
      ballDirection = 3;
    } else if ((ballPosY == currentPos || ballPosY == currentPos + 1) && ballDirection == 1) {
      ballDirection = 3;
    } else if ((ballPosY == currentPos || ballPosY == currentPos + 1) && ballDirection == 4) {
      ballDirection = 6;
    } else if ((ballPosY == currentPos || ballPosY == currentPos + 1) && ballDirection == 7) {
      ballDirection = 9;
    } else if (ballPosY == currentPos + 2 && ballDirection == 7) {
      ballDirection = 3;
    } else if (ballPosY == currentPos + 2 && ballPosY == 7 && ballDirection == 1) {
      ballDirection = 9;
    } else {
      continueOrNot = false;
      player = 1;
    }
  }
  // right side
  else if (ballPosX == 6 && (ballDirection == 3 || ballDirection == 6 || ballDirection == 9)) {
    if (ballPosY == currentPos2 - 1 && ballDirection == 3) {
      ballDirection = 7;
    } else if (ballPosY == currentPos2 - 1 && ballPosY == 0 && ballDirection == 9) {
      ballDirection = 1;
    } else if ((ballPosY == currentPos2 || ballPosY == currentPos2 + 1) && ballDirection == 3) {
      ballDirection = 1;
    } else if ((ballPosY == currentPos2 || ballPosY == currentPos2 + 1) && ballDirection == 6) {
      ballDirection = 4;
    } else if ((ballPosY == currentPos2 || ballPosY == currentPos2 + 1) && ballDirection == 9) {
      ballDirection = 7;
    } else if (ballPosY == currentPos2 + 2 && ballDirection == 9) {
      ballDirection = 1;
    } else if (ballPosY == currentPos2 + 2 && ballPosY == 7 && ballDirection == 3) {
      ballDirection = 7;
    } else {
      continueOrNot = false;
      player = 1;
    }
  }
  // edge top and bottom
  if (ballPosY == 0 && ballDirection == 9)
    ballDirection = 3;
  else if (ballPosY == 0 && ballDirection == 7)
    ballDirection = 1;
  else if (ballPosY == 7 && ballDirection == 3)
    ballDirection = 9;
  else if (ballPosY == 7 && ballDirection == 1)
    ballDirection = 7;
  if (ballPosX > 0 && ballPosX < 7) {
    switch (ballDirection) {
      case 7:
        ballPosX--;
        ballPosY--;
        break;
      case 4:
        ballPosX--;
        break;
      case 1:
        ballPosX--;
        ballPosY++;
        break;
      case 9:
        ballPosX++;
        ballPosY--;
        break;
      case 6:
        ballPosX++;
        break;
      case 3:
        ballPosX++;
        ballPosY++;
        break;
    }
  }
  return continueOrNot;
}
