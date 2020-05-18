#include "LedControl.h"

struct MatrixPinInterface {
  int digitalIn;
  int clock;
  int chipSelect;
};


enum DisplayOrientation { North, South, East, West };

enum FrameTransforms { ShiftDown, RollDown };

struct RenderFrame {
  byte lightMatrix[8];
  int intensity;
  bool inverted;
  DisplayOrientation orientation;
  byte state[32];
};

class Matrix {

  private:
    byte tick;
    int lastRenderTime;
    int renderInterval;
    LedControl ledMatrix;
    void (*frameTransformer)(RenderFrame*, byte);

  public:

    RenderFrame currentFrame;
    RenderFrame nextFrame;

    Matrix(MatrixPinInterface pinInterface);

    void setNextFrame(RenderFrame);
    
    void renderNextFrame();

    void setFrameTransformer(void (*frameTransformer)(RenderFrame*, byte));

    void setRenderInterval(int newInterval);

    void tapRenderLoop();

    void clearLightMatrix();

    static void SlideDown(RenderFrame*, byte);
    static void SlideUp(RenderFrame*, byte);
    static void SlideRight(RenderFrame*, byte);
    static void SlideLeft(RenderFrame*, byte);
    static void Interlace(RenderFrame*, byte);
    static void FlickerIn(RenderFrame*, byte);
    static void GlitchIn(RenderFrame*, byte);
    static void CollapseOut(RenderFrame*, byte);
    static void Throb(RenderFrame*, byte);
    static void Rotate(RenderFrame*, byte);
    static void Blink(RenderFrame*, byte);
    static void Flash(RenderFrame*, byte);
};
