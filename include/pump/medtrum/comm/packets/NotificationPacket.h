#ifndef NOTIFICATION_PACKET_H
#define NOTIFICATION_PACKET_H

#include <stdint.h>
#include <stddef.h>
#include <map>

class MedtrumPumpSync;

class NotificationPacket {
public:
    NotificationPacket(MedtrumPumpSync &pumpSync);

    void onNotification(const uint8_t* data, size_t dataSize);
    bool handleMaskedMessage(const uint8_t* data, size_t dataSize);

private:

    size_t calculateExpectedLengthBasedOnFieldMask(int fieldMask);
    size_t calculateOffset(uint16_t fieldMask, uint16_t targetMask);
    bool checkDataValidity(uint16_t fieldMask, const uint8_t* data, size_t dataSize);

    size_t handleSuspend(const uint8_t* data);
    size_t handleNormalBolus(const uint8_t* data);
    size_t handleExtendedBolus(const uint8_t* data);
    size_t handleBasal(const uint8_t* data);
    size_t handleSetup(const uint8_t* data);
    size_t handleReservoir(const uint8_t* data);
    size_t handleStartTime(const uint8_t* data);
    size_t handleBattery(const uint8_t* data);
    size_t handleStorage(const uint8_t* data);
    size_t handleAlarm(const uint8_t* data);
    size_t handleAge(const uint8_t* data);
    size_t handleUnknown1(const uint8_t* data);
    size_t handleUnusedCGM(const uint8_t* data);
    size_t handleUnusedCommandConfirm(const uint8_t* data);
    size_t handleUnusedAutoStatus(const uint8_t* data);
    size_t handleUnusedLegacy(const uint8_t* data);

    using HandlerFunc = size_t (NotificationPacket::*)(const uint8_t *);
    std::map<uint16_t, HandlerFunc> mMaskHandlers;
    std::map<uint16_t, size_t> mSizeMap;

    MedtrumPumpSync &mPumpSync;
};

#endif // NOTIFICATION_PACKET_H
