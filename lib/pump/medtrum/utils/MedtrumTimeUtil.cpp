#include <pump/medtrum/utils/MedtrumTimeUtil.h>
#include <zephyr/kernel.h>

uint32_t MedtrumTimeUtil::getCurrentTimePumpSeconds()
{
    struct timespec currentTime;
    clock_gettime(CLOCK_REALTIME, &currentTime);

    return static_cast<uint32_t>(currentTime.tv_sec) - startEpoch;
}

time_t MedtrumTimeUtil::convertPumpTimeToSystemTime(uint32_t pumpTime)
{
    time_t pumpEpochTime = startEpoch + pumpTime;
    return pumpEpochTime;
}
