#include <pump/medtrum/MedtrumPumpSync.h>
#include <pump/medtrum/comm/packets/NotificationPacket.h>

#include <zephyr/sys/byteorder.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(NotificationPacket);

namespace
{
    constexpr uint16_t NOTIF_STATE_START = 0;
    constexpr uint16_t NOTIF_STATE_END = NOTIF_STATE_START + 1;

    constexpr uint16_t MASK_SUSPEND = 0x01;
    constexpr uint16_t MASK_NORMAL_BOLUS = 0x02;
    constexpr uint16_t MASK_EXTENDED_BOLUS = 0x04;
    constexpr uint16_t MASK_BASAL = 0x08;

    constexpr uint16_t MASK_SETUP = 0x10;
    constexpr uint16_t MASK_RESERVOIR = 0x20;
    constexpr uint16_t MASK_START_TIME = 0x40;
    constexpr uint16_t MASK_BATTERY = 0x80;

    constexpr uint16_t MASK_STORAGE = 0x100;
    constexpr uint16_t MASK_ALARM = 0x200;
    constexpr uint16_t MASK_AGE = 0x400;
    constexpr uint16_t MASK_MAGNETO_PLACE = 0x800;

    constexpr uint16_t MASK_UNUSED_CGM = 0x1000;
    constexpr uint16_t MASK_UNUSED_COMMAND_CONFIRM = 0x2000;
    constexpr uint16_t MASK_UNUSED_AUTO_STATUS = 0x4000;
    constexpr uint16_t MASK_UNUSED_LEGACY = 0x8000;

    constexpr size_t SIZE_FIELD_MASK = 2;
    constexpr size_t SIZE_SUSPEND = 4;
    constexpr size_t SIZE_NORMAL_BOLUS = 3;
    constexpr size_t SIZE_EXTENDED_BOLUS = 3;
    constexpr size_t SIZE_BASAL = 12;
    constexpr size_t SIZE_SETUP = 1;
    constexpr size_t SIZE_RESERVOIR = 2;
    constexpr size_t SIZE_START_TIME = 4;
    constexpr size_t SIZE_BATTERY = 3;
    constexpr size_t SIZE_STORAGE = 4;
    constexpr size_t SIZE_ALARM = 4;
    constexpr size_t SIZE_AGE = 4;
    constexpr size_t SIZE_MAGNETO_PLACE = 2;
    constexpr size_t SIZE_UNUSED_CGM = 5;
    constexpr size_t SIZE_UNUSED_COMMAND_CONFIRM = 2;
    constexpr size_t SIZE_UNUSED_AUTO_STATUS = 2;
    constexpr size_t SIZE_UNUSED_LEGACY = 2;
}

NotificationPacket::NotificationPacket(MedtrumPumpSync &pumpSync) : mPumpSync(pumpSync)
{
    mMaskHandlers = {{MASK_SUSPEND, &NotificationPacket::handleSuspend},
                     {MASK_NORMAL_BOLUS, &NotificationPacket::handleNormalBolus},
                     {MASK_EXTENDED_BOLUS, &NotificationPacket::handleExtendedBolus},
                     {MASK_BASAL, &NotificationPacket::handleBasal},
                     {MASK_SETUP, &NotificationPacket::handleSetup},
                     {MASK_RESERVOIR, &NotificationPacket::handleReservoir},
                     {MASK_START_TIME, &NotificationPacket::handleStartTime},
                     {MASK_BATTERY, &NotificationPacket::handleBattery},
                     {MASK_STORAGE, &NotificationPacket::handleStorage},
                     {MASK_ALARM, &NotificationPacket::handleAlarm},
                     {MASK_AGE, &NotificationPacket::handleAge},
                     {MASK_MAGNETO_PLACE, &NotificationPacket::handleUnknown1},
                     {MASK_UNUSED_CGM, &NotificationPacket::handleUnusedCGM},
                     {MASK_UNUSED_COMMAND_CONFIRM, &NotificationPacket::handleUnusedCommandConfirm},
                     {MASK_UNUSED_AUTO_STATUS, &NotificationPacket::handleUnusedAutoStatus},
                     {MASK_UNUSED_LEGACY, &NotificationPacket::handleUnusedLegacy}};

    mSizeMap = {{MASK_SUSPEND, SIZE_SUSPEND},
                {MASK_NORMAL_BOLUS, SIZE_NORMAL_BOLUS},
                {MASK_EXTENDED_BOLUS, SIZE_EXTENDED_BOLUS},
                {MASK_BASAL, SIZE_BASAL},
                {MASK_SETUP, SIZE_SETUP},
                {MASK_RESERVOIR, SIZE_RESERVOIR},
                {MASK_START_TIME, SIZE_START_TIME},
                {MASK_BATTERY, SIZE_BATTERY},
                {MASK_STORAGE, SIZE_STORAGE},
                {MASK_ALARM, SIZE_ALARM},
                {MASK_AGE, SIZE_AGE},
                {MASK_MAGNETO_PLACE, SIZE_MAGNETO_PLACE},
                {MASK_UNUSED_CGM, SIZE_UNUSED_CGM},
                {MASK_UNUSED_COMMAND_CONFIRM, SIZE_UNUSED_COMMAND_CONFIRM},
                {MASK_UNUSED_AUTO_STATUS, SIZE_UNUSED_AUTO_STATUS},
                {MASK_UNUSED_LEGACY, SIZE_UNUSED_LEGACY}};
}

