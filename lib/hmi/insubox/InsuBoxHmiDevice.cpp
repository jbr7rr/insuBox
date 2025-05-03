#include <hmi/insubox/InsuBoxHmiDevice.h>

#include <ctime>
#include <lvgl.h>
#include <lvgl_input_device.h>
#include <lvgl_zephyr.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/pm/device.h>

#include "buzzer/Buzzer.h"
#include "buzzer/Tunes.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_insubox_hmi_device);

InsuBoxHmiDevice::InsuBoxHmiDevice(IHmiCallback &hmiCallback, k_work_q &workQueue)
    : mHmiCallback(hmiCallback), mWorkQueue(workQueue), mBuzzer(createBuzzerInstance())
{
    LOG_DBG("InsuBoxHmiDevice constructor");

    mDisplayUpdateTask.device = this;
    k_work_init_delayable(&mDisplayUpdateTask.work, [](struct k_work *work) {
        // LOG_DBG("Display update task");
        auto *container = CONTAINER_OF(work, DisplayUpdateTask, work);
        constexpr uint32_t DISPLAY_TIMEOUT = CONFIG_IB_HMI_DISPLAY_TIMEOUT_SEC * 1000;
        if (lv_disp_get_inactive_time(nullptr) > DISPLAY_TIMEOUT && container->device->mDisplayOn)
        {
            LOG_DBG("Display off");
            container->device->mDisplayOn = false;
            display_blanking_on(container->device->mDisplayDevice);
            pm_device_action_run(container->device->mDisplayDevice, PM_DEVICE_ACTION_SUSPEND);
        }
        else if (lv_disp_get_inactive_time(nullptr) < DISPLAY_TIMEOUT && !container->device->mDisplayOn)
        {
            LOG_DBG("Display on");
            container->device->mDisplayOn = true;
            display_blanking_off(container->device->mDisplayDevice);
            pm_device_action_run(container->device->mDisplayDevice, PM_DEVICE_ACTION_RESUME);
        }

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

    mBuzzer.playTune(hmiTuneWelcome);

    lv_group_t *keypad = lv_group_create();
    lv_group_set_default(keypad);
    lv_indev_t *inputDevice = lvgl_input_get_indev(mKeypadDevice);
    // Set encoder type because we only have 3 buttons
    lv_indev_set_type(inputDevice, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_group(inputDevice, keypad);

    lv_indev_add_event_cb(
        inputDevice,
        [](lv_event_t *e) {
            auto *device = static_cast<InsuBoxHmiDevice *>(lv_event_get_user_data(e));
            if (device)
            {
                device->mBuzzer.playTune(hmiTuneKeyClick, 0.6f);
            }
        },
        LV_EVENT_CLICKED, this);

    lv_theme_t *theme = lv_theme_default_init(lv_disp_get_default(), lv_palette_main(LV_PALETTE_PURPLE),
                                              lv_palette_main(LV_PALETTE_CYAN), true, LV_FONT_DEFAULT);
    lv_display_set_theme(lv_disp_get_default(), theme);

    showMainScreen();
    k_work_schedule_for_queue(&mWorkQueue, &mDisplayUpdateTask.work, K_NO_WAIT);
}

void InsuBoxHmiDevice::onUserBtPairingRequest(struct bt_conn *conn, uint32_t passkey)
{
    LOG_DBG("onUserBtPairingRequest: passkey=%u", passkey);

    // Define colors
    lv_color_t purple_color = lv_color_hex(0xff00ff);
    lv_color_t black_color = lv_color_hex(0x000000);

    // Create container to organize content - sized to fit the small display (76x284)
    lv_obj_t *cont = lv_obj_create(lv_screen_active());
    PairingScreenEntry *pairingScreenEntry = storePairingScreen(conn, cont);
    if (pairingScreenEntry == nullptr)
    {
        LOG_ERR("Failed to store pairing screen");
        mHmiCallback.onUserBtPairingResponse(conn, false);
        return;
    }

    lv_obj_set_size(cont, 260, 70);
    lv_obj_set_style_bg_color(cont, black_color, LV_PART_MAIN);
    lv_obj_set_style_border_width(cont, 3, LV_PART_MAIN);
    lv_obj_set_style_pad_all(cont, 2, LV_PART_MAIN);
    lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);

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
    lv_obj_t *acceptLabel = lv_label_create(acceptBtn);
    lv_label_set_text(acceptLabel, "Accept");
    lv_obj_center(acceptLabel);

    // Create reject button
    lv_obj_t *rejectBtn = lv_btn_create(cont);
    lv_obj_set_size(rejectBtn, 55, 22);
    lv_obj_align(rejectBtn, LV_ALIGN_RIGHT_MID, -10, 15);
    lv_obj_t *rejectLabel = lv_label_create(rejectBtn);
    lv_label_set_text(rejectLabel, "Reject");
    lv_obj_center(rejectLabel);

    // Set the callback for the accept button
    lv_obj_add_event_cb(
        acceptBtn,
        [](lv_event_t *e) {
            PairingScreenEntry *entry = static_cast<PairingScreenEntry *>(lv_event_get_user_data(e));
            if (entry)
            {
                entry->device->mHmiCallback.onUserBtPairingResponse(entry->conn, true);
                entry->device->removePairingScreen(entry->conn);
            }
            else
            {
                LOG_ERR("Failed to get pairing screen entry");
            }
        },
        LV_EVENT_CLICKED, pairingScreenEntry);

    // Set the callback for the reject button
    lv_obj_add_event_cb(
        rejectBtn,
        [](lv_event_t *e) {
            PairingScreenEntry *entry = static_cast<PairingScreenEntry *>(lv_event_get_user_data(e));
            if (entry)
            {
                entry->device->mHmiCallback.onUserBtPairingResponse(entry->conn, false);
                entry->device->removePairingScreen(entry->conn);
            }
            else
            {
                LOG_ERR("Failed to get pairing screen entry");
            }
        },
        LV_EVENT_CLICKED, pairingScreenEntry);

    lv_group_add_obj(lv_group_get_default(), cont);
    lv_group_focus_obj(acceptBtn);
    lv_disp_trig_activity(nullptr);
    mBuzzer.playTune(hmiTuneKeyRequest, 0.6f);
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

void InsuBoxHmiDevice::onBolusProgressUpdate(BolusProgressUpdate &update)
{
    mBolusUi.currentUpdate = update;

    if (update.requestedAmount == 0.0f)
    {
        return;
    }

    if (mBolusUi.popup == nullptr && mBolusUi.mainScreenLabel == nullptr)
    {
        createBolusProgressPopup();
        return;
    }

    if (update.completed)
    {
        if (mBolusUi.popup)
        {
            // Only play tune when popup is active
            mBuzzer.playTune(hmiTuneSuccess, 0.6f);
            lv_obj_del(mBolusUi.popup);
            mBolusUi.popup = nullptr;
        }
        if (mBolusUi.mainScreenLabel)
        {
            lv_label_set_text(mBolusUi.mainScreenLabel, "Bolus completed");
            mBolusUi.mainScreenLabel = nullptr;
        }
    }
    else
    {
        if (mBolusUi.popup)
        {
            LOG_DBG("Updating bolus progress popup");
            lv_obj_t *progressBar = lv_obj_get_child(mBolusUi.popup, 0);
            uint32_t progress = static_cast<uint32_t>(update.deliveredAmount * 100.0f / update.requestedAmount);
            lv_bar_set_value(progressBar, progress, LV_ANIM_ON);

            char progressText[50];
            snprintf(progressText, sizeof(progressText), "%.2fU / %.2fU", static_cast<double>(update.deliveredAmount),
                     static_cast<double>(update.requestedAmount));
            LOG_DBG("Bolus progress: %s", progressText);
            lv_label_set_text(lv_obj_get_child(mBolusUi.popup, 1), progressText);
            lv_disp_trig_activity(nullptr);
        }
        if (mBolusUi.mainScreenLabel)
        {
            LOG_DBG("Updating main screen label");
            char progressText[50];
            snprintf(progressText, sizeof(progressText), "%.2fU / %.2fU", static_cast<double>(update.deliveredAmount),
                     static_cast<double>(update.requestedAmount));
            LOG_DBG("Bolus progress: %s", progressText);
            lv_label_set_text(mBolusUi.mainScreenLabel, progressText);
        }
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

    // Show off some colors
    const int segmentCount = 6;
    const lv_color_t rainbowColors[segmentCount] = {
        lv_color_hex(0xFF0000), // Red
        lv_color_hex(0xFF7F00), // Orange
        lv_color_hex(0xFFFF00), // Yellow
        lv_color_hex(0x00FF00), // Green
        lv_color_hex(0x0000FF), // Blue
        lv_color_hex(0x8B00FF)  // Violet
    };

    for (int i = 0; i < segmentCount; ++i)
    {
        lv_obj_t *segment = lv_obj_create(lv_screen_active());
        lv_obj_set_size(segment, 20, 20);
        lv_obj_set_style_bg_color(segment, rainbowColors[i], LV_PART_MAIN);
        lv_obj_align(segment, LV_ALIGN_TOP_LEFT, 10 + i * 22, 40);
    }

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
            struct timespec currentTime;
            clock_gettime(CLOCK_REALTIME, &currentTime);
            LOG_INF("Bolus requested: %.2f, time: %lld", static_cast<double>(bolus), currentTime.tv_sec);
            data->device->mHmiCallback.onBolusRequest(bolus, currentTime.tv_sec);
            data->device->showMainScreen();
        },
        LV_EVENT_CLICKED, &cbData);

    lv_group_focus_obj(spinbox);
    lv_group_set_editing(lv_group_get_default(), true);
}

void InsuBoxHmiDevice::createBolusProgressPopup()
{
    lv_obj_t *cont = lv_obj_create(lv_screen_active());
    mBolusUi.popup = cont;

    // setup container style
    lv_obj_set_size(cont, 260, 70);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(cont, 3, LV_PART_MAIN);
    lv_obj_set_style_pad_all(cont, 2, LV_PART_MAIN);
    lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);
    lv_gridnav_add(cont, LV_GRIDNAV_CTRL_ROLLOVER);

    // Show progress bar
    lv_obj_t *progressBar = lv_bar_create(cont);
    lv_obj_set_size(progressBar, 170, 18);
    lv_obj_align(progressBar, LV_ALIGN_CENTER, -35, -10);
    uint32_t progress =
        static_cast<uint32_t>(mBolusUi.currentUpdate.deliveredAmount * 100.0f / mBolusUi.currentUpdate.requestedAmount);
    lv_bar_set_value(progressBar, progress, LV_ANIM_ON);

    // Label for progress
    lv_obj_t *progressLabel = lv_label_create(cont);
    char progressText[50];
    snprintf(progressText, sizeof(progressText), "%.2fU / %.2fU",
             static_cast<double>(mBolusUi.currentUpdate.deliveredAmount),
             static_cast<double>(mBolusUi.currentUpdate.requestedAmount));
    LOG_DBG("Bolus progress: %s", progressText);
    lv_label_set_text(progressLabel, progressText);
    lv_obj_align(progressLabel, LV_ALIGN_CENTER, -35, 10);

    // Hide button
    lv_obj_t *hideBtn = lv_btn_create(cont);
    lv_obj_set_size(hideBtn, 55, 22);
    lv_obj_align(hideBtn, LV_ALIGN_RIGHT_MID, -10, -15);
    lv_obj_t *hideLabel = lv_label_create(hideBtn);
    lv_label_set_text(hideLabel, "Hide");
    lv_obj_center(hideLabel);

    // Stop button
    lv_obj_t *stopBtn = lv_btn_create(cont);
    lv_obj_set_size(stopBtn, 55, 22);
    lv_obj_align(stopBtn, LV_ALIGN_RIGHT_MID, -10, 15);
    lv_obj_t *stopLabel = lv_label_create(stopBtn);
    lv_label_set_text(stopLabel, "Stop");
    lv_obj_center(stopLabel);

    lv_obj_add_event_cb(
        stopBtn,
        [](lv_event_t *e) {
            auto *device = static_cast<InsuBoxHmiDevice *>(lv_event_get_user_data(e));
            device->mHmiCallback.onStopBolus();
            // Do not remove the widget yet, wait for the progress update
        },
        LV_EVENT_CLICKED, this);

    lv_obj_add_event_cb(
        hideBtn,
        [](lv_event_t *e) {
            auto *device = static_cast<InsuBoxHmiDevice *>(lv_event_get_user_data(e));
            lv_obj_del(device->mBolusUi.popup);
            device->mBolusUi.popup = nullptr;
            lv_obj_t *mainScreenLabel = lv_obj_get_child(lv_screen_active(), 0);
            if (mainScreenLabel)
            {
                device->mBolusUi.mainScreenLabel = mainScreenLabel;
                char progressText[50];
                snprintf(progressText, sizeof(progressText), "%.2fU / %.2fU",
                         static_cast<double>(device->mBolusUi.currentUpdate.deliveredAmount),
                         static_cast<double>(device->mBolusUi.currentUpdate.requestedAmount));
                LOG_DBG("Bolus progress: %s", progressText);
                lv_label_set_text(mainScreenLabel, progressText);
            }
            else
            {
                LOG_ERR("Failed to get main screen label");
            }
        },
        LV_EVENT_CLICKED, this);

    lv_disp_trig_activity(nullptr);
}

InsuBoxHmiDevice::PairingScreenEntry *InsuBoxHmiDevice::storePairingScreen(bt_conn *conn, lv_obj_t *screen)
{
    for (auto &entry : mPairingScreens)
    {
        if (entry.conn == nullptr)
        {
            entry.device = this;
            entry.conn = conn;
            entry.screen = screen;
            return &entry;
        }
    }
    LOG_WRN("Max pairing screens reached, cannot store new screen");
    return nullptr;
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
            entry.device = nullptr;
            entry.conn = nullptr;
            entry.screen = nullptr;
            return;
        }
    }
}

Buzzer &InsuBoxHmiDevice::createBuzzerInstance()
{
    static Buzzer buzzer;
    return buzzer;
}

/**** Display Splash Screen Initialization ****/
extern const uint8_t logo_flat_displ_map[];
extern const lv_image_dsc_t logo_flat_displ;

static int display_splash_init(void)
{
    const struct device *display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(display))
    {
        return -ENODEV;
    }
    display_blanking_off(display);

    const struct display_buffer_descriptor desc = {
        .buf_size = static_cast<uint32_t>(logo_flat_displ.data_size),
        .width = static_cast<uint16_t>(logo_flat_displ.header.w),
        .height = static_cast<uint16_t>(logo_flat_displ.header.h),
        .pitch = static_cast<uint16_t>(logo_flat_displ.header.w),
    };
    display_write(display, 0, 0, &desc, logo_flat_displ_map);

    return 0;
}
SYS_INIT(display_splash_init, POST_KERNEL, CONFIG_APPLICATION_INIT_PRIORITY);
