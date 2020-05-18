#include "LedControl.h"
#include "Matrix.h"

unsigned char reverse_byte(unsigned char b) {
   byte r = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   r = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   r = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return r;
}

byte _rotateByteRight(byte b) {
  return (b >> 1) | (b << 7);
}
byte _rotateByteLeft(byte b) {
  return (b << 1) | (b >> 7);
}

Matrix :: Matrix(MatrixPinInterface pinInterface): renderInterval(300), tick(0), ledMatrix(pinInterface.digitalIn,pinInterface.clock,pinInterface.chipSelect,1) {
  ledMatrix.shutdown(0,false);
  ledMatrix.clearDisplay(0);

  currentFrame.intensity = 15;
  currentFrame.inverted = false;
  currentFrame.orientation = East;
  lastRenderTime = millis();

  clearLightMatrix();
  
}

void Matrix :: clearLightMatrix() {
  for (int x = 0; x < 8; x++) {
    currentFrame.lightMatrix[x] = 0;
  }
}

void Matrix :: setNextFrame(RenderFrame r) {
  // TODO which should I do?
  // currentFrame = r;
  currentFrame.intensity = r.intensity;
  currentFrame.orientation = r.orientation;
  currentFrame.inverted = r.inverted;

  for (int x = 0; x < 8; x++) {
    currentFrame.lightMatrix[x] = r.lightMatrix[x];
  }
}

void Matrix :: setRenderInterval(int newInterval) {
  if (renderInterval != newInterval) {
    renderInterval = newInterval;
  }
}

void Matrix :: renderNextFrame() {
  ledMatrix.setIntensity(0, currentFrame.intensity);

  for (int i=0; i<8; i++) {
    byte displayTuple = currentFrame.inverted ? ~currentFrame.lightMatrix[i] : currentFrame.lightMatrix[i];
    int index = (currentFrame.orientation == North || currentFrame.orientation == West) ? i : 7 - i;
    if (currentFrame.orientation == East || currentFrame.orientation == West) {
      ledMatrix.setColumn(0, index, displayTuple);
    } else {
      ledMatrix.setRow(0, index, displayTuple);
    }
  }
}

void Matrix :: tapRenderLoop() {
  int time = millis();
  if ((time - lastRenderTime) >= renderInterval) {
    lastRenderTime = time;

    if (frameTransformer != NULL) {
      frameTransformer(&currentFrame, tick);
    }

    renderNextFrame();
    tick++;
    if (tick >= 100) {
      tick = 0;
    }
 
  }
}

void Matrix :: setFrameTransformer(void (*newFrameTransformer)(RenderFrame*, byte)) {
  frameTransformer = newFrameTransformer;
}

void Matrix :: SlideDown(RenderFrame *r, byte tick) {
  byte buf;
  buf = r->lightMatrix[7];

  for (int x = 7; x > 0; x--) {
    r->lightMatrix[x] = r->lightMatrix[x-1];
  }
  r->lightMatrix[0] = buf;
}

void Matrix :: SlideUp(RenderFrame *r, byte tick) {
  byte buf;
  buf = r->lightMatrix[0];
  for (int x = 0; x < 7; x++) {
    r->lightMatrix[x] = r->lightMatrix[x+1];
  }
  r->lightMatrix[7] = buf;
}

void Matrix :: SlideRight(RenderFrame *r, byte tick) {
  for (int x = 0; x < 8; x++) {
    r->lightMatrix[x] = _rotateByteRight(r->lightMatrix[x]);
  }
}

void Matrix :: SlideLeft(RenderFrame *r, byte tick) {
  for (int x = 0; x < 8; x++) {
    r->lightMatrix[x] = _rotateByteLeft(r->lightMatrix[x]);
  }
}
void Matrix :: Interlace(RenderFrame *r, byte tick) {
  byte *counter = &r->state[0];
  
  //  Keep a counter so we only interlace N full rotations
  if (!*counter) *counter = 1;

  if (*counter <= 8 * 2) {

    for (int x = 0; x < 8; x++) {
      if (x % 2 == 0) r->lightMatrix[x] = _rotateByteLeft(r->lightMatrix[x]);
      else r->lightMatrix[x] = _rotateByteRight(r->lightMatrix[x]);
    }
    (*counter)++;
  }
}

