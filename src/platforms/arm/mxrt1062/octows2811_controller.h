#ifndef __INC_OCTOWS2811_CONTROLLER_H
#define __INC_OCTOWS2811_CONTROLLER_H

#ifdef USE_OCTOWS2811

// #include "OctoWS2811.h"

FASTLED_NAMESPACE_BEGIN

template<EOrder RGB_ORDER = GRB, uint8_t CHIP = WS2811_800kHz>
class COctoWS2811Controller : public CPixelLEDController<RGB_ORDER, 8, 0xFF> {
  OctoWS2811 *pocto = nullptr;
  uint8_t *drawbuffer = nullptr;
  uint8_t *framebuffer = nullptr;

  bool tryToAllocate = true;

  void _init(int nLeds) {
    if (pocto == nullptr && tryToAllocate) {
      // Allocate the draw buffer second in case the malloc fails,
      // because it can technically be NULL. In other words,
      // prioritize allocating the frame buffer.
      framebuffer = static_cast<uint8_t *>(malloc(nLeds * 8 * 3));

      if (framebuffer != nullptr) {
        drawbuffer = static_cast<uint8_t *>(malloc(nLeds * 8 * 3));
        // Note: A NULL draw buffer is okay

        // byte ordering is handled in show by the pixel controller
        int config = WS2811_RGB;
        config |= CHIP;

        pocto = new OctoWS2811(nLeds, framebuffer, drawbuffer, config);

        if (pocto != nullptr) {
          pocto->begin();
        } else {
          // Don't try to allocate again
          tryToAllocate = false;
        }
      } else {
        // Don't try to allocate again
        tryToAllocate = false;
      }
    }
  }
public:
  COctoWS2811Controller() = default;

  virtual int size() { return CLEDController::size() * 8; }

  virtual void init() { /* do nothing yet */ }

  virtual void showPixels(PixelController<RGB_ORDER, 8, 0xFF> &pixels) {
    uint32_t size = pixels.size();
    uint32_t sizeTimes8 = 8U * size;
    _init(size);
    if (pocto == nullptr) {
      return;
    }

    uint32_t index = 0;
    while (pixels.has(1)) {
      for (int lane = 0; lane < 8; lane++) {
        uint8_t r = pixels.loadAndScale0(lane);
        uint8_t g = pixels.loadAndScale1(lane);
        uint8_t b = pixels.loadAndScale2(lane);
        pocto->setPixel(index, r, g, b);
        index += size;
      }
      index -= sizeTimes8;
      index++;
      pixels.stepDithering();
      pixels.advanceData();
    }

    pocto->show();
  }

};

FASTLED_NAMESPACE_END

#endif

#endif
