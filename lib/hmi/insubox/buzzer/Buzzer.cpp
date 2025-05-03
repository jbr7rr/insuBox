#include "Buzzer.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(buzzer, LOG_LEVEL_DBG);

k_work_q Buzzer::mWorkQueue;

Buzzer::Buzzer() : mPwmBuzzer(PWM_DT_SPEC_GET_OR(DT_ALIAS(pwm_buzzer), {0}))
{
    if (!device_is_ready(mPwmBuzzer.dev))
    {
        LOG_ERR("PWM device not ready");
        return;
    }

    static k_work_queue_config config = {.name = "hmi_buzzer", .no_yield = false, .essential = true};
    k_work_queue_init(&mWorkQueue);
    k_work_queue_start(&mWorkQueue, mWorkQueueBuffer, K_THREAD_STACK_SIZEOF(mWorkQueueBuffer), 5, &config);
    k_work_init_delayable(&mWork, workHandler);
}

void Buzzer::playTune(const Tune *tune, float volume)
{
    if (!tune)
    {
        LOG_ERR("Invalid tune");
        return;
    }

    if (!device_is_ready(mPwmBuzzer.dev))
    {
        LOG_ERR("PWM device not ready");
        return;
    }

    stop(); // Stop previous tone
    mVolume = (volume < 0.0f) ? 0.0f : (volume > 1.0f) ? 1.0f : volume;
    mCurrentTune = tune;
    mNoteIndex = 0;
    scheduleNextNote();
}

void Buzzer::stop()
{
    k_work_cancel_delayable(&mWork);
    pwm_set_dt(&mPwmBuzzer, 1000000000UL, 0);
}

void Buzzer::scheduleNextNote()
{
    k_work_schedule_for_queue(&mWorkQueue, &mWork, K_NO_WAIT);
}

void Buzzer::workHandler(struct k_work *work)
{
    Buzzer *self = CONTAINER_OF(work, Buzzer, mWork);

    if (!self->mCurrentTune)
    {
        LOG_ERR("No tune to play");
        return;
    }

    const Tune &note = self->mCurrentTune[self->mNoteIndex];

    if (note.frequency == 0 || note.duration_ms == 0)
    {
        self->stop(); // End of tune
        return;
    }

    uint32_t period_ns = 1000000000UL / note.frequency;
    uint32_t pulse_ns = static_cast<uint32_t>(period_ns * 0.5f * self->mVolume);

    pwm_set_dt(&self->mPwmBuzzer, period_ns, pulse_ns);
    self->mNoteIndex++;

    k_work_schedule_for_queue(&self->mWorkQueue, &self->mWork, K_MSEC(note.duration_ms));
}
