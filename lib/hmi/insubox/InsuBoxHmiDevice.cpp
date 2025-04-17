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
    lv_indev_t *inputDevice = lvgl_input_get_indev(mKeypadDevice);
    // Set encoder type because we only have 3 buttons
    lv_indev_set_type(inputDevice, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_group(inputDevice, keypad);

    showMainScreen();
    k_work_schedule_for_queue(&mWorkQueue, &mDisplayUpdateTask.work, K_NO_WAIT);
}

void InsuBoxHmiDevice::onUserBtPairingRequest(struct bt_conn *conn, uint32_t passkey)
{
    LOG_DBG("onUserBtPairingRequest: passkey=%u", passkey);

    // Define colors
    lv_color_t purple_color = lv_color_hex(0xff00ff);
    lv_color_t black_color = lv_color_hex(0x000000);
    ;

    // Create container to organize content - sized to fit the small display (76x284)
    lv_obj_t *cont = lv_obj_create(lv_screen_active());
    if (!storePairingScreen(conn, cont))
    {
        LOG_ERR("Failed to store pairing screen");
        mHmiCallback.onUserBtPairingResponse(conn, false);
        return;
    }
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
            // data->device->showMainScreen();
            data->device->removePairingScreen(data->conn);
        },
        LV_EVENT_CLICKED, &callbackData);

    // Set the callback for the reject button
    lv_obj_add_event_cb(
        rejectBtn,
        [](lv_event_t *e) {
            auto *data = static_cast<CallbackData *>(lv_event_get_user_data(e));
            data->device->mHmiCallback.onUserBtPairingResponse(data->conn, false);
            // data->device->showMainScreen();
            data->device->removePairingScreen(data->conn);
        },
        LV_EVENT_CLICKED, &callbackData);

    lv_group_add_obj(lv_group_get_default(), cont);
    lv_group_focus_obj(cont);
}

void InsuBoxHmiDevice::onBtBluetoothStateChanged(struct bt_conn *conn, BtState state)
{
    LOG_DBG("onBtBluetoothStateChanged: state=%d", static_cast<int>(state));
    if (state == BtState::BT_STATE_DISCONNECTED)
    {
        // Remove the pairing screen if it exists
        removePairingScreen(conn);
    }
}

void InsuBoxHmiDevice::showMainScreen()
{
    lv_obj_clean(lv_screen_active());
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN);

    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello, InsuBox!");
    lv_obj_set_style_text_color(label, lv_color_hex(0xff00ff), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 10, 10);

    lv_obj_t *bolusBtn = lv_btn_create(lv_screen_active());
    lv_obj_set_size(bolusBtn, 55, 22);
    lv_obj_align(bolusBtn, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_t *btnLabel = lv_label_create(bolusBtn);
    lv_label_set_text(btnLabel, "Bolus");
    lv_obj_center(btnLabel);

    lv_obj_add_event_cb(
        bolusBtn,
        [](lv_event_t *e) {
            auto *device = static_cast<InsuBoxHmiDevice *>(lv_event_get_user_data(e));
            device->showBolusScreen();
        },
        LV_EVENT_CLICKED, this);
}

void InsuBoxHmiDevice::showBolusScreen()
{
    lv_obj_clean(lv_screen_active());

    constexpr int32_t MAX_BOLUS = 1500; // 15.00 units

    // Create the spinbox
    lv_obj_t *spinbox = lv_spinbox_create(lv_screen_active());
    lv_spinbox_set_range(spinbox, 0, MAX_BOLUS);
    lv_spinbox_set_rollover(spinbox, true);
    lv_spinbox_set_digit_format(spinbox, 4, 2);
    lv_spinbox_set_cursor_pos(spinbox, 2);
    lv_obj_set_width(spinbox, 55);
    lv_obj_align(spinbox, LV_ALIGN_CENTER, -32, 0);
    lv_obj_set_style_text_color(spinbox, lv_color_hex(0xff00ff), LV_PART_MAIN);

    // OK button
    lv_obj_t *okBtn = lv_btn_create(lv_screen_active());
    lv_obj_set_size(okBtn, 55, 22);
    lv_obj_align(okBtn, LV_ALIGN_RIGHT_MID, -10, -15);
    lv_obj_t *okLabel = lv_label_create(okBtn);
    lv_label_set_text(okLabel, "OK");
    lv_obj_center(okLabel);

    // Cancel button
    lv_obj_t *cancelBtn = lv_btn_create(lv_screen_active());
    lv_obj_set_size(cancelBtn, 55, 22);
    lv_obj_align(cancelBtn, LV_ALIGN_RIGHT_MID, -10, 15);
    lv_obj_t *cancelLabel = lv_label_create(cancelBtn);
    lv_label_set_text(cancelLabel, "Cancel");
    lv_obj_center(cancelLabel);

    struct BolusSpinboxData
    {
        InsuBoxHmiDevice *device;
        lv_obj_t *spinbox;
    };

    static BolusSpinboxData cbData{this, spinbox};

    lv_obj_add_event_cb(
        cancelBtn,
        [](lv_event_t *e) {
            auto *data = static_cast<BolusSpinboxData *>(lv_event_get_user_data(e));
            data->device->showMainScreen();
        },
        LV_EVENT_CLICKED, &cbData);

    lv_obj_add_event_cb(
        okBtn,
        [](lv_event_t *e) {
            auto *data = static_cast<BolusSpinboxData *>(lv_event_get_user_data(e));
            int32_t value = lv_spinbox_get_value(data->spinbox);
            float bolus = value / 100.0f;
            LOG_INF("Bolus requested: %.2f", static_cast<double>(bolus));
            // TODO: Send bolus request
            data->device->showMainScreen();
        },
        LV_EVENT_CLICKED, &cbData);

    lv_group_focus_obj(spinbox);
    lv_group_set_editing(lv_group_get_default(), true);
}

bool InsuBoxHmiDevice::storePairingScreen(bt_conn *conn, lv_obj_t *screen)
{
    for (auto &entry : mPairingScreens)
    {
        if (entry.conn == nullptr)
        {
            entry.conn = conn;
            entry.screen = screen;
            return true;
        }
    }
    LOG_WRN("Max pairing screens reached, cannot store new screen");
    return false;
}

void InsuBoxHmiDevice::removePairingScreen(bt_conn *conn)
{
    for (auto &entry : mPairingScreens)
    {
        if (entry.conn == conn)
        {
            if (entry.screen)
            {
                lv_obj_del(entry.screen);
            }
            entry.conn = nullptr;
            entry.screen = nullptr;
            return;
        }
    }
}
