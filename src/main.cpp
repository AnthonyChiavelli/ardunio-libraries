
#include "Matrix.h"


MatrixPinInterface p = {
  12, 10, 11
};
  
Matrix m = Matrix(p);

byte heart[8] = {
  B00000000,
  B01100110,
  B11111111,
  B11111111,
  B01111110,
  B00111100,
  B00011000,
  B00000000,
};

RenderFrame initialFrame;

void setup(){
  // TODO how to do declaratively?
  initialFrame.intensity = 5;
  initialFrame.inverted = false;
  initialFrame.orientation = East;
  for (int x = 0; x < 8; x++) {
    initialFrame.lightMatrix[x] = heart[x];
  }
  m.setNextFrame(initialFrame);
  m.setRenderInterval(20);

  // m.setFrameTransformer(Matrix::Throb);
  // m.setFrameTransformer(Matrix::SlideUp);
  // m.setFrameTransformer(Matrix::SlideDown);
  // m.setFrameTransformer(Matrix::Interlace);
  // m.setFrameTransformer(Matrix::FlickerIn);
  // m.setFrameTransformer(Matrix::GlitchIn);
  m.setFrameTransformer(Matrix::CollapseOut);
  // m.setFrameTransformer(Matrix::Rotate);
  // m.setFrameTransformer(Matrix::Blink);
  // m.setFrameTransformer(Matrix::Flash);

  // TODO add ability to define animation queue and provide pointer to class to advance through
  // TODO use this for transition-type animations like glitchIN, or find a different way to express these
  // perhaps an abnility to run an animation until it's transform returns false or indicates it's done?
  // and maybe a way to define animations that are only functions of time (sine wave)
}

void loop() { 
  m.tapRenderLoop();
  delay(1);
}


