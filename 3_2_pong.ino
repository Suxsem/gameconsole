byte currentPos2;
byte ballPosX;
byte ballPosY;
byte ballDirection;
/*
 * 7 8 9     7 9
 * 4 5 6  => 4 6
 * 1 2 3     1 3
 */

void pongInit() {
  frame = 240;
  currentPos = 3;
  ballPosX = 1;
  ballPosY = 3;
  ballDirection = 6;
  currentPos2 = 4;
  state = 42;
}

void pongDraw() {
  CRGB randomColor;
  byte tiltTarget1;
  byte tiltTarget2;
  switch (state) {
  case 41: //start
    pongInit();
    break;
  case 42: //playing
    tiltTarget1 = 255;
    tiltTarget2 = 255;
    if (wsClt0 < 255)
      tiltTarget1 = map(wsPos0, -127, 127, 0, 7);
    if (wsClt1 < 255)
      tiltTarget2 = map(wsPos1, -127, 127, 0, 7);
    leds[getPoint(ballPosX, ballPosY)] = CRGB::Black;
    leds[getPoint(0, currentPos)] = CRGB::Black;
    leds[getPoint(7, currentPos2)] = CRGB::Black;
    if (btnPressed(false, 1) || btnIsPressed(false, 1) || (tiltTarget1 < 255 && currentPos > tiltTarget1)) {
      if (currentPos > 0)
        currentPos--;
    }
    if (btnPressed(false, 2) || btnIsPressed(false, 2) || (tiltTarget1 < 255 && currentPos < tiltTarget1)) {
      if (currentPos < 7)
        currentPos++;
    }    
    if (btnPressed(true, 1) || btnIsPressed(true, 1) || (tiltTarget2 < 255 && currentPos2 > tiltTarget2)) {
      if (currentPos2 > 0)
        currentPos2--;
    }
    if (btnPressed(true, 2) || btnIsPressed(true, 2) || (tiltTarget2 < 255 && currentPos2 < tiltTarget2)) {
      if (currentPos2 < 7)
        currentPos2++;
    }
    leds[getPoint(0, currentPos)] = CRGB::YellowGreen;
    leds[getPoint(7, currentPos2)] = CRGB::PaleVioletRed;
    if (!pongNextBall()) {
      state = 44;
    }
    leds[getPoint(ballPosX, ballPosY)] = CRGB::Yellow;
    if (frame > 50)
      frame--;
    break;
  case 44: //won glowing
    frame = DEFAULT_FRAME;
    randomColor = CHSV(random8(),255,255);
    if (!player)
      leds[getPoint(0, currentPos)] = randomColor;
    else
      leds[getPoint(7, currentPos2)] = randomColor;
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
  if (ballPosX == 1) {
    if (currentPos - 1 == ballPosY) {
      ballDirection = 9;
    } else if (currentPos == ballPosY) {
      ballDirection = 6;
    } else if (currentPos + 1 == ballPosY) {
      ballDirection = 3;
    } else {
      continueOrNot = false;
      player = 1;
    }
  }
  // right side
  else if (ballPosX == 6) {
    if (currentPos2 -1 == ballPosY) {
      ballDirection = 7;
    } else if (currentPos2 == ballPosY) {
      ballDirection = 4;
    } else if (currentPos2 + 1 == ballPosY) {
      ballDirection = 1;
    } else {
      continueOrNot = false;
      player = 0;
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
