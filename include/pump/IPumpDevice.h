#ifndef IPUMP_DEVICE_H
#define IPUMP_DEVICE_H

#include <ctime>
#include <pump/PumpServiceMessages.h>

class IPumpDeviceCallback
{
public:
    /**
     * @brief Callback for pump status updates.
     * @param status The updated pump status.
     */
    virtual void pumpStatusUpdate(const PumpStatus &status) = 0;

    /**
     * @brief Callback for bolus progress updates.
     * @param update The updated bolus progress.
     */
    virtual void bolusProgressUpdate(const BolusProgressUpdate &update) = 0;
};

class IPumpDevice
{
public:
    virtual ~IPumpDevice() = default;
    virtual void init() = 0;

    /**
     * @brief Handle a bolus request.
     * @param amount The amount of insulin to deliver.
     * @param timestamp The timestamp of the request.
     */
    virtual void onBolusRequest(float amount, time_t timestamp) = 0;

    /**
     * @brief Handle a request to stop the bolus delivery.
     */
    virtual void onStopBolusRequest() = 0;

    /**
     * @brief Handle a request to retract the plunger.
     */
    virtual void onRetractRequest() = 0;
};

#endif // IPUMP_DEVICE_H
