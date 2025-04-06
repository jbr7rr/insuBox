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

    lv_group_t *keypad = lv_group_create();
    lv_group_set_default(keypad);
    lv_indev_set_group(lvgl_input_get_indev(mKeypadDevice), keypad);

    showMainScreen();
    k_work_schedule_for_queue(&mWorkQueue, &mDisplayUpdateTask.work, K_NO_WAIT);
}

void InsuBoxHmiDevice::onUserBtPairingRequest(struct bt_conn *conn, uint32_t passkey)
{
    LOG_DBG("onUserBtPairingRequest: passkey=%u", passkey);

    // Define colors
    lv_color_t purple_color = lv_color_hex(0xff00ff);
    lv_color_t black_color = lv_color_hex(0x000000);

    // Clear the display
    lv_obj_clean(lv_screen_active());
    // Set black background
    lv_obj_set_style_bg_color(lv_screen_active(), black_color, LV_PART_MAIN);

    // Create container to organize content - sized to fit the small display (76x284)
    lv_obj_t *cont = lv_obj_create(lv_screen_active());
    lv_obj_set_size(cont, 260, 70);
    lv_obj_set_style_bg_color(cont, black_color, LV_PART_MAIN);
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(cont, 2, LV_PART_MAIN);
    lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);
    lv_gridnav_add(cont, LV_GRIDNAV_CTRL_ROLLOVER);

    // Show the passkey
    lv_obj_t *passkeyLabel = lv_label_create(cont);
    std::string passkeyText = "Pairing request \nPasskey: " + std::to_string(passkey);
    lv_label_set_text(passkeyLabel, passkeyText.c_str());
    lv_obj_set_style_text_color(passkeyLabel, purple_color, LV_PART_MAIN);
    lv_obj_align(passkeyLabel, LV_ALIGN_LEFT_MID, 10, 0);

    // Create accept button
    lv_obj_t *acceptBtn = lv_btn_create(cont);
    lv_obj_set_size(acceptBtn, 55, 22);
    lv_obj_align(acceptBtn, LV_ALIGN_RIGHT_MID, -10, -15);
    lv_obj_set_style_bg_color(acceptBtn, purple_color, LV_PART_MAIN);
    lv_obj_set_style_bg_color(acceptBtn, black_color, LV_STATE_FOCUSED);
    lv_obj_set_style_border_color(acceptBtn, purple_color, LV_STATE_FOCUSED);
    lv_obj_set_style_border_width(acceptBtn, 2, LV_STATE_FOCUSED);
    lv_obj_set_style_bg_color(acceptBtn, lv_color_darken(purple_color, 30), LV_STATE_PRESSED);
    lv_obj_t *acceptLabel = lv_label_create(acceptBtn);
    lv_label_set_text(acceptLabel, "Accept");
    lv_obj_center(acceptLabel);

    // Create reject button
    lv_obj_t *rejectBtn = lv_btn_create(cont);
    lv_obj_set_size(rejectBtn, 55, 22);
    lv_obj_align(rejectBtn, LV_ALIGN_RIGHT_MID, -10, 15);
    lv_obj_set_style_bg_color(rejectBtn, purple_color, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rejectBtn, black_color, LV_STATE_FOCUSED);
    lv_obj_set_style_border_color(rejectBtn, purple_color, LV_STATE_FOCUSED);
    lv_obj_set_style_border_width(rejectBtn, 2, LV_STATE_FOCUSED);
    lv_obj_set_style_bg_color(rejectBtn, lv_color_darken(purple_color, 30), LV_STATE_PRESSED);
    lv_obj_t *rejectLabel = lv_label_create(rejectBtn);
    lv_label_set_text(rejectLabel, "Reject");
    lv_obj_center(rejectLabel);

    struct CallbackData
    {
        InsuBoxHmiDevice *device;
        struct bt_conn *conn;
    };
    static CallbackData callbackData; // Static to ensure it persists
    callbackData.device = this;
    callbackData.conn = conn;

    // Set the callback for the accept button
    lv_obj_add_event_cb(
        acceptBtn,
        [](lv_event_t *e) {
            auto *data = static_cast<CallbackData *>(lv_event_get_user_data(e));
            data->device->mHmiCallback.onUserBtPairingResponse(data->conn, true);
            data->device->showMainScreen();
        },
        LV_EVENT_CLICKED, &callbackData);

    // Set the callback for the reject button
    lv_obj_add_event_cb(
        rejectBtn,
        [](lv_event_t *e) {
            auto *data = static_cast<CallbackData *>(lv_event_get_user_data(e));
            data->device->mHmiCallback.onUserBtPairingResponse(data->conn, false);
            data->device->showMainScreen();
        },
        LV_EVENT_CLICKED, &callbackData);

    lv_group_add_obj(lv_group_get_default(), cont);
    lv_group_focus_obj(cont);
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
