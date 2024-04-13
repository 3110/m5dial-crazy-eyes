// clang-format off
#include <string.h>
#include "CrazyEyes.hpp"
#include "M5DialEncoder.hpp"
// clang-format on

const char* CrazyEyes::VERSION = "v0.0.1";

const char* CrazyEyes::OPEN_EYE_IMAGE_FILE = "/crazy-eyes-open.jpg";
const char* CrazyEyes::CLOSE_EYE_IMAGE_FILE = "/crazy-eyes-close.jpg";

static const int32_t TEXT_AREA_X = 35;
static const int32_t TEXT_AREA_Y = 35;
static const int32_t TEXT_AREA_WIDTH = 170;
static const int32_t TEXT_AREA_HEIGHT = 170;

static const bool FORMAT_FS_IF_FAILED = true;
static const char* EXT_JPG = ".jpg";
static const char* EXT_JPEG = ".jpeg";
static const char* EXT_BMP = ".bmp";
static const char* EXT_PNG = ".png";

static M5DialEncoder encoder;
static int16_t prev_dial_pos = 0;

inline void M5_BEGIN(m5::M5Unified::config_t cfg) {
    M5.begin(cfg);
    encoder.begin();
}

inline void M5_BEGIN(void) {
    auto cfg = M5.config();
    M5_BEGIN(cfg);
}

inline void M5_UPDATE(void) {
    M5.update();
}

inline int16_t getDirection(void) {
    const int16_t pos = encoder.read();
    M5_LOGV("Dial: %d -> %d", prev_dial_pos, pos);
    if (abs(prev_dial_pos - pos) >= 4) {
        const int16_t direction = pos - prev_dial_pos > 0 ? 1 : -1;
        prev_dial_pos = pos;
        return direction;
    } else {
        return 0;
    }
}

CrazyEyes::CrazyEyes(void)
    : _imageFiles{OPEN_EYE_IMAGE_FILE, CLOSE_EYE_IMAGE_FILE},
      _pos(0),
      _prevUpdate(0),
      _isAutoMode(false),
      _interval(AUTO_INTERVAL_MS) {
}

CrazyEyes::~CrazyEyes(void) {
}

bool CrazyEyes::begin(int bgColor) {
    M5_BEGIN();

#if defined(CRAZY_EYES_ORIENTATION)
    M5.Lcd.setRotation(CRAZY_EYES_ORIENTATION);
#else
    M5.Lcd.setRotation(0);
#endif

    M5.Lcd.setTextScroll(true);
    M5.Lcd.setCursor(TEXT_AREA_X, TEXT_AREA_Y);
    M5.Lcd.setScrollRect(TEXT_AREA_X, TEXT_AREA_Y, TEXT_AREA_WIDTH,
                         TEXT_AREA_HEIGHT);

    if (!IV_FS.begin(FORMAT_FS_IF_FAILED)) {
        M5.Lcd.println("Failed to mount File System");
        return false;
    }
    M5.Lcd.setFileStorage(IV_FS);

    M5.Lcd.printf("Crazy Eyes %s", VERSION);
    M5.Lcd.println();

    M5.Lcd.println("Config:");
    M5.Lcd.printf(" AutoMode: %s", this->_isAutoMode ? "true" : "false");
    M5.Lcd.println();
    M5.Lcd.printf(" Interval: %dms", this->_interval);
    M5.Lcd.println();

    M5.Lcd.println("Mode:");
    M5_UPDATE();
    if (M5.BtnA.isPressed()) {
        this->_isAutoMode = true;
    }
    M5.Lcd.printf(" %s", this->_isAutoMode ? "Auto" : "Manual");
    M5.Lcd.println();

    M5.Lcd.println("Orientation:");
#if defined(CRAZY_EYES_ORIENTATION)
    M5.Lcd.printf(" %s Eye", CRAZY_EYES_ORIENTATION == RIGHT_EYE_ORIENTATION
                                 ? "Right"
                                 : "Left");
#else
    M5.Lcd.printf(" %s", "Manual");
#endif
    M5.Lcd.println();
    delay(START_INTERVAL_MS);

    M5.Lcd.clearScrollRect();
    M5.Lcd.setCursor(0, 0);

    M5.Lcd.clear();
    M5.Lcd.fillScreen(bgColor);

#if !defined(CRAZY_EYES_ORIENTATION)
    M5.Lcd.setRotation(RIGHT_EYE_ORIENTATION);
#endif

    if (!this->_isAutoMode) {
        showImage(this->_imageFiles, this->_pos);
    }

    return true;
}

bool CrazyEyes::update(void) {
    M5_UPDATE();

    if (M5.BtnA.wasClicked()) {
#if defined(CRAZY_EYES_ORIENTATION)
        this->_isAutoMode = !this->_isAutoMode;
#else
        M5.Lcd.setRotation(toggleOrientation(M5.Lcd.getRotation()));
        showImage(this->_imageFiles, this->_pos);
#endif
    }

    const uint32_t t = millis();
    int32_t direction = getDirection();
    if (direction == 0 && this->_isAutoMode &&
        t - this->_prevUpdate >= this->_interval) {
        direction = 1;
    }
    if (direction != 0) {
        this->_prevUpdate = t;
        if (direction < 0 && this->_pos == 0) {
            this->_pos = MAX_IMAGE_FILES - 1;
        } else if (direction > 0 && this->_pos == MAX_IMAGE_FILES - 1) {
            this->_pos = 0;
        } else {
            this->_pos += direction;
        }
        showImage(this->_imageFiles, this->_pos);
    }
    return direction != 0;
}

uint8_t CrazyEyes::toggleOrientation(uint8_t orientation) const {
    return orientation == RIGHT_EYE_ORIENTATION ? LEFT_EYE_ORIENTATION
                                                : RIGHT_EYE_ORIENTATION;
}

void CrazyEyes::showImage(const String images[], size_t p) {
    const char* filename = images[p].c_str();
    M5.Lcd.startWrite();
    if (isJpeg(filename)) {
        M5.Lcd.drawJpgFile(filename, 0, 0, M5.Display.width(),
                           M5.Display.height(), 0, 0, 0.0F, 0.0F,
                           middle_center);
    } else if (isPng(filename)) {
        M5.Lcd.drawPngFile(filename, 0, 0, M5.Display.width(),
                           M5.Display.height(), 0, 0, 0.0F, 0.0F,
                           middle_center);
    } else if (isBmp(filename)) {
        M5.Lcd.drawBmpFile(filename, 0, 0, M5.Display.width(),
                           M5.Display.height(), 0, 0, 0.0F, 0.0F,
                           middle_center);
    } else {
        M5.Lcd.printf("ignore: %s", filename);
        M5.Lcd.println();
    }
    M5.Lcd.endWrite();
}

bool CrazyEyes::hasExt(const char* filename, const char* ext) const {
    if (filename == nullptr) {
        return false;
    }
    if (ext == nullptr) {
        return false;
    }
    const char* p = strrchr(filename, '.');
    return p != nullptr && strcasecmp(ext, p) == 0;
}

bool CrazyEyes::isJpeg(const char* filename) const {
    if (filename == nullptr) {
        return false;
    }
    return hasExt(filename, EXT_JPG) || hasExt(filename, EXT_JPEG);
}

bool CrazyEyes::isPng(const char* filename) const {
    if (filename == nullptr) {
        return false;
    }
    return hasExt(filename, EXT_PNG);
}

bool CrazyEyes::isBmp(const char* filename) const {
    if (filename == nullptr) {
        return false;
    }
    return hasExt(filename, EXT_BMP);
}
