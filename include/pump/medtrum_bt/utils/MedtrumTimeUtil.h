#ifndef MEDTRUM_TIME_UTIL_H
#define MEDTRUM_TIME_UTIL_H

#include <cstdint>
#include <zephyr/kernel.h>
#include <zephyr/sys/timeutil.h>

class MedtrumTimeUtil
{
public:
    /**
     * @brief Get the current time in pump seconds since the reference start time.
     *
     * @return Number of seconds since "2014-01-01T00:00:00Z".
     */
    static uint32_t getCurrentTimePumpSeconds();

    /**
     * @brief Convert the given pump time to system time in milliseconds.
     *
     * @param pumpTime The pump time in seconds since "2014-01-01T00:00:00Z".
     * @return Corresponding system time in milliseconds since the Unix epoch.
     */
    static time_t convertPumpTimeToSystemTime(uint32_t pumpTime);

    /**
     * @brief Get the current system time in seconds since the Unix epoch.
     *
     * @return Number of seconds since the Unix epoch.
     */
    static time_t getCurrentTime();

private:
    static const time_t startEpoch = 1388534400; // "2014-01-01T00:00:00Z" in Unix time
};

#endif // MEDTRUM_TIME_UTIL_H
