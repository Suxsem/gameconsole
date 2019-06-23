byte currentFalling;

void c4Init() {
  state = 0;
  currentPos = 0;
  player = random8(2) == 0;
  blynk = false;
  currentFalling = 1;
  counter = 0;
  autoTarget = random8(7);
  autoJustFound = false;
  for (byte x=0; x<7; x++)
    for (byte y=0; y<6; y++)
      board[x][y] = 0;
}

void c4Draw() {
  CRGB randomColor;
  byte tiltTarget;
  switch (state) {
  case 10: //start
    c4Init();
    break;
  case 0: //choose col
    tiltTarget = 255;
    if (!autoMode) {
      if (player == 0 && wsClt0 < 255)
        tiltTarget = map(wsPos0, -127, 127, 0, 6);
      else if (player == 1 && wsClt1 < 255)
        tiltTarget = map(wsPos1, -127, 127, 0, 6);
    }
    leds[getPoint(currentPos, 0)] = CRGB::Black;
    if (autoMode && currentPos == autoTarget && !autoJustFound) {      
      autoTarget = random8(7);
    }
    if (btnPressed(1) || !digitalRead(BTN_PIN_1) || (tiltTarget < 255 && currentPos > tiltTarget) || (autoMode && currentPos > autoTarget)) {
      if (currentPos > 0)
        currentPos--;      
      blynk = false;
    }
    if (btnPressed(2) || !digitalRead(BTN_PIN_2) || (tiltTarget < 255 && currentPos < tiltTarget) || (autoMode && currentPos < autoTarget)) {
      if (currentPos < 6)
        currentPos++;
      blynk = false;
    }
    if (!autoJustFound) {
      if (currentPos == autoTarget)
        autoJustFound = true;
    } else {
       autoJustFound = false;
    }
    if (btnPressed(3) || (player == 0 && tiltPressed(true, 3, true)) || (player == 1 && tiltPressed(false, 3, true)) || (autoMode && currentPos == autoTarget && !autoJustFound)) {
      if (board[currentPos][0] == 0) {
        state = 1;
        leds[getPoint(currentPos, 0)] = CRGB::Black;
        blynk = false;
        leds[getPoint(currentPos, 1)] = player ? CRGB::Red : CRGB::Blue;
      }
    } else {
      if (!blynk)
        leds[getPoint(currentPos, 0)] = player ? CRGB::Red : CRGB::Blue;
      blynk = !blynk;
    }
    break;
  case 1: //falling
    leds[getPoint(currentPos, currentFalling)] = CRGB::Black;
    if (currentFalling < 7 && board[currentPos][currentFalling-1] == 0) {
      currentFalling++;
      leds[getPoint(currentPos, currentFalling)] = player ? CRGB::Red : CRGB::Blue;
    } else {
      leds[getPoint(currentPos, currentFalling)] = player ? CRGB::Red : CRGB::Blue;
      board[currentPos][currentFalling-2] = player ? 1 : 2;
      if (c4IsWon(currentPos, currentFalling-2)) {
        state = 2;
      } else {
        state = 0;
        player = !player;
      }
      if (c4IsFull()) {
        fade(10);
      }
      currentFalling = 1;
    }
    break;
  case 2: //won glowing
    randomColor = CHSV(random8(),255,255);
    for (byte i=0; i<4; i++) {
      leds[getPoint(winningCells[i][0], winningCells[i][1]+2)] = randomColor;
    }
    counter++;
    if (counter == 25)
      state = 3;
    break;
  case 3: //won
    for (byte i=0; i<4; i++) {
      leds[getPoint(winningCells[i][0], winningCells[i][1]+2)] = CRGB::Black;
      if (!blynk)
        leds[getPoint(winningCells[i][0], winningCells[i][1]+2)] = player ? CRGB::Red : CRGB::Blue;
    }
    blynk = !blynk;

    if (btnPressed(3) || (player == 0 && tiltPressed(true, 3, true)) || (player == 1 && tiltPressed(false, 3, true)) || autoMode) {
      fade(10);
    }
    break;
  }
  
  FastLED.show();
}

