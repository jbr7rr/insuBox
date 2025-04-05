#include <hmi/insubox/InsuBoxHmiDevice.h>

#include <lvgl.h>
#include <lvgl_input_device.h>
#include <lvgl_zephyr.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_insubox_hmi_device);

InsuBoxHmiDevice::InsuBoxHmiDevice(IHmiCallback &hmiCallback, k_work_q &workQueue)
    : mHmiCallback(hmiCallback), mWorkQueue(workQueue)
{
    LOG_DBG("InsuBoxHmiDevice constructor");

    // TODO: Not sure if we want this on the kernel thread
    mDisplayUpdateTask.device = this;
    k_work_init_delayable(&mDisplayUpdateTask.work, [](struct k_work *work) {
        // LOG_DBG("Display update task");
        auto *container = CONTAINER_OF(work, DisplayUpdateTask, work);
        uint32_t sleepMs = lv_timer_handler();
        k_work_schedule_for_queue(&container->device->mWorkQueue, k_work_delayable_from_work(work), K_MSEC(sleepMs));
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
    if (device_init(mDisplayDevice) != 0)
    {
        LOG_ERR("Failed to initialize display device");
        return;
    }
    if (!device_is_ready(mDisplayDevice))
    {
        LOG_ERR("Display device not ready");
        return;
    }
    display_blanking_off(mDisplayDevice);

    mKeypadDevice = DEVICE_DT_GET(DT_COMPAT_GET_ANY_STATUS_OKAY(zephyr_lvgl_keypad_input));
    if (!device_is_ready(mKeypadDevice))
    {
        LOG_ERR("Keypad device not ready");
        return;
    }

    lvgl_init();

    showMainScreen();

    k_work_schedule_for_queue(&mWorkQueue, &mDisplayUpdateTask.work, K_NO_WAIT);
}

void InsuBoxHmiDevice::onUserBtPairingRequest(struct bt_conn *conn, uint32_t passkey)
{
    // Handle user Bluetooth pairing request
    LOG_DBG("onUserBtPairingRequest: passkey=%u", passkey);

    // Clear the display
    lv_obj_clean(lv_screen_active());

    // Show the passkey on the display
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Pairing request");
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0xff00ff), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -20);

    lv_obj_t *passkeyLabel = lv_label_create(lv_screen_active());
    std::string passkeyText = "Passkey: " + std::to_string(passkey);
    lv_label_set_text(passkeyLabel, passkeyText.c_str());
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0xff00ff), LV_PART_MAIN);
    lv_obj_align(passkeyLabel, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN);

    // TODO: Listen for keypad input
}

void InsuBoxHmiDevice::showMainScreen()
{
    // Clear the display
    lv_obj_clean(lv_screen_active());

    // Set black background
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN);

    // Show a welcome message on the display
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello, InsuBox!");
    lv_obj_set_style_text_color(label, lv_color_hex(0xff00ff), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}
