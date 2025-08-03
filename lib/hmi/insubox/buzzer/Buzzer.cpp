#include "Buzzer.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(buzzer, LOG_LEVEL_DBG);

Buzzer::Buzzer() : mPwmBuzzer(PWM_DT_SPEC_GET_OR(DT_ALIAS(pwm_buzzer), {0}))
{
    if (!device_is_ready(mPwmBuzzer.dev))
    {
        LOG_ERR("PWM device not ready");
        return;
    }

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

    stop();
    mVolume = (volume < 0.0f) ? 0.0f : (volume > 1.0f) ? 1.0f : volume;
    mCurrentTune = tune;
    mNoteIndex = 0;
    k_work_schedule(&mWork, K_NO_WAIT);
}

void Buzzer::stop()
{
    k_work_cancel_delayable(&mWork);
    pwm_set_dt(&mPwmBuzzer, 1000000000UL, 0);
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

    if (note.frequency == 0 && note.duration_ms == 0)
    {
        self->stop(); // End of tune
        return;
    }

    uint32_t period_ns = 1000000000UL;
    uint32_t pulse_ns = 0;
    if (note.frequency > 20 && note.frequency < 20000)
    {
        period_ns = 1000000000UL / note.frequency;
        pulse_ns = static_cast<uint32_t>(period_ns * 0.5f * self->mVolume);
    }

    pwm_set_dt(&self->mPwmBuzzer, period_ns, pulse_ns);
    self->mNoteIndex++;

    k_work_schedule(&self->mWork, K_MSEC(note.duration_ms));
}