void NotificationPacket::onNotification(const uint8_t *data, size_t dataSize)
{
    PumpState state = data[NOTIF_STATE_START];
    LOG_DBG("Notification state: %d", static_cast<uint8_t>(state));

    if (mPumpSync.getPumpState() != state)
    {
        mPumpSync.setPumpState(state);
    }

    if (dataSize > NOTIF_STATE_END + SIZE_FIELD_MASK)
    {
        (void)handleMaskedMessage(data + NOTIF_STATE_END, dataSize - NOTIF_STATE_END);
    }
}

bool NotificationPacket::handleMaskedMessage(const uint8_t *data, size_t dataSize)
{
    int fieldMask = sys_get_le16(data);

    size_t expectedLength = calculateExpectedLengthBasedOnFieldMask(fieldMask);
    if (dataSize < expectedLength)
    {
        LOG_ERR("Incorrect message length. Expected at least %d bytes. but got: %d", expectedLength, dataSize);
        return false;
    }

    if (!checkDataValidity(fieldMask, data, dataSize))
    {
        LOG_ERR("Invalid data in message");
        return false;
    }

    LOG_DBG("Message field mask: %d", fieldMask);

    // Handle the data
    size_t offset = 2;
    for (const auto &handler : mMaskHandlers)
    {
        if (fieldMask & handler.first)
        {

            offset += (this->*handler.second)(data + offset);
        }
    }
    return true;
}

size_t NotificationPacket::calculateExpectedLengthBasedOnFieldMask(int fieldMask)
{
    size_t length = SIZE_FIELD_MASK;
    for (const auto &size : mSizeMap)
    {
        if (fieldMask & size.first)
        {
            length += size.second;
        }
    }
    return length;
}

size_t NotificationPacket::calculateOffset(uint16_t fieldMask, uint16_t targetMask)
{
    size_t offset = SIZE_FIELD_MASK;
    for (const auto &size : mSizeMap)
    {
        if (size.first == targetMask)
        {
            break;
        }
        if (fieldMask & size.first)
        {
            offset += size.second;
        }
    }
    return offset;
}

bool NotificationPacket::checkDataValidity(uint16_t fieldMask, const uint8_t *data, size_t dataSize)
{
    // Notification packet does not have crc check, so we check validity based on expected values in the packet
    if (fieldMask & MASK_NORMAL_BOLUS)
    {
        size_t offset = calculateOffset(fieldMask, MASK_NORMAL_BOLUS);
        int16_t bolusDelivered = sys_get_le16(data + offset + 1) * 0.05;
        if (bolusDelivered < 0 || bolusDelivered > 50)
        {
            LOG_ERR("Invalid bolus delivered: %d", bolusDelivered);
            return false;
        }
    }

    if (fieldMask & MASK_BASAL)
    {
        size_t offset = calculateOffset(fieldMask, MASK_BASAL);
        uint16_t basalPatchId = sys_get_le16(data + offset + 3);
        uint32_t basalRateAndDelivery = sys_get_le32(data + offset + 9);
        double basalRate = (basalRateAndDelivery & 0xFFF) * 0.05;
        // if (medtrumPump.patchId != 0 && basalPatchId != medtrumPump.patchId)
        // {
        //     LOG_ERR("Mismatched patch ID: %lu vs stored patchID: %lu", basalPatchId, medtrumPump.patchId);
        //     return false;
        // }
        if (basalRate < 0 || basalRate > 40)
        {
            LOG_ERR("Invalid basal rate: %f", basalRate);
            return false;
        }
    }

    if (fieldMask & MASK_RESERVOIR)
    {
        size_t offset = calculateOffset(fieldMask, MASK_RESERVOIR);
        float reservoirValue = sys_get_le16(data + offset) * 0.05;
        if (reservoirValue < 0 || reservoirValue > 400)
        {
            LOG_ERR("Invalid reservoir value: %f", static_cast<double>(reservoirValue));
            return false;
        }
    }

    if (fieldMask & MASK_STORAGE)
    {
        size_t offset = calculateOffset(fieldMask, MASK_STORAGE);
        uint16_t patchId = sys_get_le16(data + offset + 2);
        // if (medtrumPump.patchId != 0 && patchId != medtrumPump.patchId)
        // {
        //     LOG_ERR("Mismatched patch ID: %lu vs stored patchID: %lu", patchId, medtrumPump.patchId);
        //     return false;
        // }
    }
    return true;
}

