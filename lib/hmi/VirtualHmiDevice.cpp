#include <hmi/VirtualHmiDevice.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_virtual_hmi_device);

VirtualHmiDevice::VirtualHmiDevice(IHmiCallback &hmiCallback) : mHmiCallback(hmiCallback)
{
    LOG_DBG("VirtualHmiDevice constructor");
}

VirtualHmiDevice::~VirtualHmiDevice()
{
    LOG_DBG("VirtualHmiDevice destructor");
}

void VirtualHmiDevice::init()
{
    LOG_DBG("VirtualHmiDevice init");
}

void VirtualHmiDevice::onUserBtPairingRequest(struct bt_conn *conn, uint32_t passkey)
{
    // Accept the pairing request, we are virtual :)
    mHmiCallback.onUserBtPairingResponse(conn, true);
}

void VirtualHmiDevice::onBtBluetoothStateChanged(struct bt_conn *conn, BtState state)
{
    LOG_DBG("onBtBluetoothStateChanged: state=%d", static_cast<int>(state));
}

void VirtualHmiDevice::onBolusProgressUpdate(BolusProgressUpdate &update)
{
    LOG_DBG("onBolusProgressUpdate: requestedAmount=%f, deliveredAmount=%f, completed=%d",
            static_cast<double>(update.requestedAmount), static_cast<double>(update.deliveredAmount), update.completed);
}
