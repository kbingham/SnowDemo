
/*
 * ESP8266 has too much flicker with Interrupts enabled
 */
#define FASTLED_INTERRUPT_RETRY_COUNT 1
#define FASTLED_ALLOW_INTERRUPTS 0

#include <FastLED.h>

// https://github.com/AaronLiddiment/LEDMatrix
#include <LEDMatrix.h>

// Change the next 6 defines to match your matrix type and size

#define LED_PIN        4
#define COLOR_ORDER    GRB
#define CHIPSET        WS2812B

#define MATRIX_WIDTH   32  // Set this negative if physical led 0 is opposite to where you want logical 0
#define MATRIX_HEIGHT  8   // Set this negative if physical led 0 is opposite to where you want logical 0
#define MATRIX_TYPE    VERTICAL_ZIGZAG_MATRIX  // See top of LEDMatrix.h for matrix wiring types

cLEDMatrix<-MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> leds;
cLEDMatrix<-MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> snowpixels;

/*
 * The meltline lets the snow settle on the ground, and remembers
 * snow pixels as they 'melts' away
 */
CRGB meltline[MATRIX_WIDTH];


void setup()
{
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds[0], leds.Size());
  FastLED.setBrightness(128);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 150);
  FastLED.clear(true);
  delay(500);
  FastLED.clear(true);

}

/*
 * LEDMatrix doesn't offer a functionality to set the whole frame.
 * Provide a couple of local helpers to ease.
 * If we use FastLED.clear() we get flicker as the black frame is written out.
 */

void SetMatrix(cLEDMatrixBase & matrix, CRGB colour)
{
  int i;

  for (i=0; i < matrix.Size(); i++)
    matrix(i) = CRGB::Black;
}

void ClearMatrix(cLEDMatrixBase & matrix)
{
  SetMatrix(matrix, CRGB::Black);
}

void AddSnow()
{
  int x = 0;
  int y = 0;
  static const fract8 chanceOfSnow = 10;

  /* Save the MeltLine */
  for (x=0; x < MATRIX_WIDTH; x++)
    meltline[x] = snowpixels(x, 0).fadeToBlackBy(64);

  /* Let the snow fall */
  snowpixels.ShiftDown();

  /* Add Snow to top line */
  for (x=0; x < MATRIX_WIDTH; x++) {
    CRGB s = (random8() < chanceOfSnow) ? CRGB::White : CRGB::Black;
    snowpixels(x, MATRIX_HEIGHT - 1) = s;

    /* Maintain melting snow on the ground */
    snowpixels(x, 0) += meltline[x];
  }

  /* Copy over pixels to target */
  // Could be optimised as :
  // for (x=0; x < leds.Size(); x++) leds(x) += snowpixels(x);
  for (x=0; x < MATRIX_WIDTH; x++) {
    for (y=0; y < MATRIX_HEIGHT; y++) {
      leds(x, y) += snowpixels(x, y);
    }
  }
}

struct Present {
  /* Size */
  int width;
  int height;

  /* Style */
  CRGB wrap;
  CRGB ribbon;

  /* Speed */
  int bpm;

  /* Position */
  int x;
  int y;
};

struct Present presents[] = {
  { 3, 3, CRGB::Purple, CRGB::Yellow, 9, 0, 0},
  { 5, 3, CRGB::Blue,   CRGB::Red,    7, 0, 0},
  { 3, 2, CRGB::Green,  CRGB::Blue,   5, 0, 0},
};

void DrawPresent(cLEDMatrixBase & matrix, struct Present *p)
{
  matrix.DrawFilledRectangle(p->x, p->y, p->x + p->width-1, p->y + p->height-1, p->wrap);

  if (p->width & 1) {
    int m = p->x + (p->width/2); /* Middle */
    int top = p->y + p->height;

    matrix.DrawLine(m, p->y,  m, top-1, p->ribbon); /* Vertical line */
    matrix.DrawLine(m, top, m-1, top+1, p->ribbon); /* Left Bow */
    matrix.DrawLine(m, top, m+1, top+1, p->ribbon); /* Right Bow */
  }
}

void DrawMovingPresent(cLEDMatrixBase &matrix, int l, int r, struct Present *p)
{
  p->x = beatsin8(p->bpm, l, r - p->width);
  p->y = 0;

  DrawPresent(leds, p);
}

void DrawXmasTree(cLEDMatrixBase &matrix, int m)
{
  int hue = 0;

  /* Base */
  matrix.DrawFilledRectangle(m-1, 0, m+1, 2, CRGB::SaddleBrown);

  /* Draw the Tree */
  matrix.DrawFilledRectangle(m-3, 2, m+3, 2, CRGB::Green);
  matrix.DrawFilledRectangle(m-2, 3, m+2, 4, CRGB::Green);
  matrix.DrawFilledRectangle(m-1, 5, m+1, 6, CRGB::Green);

  /* With a star on top, of course */
  matrix(m, 7) = CRGB::Yellow;

  /* Add the sparkle */
  matrix(m-2, 2) = CHSV( hue + random8(64), 200, 255);
  matrix(m+3, 2) = CHSV( hue + random8(64), 200, 255);
  matrix(m,   3) = CHSV( hue + random8(64), 200, 255);
  matrix(m-2, 4) = CHSV( hue + random8(64), 200, 255);
  matrix(m+1, 5) = CHSV( hue + random8(64), 200, 255);
  matrix(m-1, 6) = CHSV( hue + random8(64), 200, 255);
}

void loop()
{
  ClearMatrix(leds);

  DrawMovingPresent(leds, 0,  13, &presents[0]);
  DrawMovingPresent(leds, 20, 32, &presents[1]);

  DrawXmasTree(leds, 16);

  AddSnow();

  FastLED.show();
  delay(80);
}
