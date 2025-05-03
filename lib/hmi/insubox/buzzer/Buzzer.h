#ifndef BUZZER_H
#define BUZZER_H

#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include "Tunes.h"

class Buzzer
{
public:
    Buzzer();
    
    /**
     * @brief Play a tune on the buzzer. (async)
     * 
     * @param tune Pointer to the tune to play.
     * @param volume Volume level (0.0 to 1.0).
     */
    void playTune(const Tune* tune, float volume = 1.0);

    /**
     * @brief Stop playing the current tune.
     */
    void stop();

private:
    const struct pwm_dt_spec mPwmBuzzer;
    
    float mVolume = 1.0;
    const Tune* mCurrentTune = nullptr;
    size_t mNoteIndex = 0;

    struct k_work_delayable mWork;
    static k_work_q mWorkQueue;
    K_KERNEL_STACK_MEMBER(mWorkQueueBuffer, 512);

    void scheduleNextNote();
    static void workHandler(struct k_work* work);
};

#endif // BUZZER_H
