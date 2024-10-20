#include <hmi/HmiService.h>
#include <ble/BLEComm.h>

#include <zephyr/zbus/zbus.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_hmi_service);

HmiService::HmiService()
{
    LOG_DBG("HmiService constructor");
}

HmiService::~HmiService() {}

void HmiService::init()
{
    LOG_DBG("Initializing HmiService");
}

// ZBUS stuff
ZBUS_CHAN_DECLARE(bleChan);
ZBUS_MSG_SUBSCRIBER_DEFINE(controlSubBleChan);
ZBUS_CHAN_ADD_OBS(bleChan, controlSubBleChan, 3);

static void controlSubBleChanTask(void)
{
    const struct zbus_channel *chan;
    struct BleCommMsg msg;

    while (1)
    {
        if (zbus_sub_wait_msg(&controlSubBleChan, &chan, &msg, K_FOREVER) == 0)
        {
            LOG_DBG("Received message on controlSubBleChan");
            if (msg.type == BleCommMsg::Type::PASSKEY_CONFIRM_REQ)
            {
                // TODO: HMI link etc, for now just confirm
                LOG_DBG("Passkey confirm request");
                // Create response msg
                BleCommMsg response = {
                    .type = BleCommMsg::Type::PASSKEY_CONFIRM,
                    .conn = msg.conn,
                    .passkey = msg.passkey,
                };

                // Publish response to zbus
                zbus_chan_pub(&bleChan, &response, K_FOREVER);
            }
        }
    }
}
K_THREAD_DEFINE(controlSubBleChanThread, KB(1), controlSubBleChanTask, NULL, NULL, NULL, 3, 0, 0);