void Matrix :: Rotate(RenderFrame *r, byte tick) {
  r->orientation = static_cast<DisplayOrientation>((r->orientation + 1) % 4);
}
void Matrix :: Blink(RenderFrame *r, byte tick) {
  r->inverted = !r->inverted;
}
void Matrix :: Flash(RenderFrame *r, byte tick) {
  byte centiTick = tick % 10;
  if (centiTick == 0) {
    // Hide display frame in buffer
    for (int x = 0; x < 8; x++) {
      r->state[x] = r->lightMatrix[x];
      r->lightMatrix[x] = 0;
    }
  }
  if (centiTick == 5) {
    // Move from buffer into display
    for (int x = 0; x < 8; x++) {
      r->lightMatrix[x] = r->state[x];
    }
  }
}
void Matrix :: FlickerIn(RenderFrame *r, byte tick) {
  //  Keep a counter
  // TODO assign pointer for readability
  if (!r->state[9]) r->state[9] = 1;
  if (r->state[9] <= 36) {

    byte centiTick = tick % 2;
    if (centiTick == 0) {
      // Hide display frame in buffer
      for (int x = 0; x < 8; x++) {
        r->state[x] = r->lightMatrix[x];
        r->lightMatrix[x] = 0;
      }
    }
    else {
      // Move from buffer into display
      for (int x = 0; x < 8; x++) {
        r->lightMatrix[x] = r->state[x];
      }
    }
    r->state[9]++;
  }
}

void Matrix :: CollapseOut(RenderFrame *r, byte tick) {

  if (tick % 20 == 0) {
    for (int x = 6; x > 0; x--) {
      r->lightMatrix[x] = r->lightMatrix[x-1];
    }
    r->lightMatrix[7] = r->lightMatrix[7] | r->lightMatrix[6];
  }
}

void Matrix :: GlitchIn(RenderFrame *r, byte tick) {
  //  Keep a counter
  byte *flickerCounter = &r->state[9];
  byte *interLacerCounter = &r->state[10];
  byte flickerSpeed = 2;

  if (!*interLacerCounter) *interLacerCounter = 1;
  if (!*flickerCounter) *flickerCounter = 1;

  if (*flickerCounter <= 100) {

    flickerSpeed = *flickerCounter <= 74 ? 2 : 4;

    byte centiTick = tick % flickerSpeed;
    if (centiTick == 0) {
      // Hide display frame in buffer
      for (int x = 0; x < 8; x++) {
        r->state[x] = r->lightMatrix[x];
        r->lightMatrix[x] = 0;
      }
    }
    else {
      // Move from buffer into display
      for (int x = 0; x < 8; x++) {
        r->lightMatrix[x] = r->state[x];
      }
    }
    (*flickerCounter)++;
  }

  if (*interLacerCounter <= 8 * 8) {

  for (int x = 0; x < 8; x++) {
    if (x % 2 == 0) r->lightMatrix[x] = _rotateByteLeft(r->lightMatrix[x]);
    else r->lightMatrix[x] = _rotateByteRight(r->lightMatrix[x]);
  }
  r->state[10]++;
  }
}

void Matrix :: Throb(RenderFrame *r, byte tick) {
  // TODO how to best store state
  enum ThrobPhase { UP, DOWN };
  byte *phase = &r->state[0];

  // Decrease phase
  if (*phase == byte(UP)) {
    if (r->intensity > 0) {
      r->intensity--;
    } else {
      r->intensity = 0;
      *phase = byte(DOWN);
    }
  // Increase phase
  } else {
    if (r->intensity < 16) {
      r->intensity++;
    } else {
      r->intensity = 16;
      *phase = byte(UP);
    }
  }
}