size_t NotificationPacket::handleSuspend(const uint8_t *data)
{
    LOG_DBG("Suspend data received");
    uint32_t suspendTime = sys_get_le32(data); // TODO: Convert to time

    return SIZE_SUSPEND;
}

size_t NotificationPacket::handleNormalBolus(const uint8_t *data)
{
    LOG_DBG("Normal bolus data received");
    // TODO: Handle normal bolus
    return SIZE_NORMAL_BOLUS;
}

size_t NotificationPacket::handleExtendedBolus(const uint8_t *data)
{
    LOG_DBG("Extended bolus data received");
    // TODO: Handle extended bolus
    return SIZE_EXTENDED_BOLUS;
}

size_t NotificationPacket::handleBasal(const uint8_t *data)
{
    LOG_DBG("Basal data received");
    // TODO: Handle basal
    return SIZE_BASAL;
}

size_t NotificationPacket::handleSetup(const uint8_t *data)
{
    LOG_DBG("Setup data received");
    // TODO: Handle setup
    return SIZE_SETUP;
}

size_t NotificationPacket::handleReservoir(const uint8_t *data)
{
    LOG_DBG("Reservoir data received");
    // TODO: Handle reservoir
    float reservoirValue = sys_get_le16(data) * 0.05;
    LOG_DBG("Reservoir value: %f", static_cast<double>(reservoirValue));
    mPumpSync.setReservoirLevel(reservoirValue);
    return SIZE_RESERVOIR;
}

size_t NotificationPacket::handleStartTime(const uint8_t *data)
{
    LOG_DBG("Start time data received");
    // TODO
    return SIZE_START_TIME;
}

size_t NotificationPacket::handleBattery(const uint8_t *data)
{
    LOG_DBG("Battery data received");
    // TODO
    return SIZE_BATTERY;
}

size_t NotificationPacket::handleStorage(const uint8_t *data)
{
    LOG_DBG("Storage data received");
    // TODO
    return SIZE_STORAGE;
}

size_t NotificationPacket::handleAlarm(const uint8_t *data)
{
    LOG_DBG("Alarm data received");
    // TODO
    return SIZE_ALARM;
}

size_t NotificationPacket::handleAge(const uint8_t *data)
{
    LOG_DBG("Age data received");
    // TODO
    return SIZE_AGE;
}

size_t NotificationPacket::handleUnknown1(const uint8_t *data)
{
    LOG_WRN("Unknown1 data received");
    return SIZE_MAGNETO_PLACE;
}

size_t NotificationPacket::handleUnusedCGM(const uint8_t *data)
{
    LOG_WRN("Unused CGM data received");
    return SIZE_UNUSED_CGM;
}

size_t NotificationPacket::handleUnusedCommandConfirm(const uint8_t *data)
{
    LOG_WRN("Unused command confirmation data received");
    return SIZE_UNUSED_COMMAND_CONFIRM;
}

size_t NotificationPacket::handleUnusedAutoStatus(const uint8_t *data)
{
    LOG_WRN("Unused auto status data received");
    return SIZE_UNUSED_AUTO_STATUS;
}

size_t NotificationPacket::handleUnusedLegacy(const uint8_t *data)
{
    LOG_WRN("Unused legacy data received");
    return SIZE_UNUSED_LEGACY;
}
