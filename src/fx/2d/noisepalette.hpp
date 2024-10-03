/// @file    NoisePlusPalette.hpp
/// @brief   Demonstrates how to mix noise generation with color palettes on a
/// 2D LED matrix
/// @example NoisePlusPalette.hpp

#pragma once

#include <stdint.h>

#include "FastLED.h"
#include "fx/_fx2d.h"
#include "fx/_xymap.h"
#include "ptr.h"
#include "lib8tion/random8.h"
#include "noise.h"

#define MAX_DIMENSION 16

FASTLED_NAMESPACE_BEGIN

class NoisePalette : public FxGrid {
  public:
    NoisePalette(CRGB *leds, XYMap xyMap)
        : FxGrid(xyMap), leds(leds) {
        width = xyMap.getWidth();
        height = xyMap.getHeight();

        // Initialize our coordinates to some random values
        x = random16();
        y = random16();
        z = random16();

        // Allocate memory for the noise array
        noise = new uint8_t *[width];
        for (int i = 0; i < width; ++i) {
            noise[i] = new uint8_t[height];
        }
    }

    ~NoisePalette() {
        // Free the allocated memory
        for (int i = 0; i < width; ++i) {
            delete[] noise[i];
        }
        delete[] noise;
    }

    void lazyInit() override { this->mXyMap.optimizeAsLookupTable(); }

    void draw() override {
        fillnoise8();
        mapNoiseToLEDsUsingPalette();
        ChangePaletteAndSettingsPeriodically();
    }

    const char* fxName() const override {
        return "NoisePalette";
    }

    void mapNoiseToLEDsUsingPalette() {
        static uint8_t ihue = 0;

        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                // We use the value at the (i,j) coordinate in the noise
                // array for our brightness, and the flipped value from (j,i)
                // for our pixel's index into the color palette.

                uint8_t index = noise[j][i];
                uint8_t bri = noise[i][j];

                // if this palette is a 'loop', add a slowly-changing base value
                if (colorLoop) {
                    index += ihue;
                }

                // brighten up, as the color palette itself often contains the
                // light/dark dynamic range desired
                if (bri > 127) {
                    bri = 255;
                } else {
                    bri = dim8_raw(bri * 2);
                }

                CRGB color = ColorFromPalette(currentPalette, index, bri);
                leds[XY(i, j)] = color;
            }
        }

        ihue += 1;
    }

  private:
    CRGB *leds;
    uint16_t x, y, z;
    uint16_t width, height;
    uint16_t speed = 20;
    uint16_t scale = 30;
    uint8_t **noise;
    CRGBPalette16 currentPalette = PartyColors_p;
    uint8_t colorLoop = 1;

    void fillnoise8() {
    // If we're runing at a low "speed", some 8-bit artifacts become visible
    // from frame-to-frame.  In order to reduce this, we can do some fast
    // data-smoothing. The amount of data smoothing we're doing depends on
    // "speed".
    uint8_t dataSmoothing = 0;
    if (speed < 50) {
        dataSmoothing = 200 - (speed * 4);
    }

    for (int i = 0; i < MAX_DIMENSION; i++) {
        int ioffset = scale * i;
        for (int j = 0; j < MAX_DIMENSION; j++) {
            int joffset = scale * j;

            uint8_t data = inoise8(x + ioffset, y + joffset, z);

            // The range of the inoise8 function is roughly 16-238.
            // These two operations expand those values out to roughly 0..255
            // You can comment them out if you want the raw noise data.
            data = qsub8(data, 16);
            data = qadd8(data, scale8(data, 39));

            if (dataSmoothing) {
                uint8_t olddata = noise[i][j];
                uint8_t newdata = scale8(olddata, dataSmoothing) +
                                  scale8(data, 256 - dataSmoothing);
                data = newdata;
            }

            noise[i][j] = data;
        }
    }

    z += speed;

    // apply slow drift to X and Y, just for visual variation.
    x += speed / 8;
    y -= speed / 16;
}

    uint16_t XY(uint8_t x, uint8_t y) {
        return mXyMap.mapToIndex(x, y);
    }

    void ChangePaletteAndSettingsPeriodically() {
    static const uint8_t HOLD_PALETTES_X_TIMES_AS_LONG = 5; // You can adjust this value as needed
    uint8_t secondHand =
        ((millis() / 1000) / HOLD_PALETTES_X_TIMES_AS_LONG) % 60;
    static uint8_t lastSecond = 99;

    if (lastSecond != secondHand) {
        lastSecond = secondHand;
        if (secondHand == 0) {
            currentPalette = RainbowColors_p;
            speed = 20;
            scale = 30;
            colorLoop = 1;
        }
        if (secondHand == 5) {
            SetupPurpleAndGreenPalette();
            speed = 10;
            scale = 50;
            colorLoop = 1;
        }
        if (secondHand == 10) {
            SetupBlackAndWhiteStripedPalette();
            speed = 20;
            scale = 30;
            colorLoop = 1;
        }
        if (secondHand == 15) {
            currentPalette = ForestColors_p;
            speed = 8;
            scale = 120;
            colorLoop = 0;
        }
        if (secondHand == 20) {
            currentPalette = CloudColors_p;
            speed = 4;
            scale = 30;
            colorLoop = 0;
        }
        if (secondHand == 25) {
            currentPalette = LavaColors_p;
            speed = 8;
            scale = 50;
            colorLoop = 0;
        }
        if (secondHand == 30) {
            currentPalette = OceanColors_p;
            speed = 20;
            scale = 90;
            colorLoop = 0;
        }
        if (secondHand == 35) {
            currentPalette = PartyColors_p;
            speed = 20;
            scale = 30;
            colorLoop = 1;
        }
        if (secondHand == 40) {
            SetupRandomPalette();
            speed = 20;
            scale = 20;
            colorLoop = 1;
        }
        if (secondHand == 45) {
            SetupRandomPalette();
            speed = 50;
            scale = 50;
            colorLoop = 1;
        }
        if (secondHand == 50) {
            SetupRandomPalette();
            speed = 90;
            scale = 90;
            colorLoop = 1;
        }
        if (secondHand == 55) {
            currentPalette = RainbowStripeColors_p;
            speed = 30;
            scale = 20;
            colorLoop = 1;
        }
    }
}

    void SetupRandomPalette() {
        currentPalette =
            CRGBPalette16(CHSV(random8(), 255, 32), CHSV(random8(), 255, 255),
                          CHSV(random8(), 128, 255), CHSV(random8(), 255, 255));
    }

    void SetupBlackAndWhiteStripedPalette() {
        fill_solid(currentPalette, 16, CRGB::Black);
        currentPalette[0] = CRGB::White;
        currentPalette[4] = CRGB::White;
        currentPalette[8] = CRGB::White;
        currentPalette[12] = CRGB::White;
    }

    void SetupPurpleAndGreenPalette() {
        CRGB purple = CHSV(HUE_PURPLE, 255, 255);
        CRGB green = CHSV(HUE_GREEN, 255, 255);
        CRGB black = CRGB::Black;

        currentPalette =
            CRGBPalette16(green, green, black, black, purple, purple, black, black,
                          green, green, black, black, purple, purple, black, black);
    }
};

FASTLED_NAMESPACE_END