boolean c4IsWon(byte lastX, byte lastY) {
  byte marginRight = 6 - lastX;
  byte marginLeft = lastX;
  byte marginBottom = 5 - lastY;
  byte marginTop = lastY;
  byte marginTopLeft = min(marginLeft, marginTop);
  byte marginTopRight = min(marginRight, marginTop);
  byte marginBottomLeft = min(marginLeft, marginBottom);
  byte marginBottomRight = min(marginRight, marginBottom);
  
  for (byte d=0; d<4; d++) {
    for (byte i=0; i<4; i++) {
      byte forward = 3-i;
      byte backward = i;
      if ((d==0 && backward<=marginLeft && forward<=marginRight)
       || (d==1 && backward<=marginTop && forward<=marginBottom)
       || (d==2 && backward<=marginTopLeft && forward<=marginBottomRight)
       || (d==3 && backward<=marginBottomLeft && forward<=marginTopRight)) {
        if (c4IsFour(d, backward, forward, lastX, lastY, false)) {
          c4IsFour(d, backward, forward, lastX, lastY, true);
          return true;
        }
      }
    }
  }
  return false;
}

boolean c4IsFour(byte direct, byte backward, byte forward, byte x, byte y, bool mark) {  
  byte markIndex = 0;
  if (mark) {
    winningCells[markIndex][0] = x;
    winningCells[markIndex][1] = y;
  }  
  switch (direct) {
  case 0: //horiz
    for (byte i=1; i<=forward; i++) {
      if (board[x][y] != board[x+i][y]) {
        return false;
      } else if (mark) {
        winningCells[++markIndex][0] = x+i;
        winningCells[markIndex][1] = y;
      }
    }
    for (byte i=1; i<=backward; i++) {
      if (board[x][y] != board[x-i][y]) {
        return false;
      } else if (mark) {
          winningCells[++markIndex][0] = x-i;
          winningCells[markIndex][1] = y;
      }
    }
    break;
  case 1: //vert
    for (byte i=1; i<=forward; i++) {
      if (board[x][y] != board[x][y+i]) {
        return false;
      } else if (mark) {
          winningCells[++markIndex][0] = x;
          winningCells[markIndex][1] = y+i;
      }
    }
    for (byte i=1; i<=backward; i++) {
      if (board[x][y] != board[x][y-i]) {
        return false;
      } else if (mark) {
          winningCells[++markIndex][0] = x;
          winningCells[markIndex][1] = y-i;
      }
    }
    break;
  case 2: //diag 1
    for (byte i=1; i<=forward; i++) {
      if (board[x][y] != board[x+i][y+i]) {
        return false;
      } else if (mark) {
          winningCells[++markIndex][0] = x+i;
          winningCells[markIndex][1] = y+i;
      }
    }
    for (byte i=1; i<=backward; i++) {
      if (board[x][y] != board[x-i][y-i]) {
        return false;
      } else if (mark) {
          winningCells[++markIndex][0] = x-i;
          winningCells[markIndex][1] = y-i;
      }
    }
    break;
  case 3: //diag 2
    for (byte i=1; i<=forward; i++) {
      if (board[x][y] != board[x+i][y-i]) {
        return false;
      } else if (mark) {
          winningCells[++markIndex][0] = x+i;
          winningCells[markIndex][1] = y-i;
      }
    }
    for (byte i=1; i<=backward; i++) {
      if (board[x][y] != board[x-i][y+i]) {
        return false;
      } else if (mark) {
          winningCells[++markIndex][0] = x-i;
          winningCells[markIndex][1] = y+i;
      }
    }
    break;
  }
  return true;
}

boolean c4IsFull() {
  for (byte i=0; i<7;i++)
    if (board[i][0] == 0)
      return false;
  return true;
}
