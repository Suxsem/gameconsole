bool forceFirstCell;

void tttInit() {
  if (autoMode)
    frame = 320;
  state = 22;
  currentPos = 0;
  player = random8(2) == 0;
  blynk = false;
  counter = 0;
  autoTarget = 10;
  forceFirstCell = true;
  autoJustFound = false;
  for (byte x=0; x<3; x++)
    for (byte y=0; y<3; y++)
      board[x][y] = 0;
}

void tttDraw() {
  bool cellAvailable;
  CRGB randomColor;
  byte tiltTarget;
  switch (state) {
  case 21: //start
    tttInit();
    for (byte i=0; i<8; i++) {
      for (byte j=1; j<3; j++) {
        leds[getPoint(i, 3*j-1)] = CRGB::SlateGray;
        leds[getPoint(3*j-1, i)] = CRGB::SlateGray;
      }
    }
    break;
  case 22: //choose pos
    tiltTarget = 255;
    if (!autoMode) {
      if (player == 0 && wsClt0 < 255)
        tiltTarget = tttTiltFree(wsPos0);
      else if (player == 1 && wsClt1 < 255)
        tiltTarget = tttTiltFree(wsPos1);
    }
    tttFillCell(currentPos, CRGB::Black);
    if (autoMode && (autoTarget == 10 || (currentPos == autoTarget && !autoJustFound))) {
      autoTarget = tttRandomFree();
    }
    if (btnPressed(player, 1) || btnIsPressed(player, 1) || (tiltTarget < 255 && currentPos > tiltTarget) || (autoMode && !forceFirstCell && currentPos > autoTarget)) {
      for (int8_t i=currentPos-1; i>=0; i--) {
        if (board[i%3][i/3] == 0) {
          currentPos = i;
          break;
        }
      }
      blynk = false;
    }
    if (btnPressed(player, 2) || btnIsPressed(player, 2) || (tiltTarget < 255 && currentPos < tiltTarget) || (autoMode && !forceFirstCell && currentPos < autoTarget)) {
      for (byte i=currentPos+1; i<9; i++) {
        if (board[i%3][i/3] == 0) {
          currentPos = i;
          break;
        }
      }
      blynk = false;
    }
    if (!autoJustFound) {
      if (currentPos == autoTarget) {
        autoJustFound = true;
      }
    } else {
       autoJustFound = false;
    }
    forceFirstCell = false;
    if (btnPressed(player, 3) || tiltPressed(player, 3, true) || (autoMode && currentPos == autoTarget && !autoJustFound)) {
      tttFillCell(currentPos, player ? CRGB::Red : CRGB::Green);
      board[currentPos % 3][currentPos / 3] = player ? 1 : 2;
      if (tttIsWon()) {
        state = 23;
      } else {
        forceFirstCell = true;
        player = !player;
        cellAvailable = false;
        autoTarget = 10;
        blynk = false;
        for (byte i=0; i<9; i++) {
          if (board[i%3][i/3] == 0) {
            cellAvailable = true;
            currentPos = i;
            break;
          }
        }
        if (!cellAvailable)
          fade(21);
      }
    } else {
      if (!blynk)
        tttFillCell(currentPos, player ? CRGB::Red : CRGB::Green);
      blynk = !blynk;
    }
    break;
  case 23: //won glowing
    randomColor = CHSV(random8(),255,255);
    for (byte i=0; i<3; i++) {
      tttFillCell(winningCells[i][0], randomColor);
    }
    counter++;
    if (counter == 25)
      state = 24;
    break;
  case 24: //won
    for (byte i=0; i<3; i++) {
      tttFillCell(winningCells[i][0], CRGB::Black);
      if (!blynk)
        tttFillCell(winningCells[i][0], player ? CRGB::Red : CRGB::Green);
    }
    blynk = !blynk;
    if (btnPressed(player, 3) || tiltPressed(player, 3, true) || autoMode) {
      fade(21);
    }
    break;
  }
  FastLED.show();
}

void tttFillCell(byte cell, const struct CRGB &color) {
  byte x = (cell % 3) * 3;
  byte y = cell / 3 * 3;
  leds[getPoint(x, y)] = color;
  leds[getPoint(x+1, y)] = color;
  leds[getPoint(x, y+1)] = color;
  leds[getPoint(x+1, y+1)] = color;
}

boolean tttIsWon() {
  //vert
  for (byte i=0; i<3; i++) {
    if (board[i][0] > 0 && board[i][0] == board[i][1] && board[i][0] == board[i][2]) {
      for (byte j=0; j<3; j++) {
        winningCells[j][0] = j*3+i;
      }
      return true;
    }
  }
  //horiz
  for (byte i=0; i<3; i++) {
    if (board[0][i] > 0 && board[0][i] == board[1][i] && board[0][i] == board[2][i]) {
      for (byte j=0; j<3; j++) {
        winningCells[j][0] = j+i*3;
      }
      return true;
    }
  }
  //diag1
  if (board[0][0] > 0 && board[0][0] == board[1][1] && board[0][0] == board[2][2]) {
    for (byte j=0; j<3; j++) {
      winningCells[j][0] = j*3+j;
    }
    return true;
  }
  //diag2
  if (board[2][0] > 0 && board[2][0] == board[1][1] && board[2][0] == board[0][2]) {
    for (byte j=0; j<3; j++) {
      winningCells[j][0] = j*3+(2-j);
    }
    return true;
  }
  return false;
}

byte tttRandomFree() {
  byte freePositions[9] = {10};
  byte freeCount = 0;
  for (byte pos=0; pos<9; pos++)
    if (board[pos%3][pos/3] == 0)
      freePositions[freeCount++] = pos;
  return freePositions[random8(freeCount)];
}

byte tttTiltFree(int8_t wsPos) {
  byte freePositions[9] = {10};
  byte freeCount = 0;
  for (byte pos=0; pos<9; pos++)
    if (board[pos%3][pos/3] == 0)
      freePositions[freeCount++] = pos;
  byte mapFree = map(wsPos, -127, 127, 0, freeCount-1);
  return freePositions[mapFree];
}
