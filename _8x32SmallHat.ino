#define FASTLED_INTERRUPT_RETRY_COUNT 1
#define FASTLED_ALLOW_INTERRUPTS 0

#include <FastLED.h>
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

void DrawPresent(cLEDMatrixBase & matrix, int x, int y, int w, int h, CRGB color)
{
  matrix.DrawFilledRectangle(x, y, x+w-1, y+h-1, color);

  if (w & 1) {
    int m = x + (w/2);
    matrix.DrawLine(m, y, m, y+h-1, CRGB::Red);
  }
}

void loop()
{
  ClearMatrix(leds);

  DrawPresent(leds, 10, 0, 5, 4, CRGB::Blue);
  AddSnow();

  FastLED.show();
  delay(80);
}
