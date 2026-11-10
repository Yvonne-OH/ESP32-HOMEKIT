#include "SdCard.h"
#include "src/BoardPins.h"
#include <SD.h>
#include <SPI.h>

bool SdCard::begin(uint8_t csPin) {
  _cs = csPin;
  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, HIGH);            // 挂载前先拉高 CS，避免干扰 LCD
  _mounted = SD.begin(_cs);
  if (_mounted) {
    uint64_t sz = SD.cardSize();
    _sizeMB = (uint32_t)(sz / (1024ULL * 1024ULL));
  }
  return _mounted;
}

void SdCard::printInfo(Print &out) {
  if (!_mounted) {
    out.println("[SD] not mounted");
    return;
  }
  const char *type;
  switch (SD.cardType()) {
    case CARD_MMC:  type = "MMC";  break;
    case CARD_SD:   type = "SDSC"; break;
    case CARD_SDHC: type = "SDHC"; break;
    case CARD_NONE: type = "NONE"; break;
    default:        type = "UNKNOWN"; break;
  }
  out.printf("[SD] type=%s  size=%u MB  total=%llu MB\n",
             type, _sizeMB, (unsigned long long)(SD.totalBytes() / (1024ULL * 1024ULL)));
}

void SdCard::listDir(const char *path, int maxEntries, Print &out) {
  if (!_mounted) {
    out.println("[SD] not mounted");
    return;
  }
  File dir = SD.open(path);
  if (!dir) {
    out.printf("[SD] open %s failed\n", path);
    return;
  }
  out.printf("[SD] --- %s ---\n", path);
  int n = 0;
  File f;
  while ((f = dir.openNextFile()) && n < maxEntries) {
    out.printf("  %s %s (%u bytes)\n",
               f.isDirectory() ? "<DIR>" : "     ", f.name(), (unsigned)f.size());
    f.close();
    n++;
  }
  dir.close();
}
