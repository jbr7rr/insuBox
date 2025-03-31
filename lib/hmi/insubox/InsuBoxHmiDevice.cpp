#include <hmi/insubox/InsuBoxHmiDevice.h>

#include <lvgl.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_insubox_hmi_device);

InsuBoxHmiDevice::InsuBoxHmiDevice(IHmiCallback &hmiCallback) : mHmiCallback(hmiCallback)
{
    LOG_DBG("InsuBoxHmiDevice constructor");

    // TODO: Not sure if we want this on the kernel thread
    k_work_init_delayable(&mDisplayUpdateTask, [](struct k_work *work) {
        uint32_t sleepMs = lv_timer_handler();
        k_work_reschedule(k_work_delayable_from_work(work), K_MSEC(sleepMs));
    });
}

InsuBoxHmiDevice::~InsuBoxHmiDevice()
{
    LOG_DBG("InsuBoxHmiDevice destructor");
}

void InsuBoxHmiDevice::init()
{
    LOG_DBG("InsuBoxHmiDevice init");
    mDisplayDevice = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(mDisplayDevice))
    {
        LOG_ERR("Display device not ready");
        return;
    }

    // Show a welcome message on the display
    // TODO: Splash screen
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello, InsuBox!");
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0xff00ff), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lv_timer_handler();
    display_blanking_off(mDisplayDevice);

    k_work_reschedule(&mDisplayUpdateTask, K_NO_WAIT);
}

void InsuBoxHmiDevice::onUserBtPairingRequest(struct bt_conn *conn, uint32_t passkey)
{
    // Handle user Bluetooth pairing request
}
