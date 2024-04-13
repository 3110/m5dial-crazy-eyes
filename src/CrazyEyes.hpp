#pragma once

// clang-format off
#include <LittleFS.h>
#define IV_FS LittleFS
#include <M5Unified.h>
// clang-format on

class CrazyEyes {
public:
    static const char* VERSION;

    static const size_t MAX_IMAGE_FILES = 2;

    static const uint8_t LEFT_EYE_ORIENTATION = 1;
    static const uint8_t RIGHT_EYE_ORIENTATION = 3;

    static const char* OPEN_EYE_IMAGE_FILE;
    static const char* CLOSE_EYE_IMAGE_FILE;

    static const uint32_t START_INTERVAL_MS = 5000;
    static const uint32_t AUTO_INTERVAL_MS = 100;

    CrazyEyes(void);
    virtual ~CrazyEyes(void);

    virtual bool begin(const int bgColor = TFT_WHITE);
    virtual bool update(void);

protected:
    virtual uint8_t toggleOrientation(uint8_t orientation) const;
    virtual void showImage(const String images[], size_t p);
    virtual bool hasExt(const char* filename, const char* ext) const;
    virtual bool isJpeg(const char* filename) const;
    virtual bool isPng(const char* filename) const;
    virtual bool isBmp(const char* filename) const;

private:
    String _imageFiles[MAX_IMAGE_FILES];
    size_t _pos;
    uint32_t _prevUpdate;
    bool _isAutoMode;
    uint32_t _interval;
};