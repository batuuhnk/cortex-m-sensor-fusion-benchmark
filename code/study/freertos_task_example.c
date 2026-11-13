/**
 * @file freertos_task_example.c
 * @brief How the loads were run in the study: one FreeRTOS task per load,
 *        traced with SEGGER SystemView, which reports each task's run time.
 *
 * Swap the call in the loop to measure dummy_kf() or opt_sim(), and rebuild
 * with KF_S / KF_REAL set for the case. The stack high-water mark is sent to
 * SystemView from the idle hook to size the task stack.
 */
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"
#include "SEGGER_SYSVIEW.h"

#include "dummy_kf.h"

extern osThreadId_t calibrationTaskHandle;

void StartCalibrationTask(void *argument)
{
    static kf_real_t xhat[KF_S];
    static kf_real_t sigma_x[KF_S][KF_S];
    (void)argument;

    for (;;) {
        dummy_kf(xhat, sigma_x);   /* or: opt_sim(); */
        osDelay(5);
    }
}

void vApplicationIdleHook(void)
{
    UBaseType_t watermark = uxTaskGetStackHighWaterMark(calibrationTaskHandle);
    SEGGER_SYSVIEW_PrintfHost("calibrationTask stack min free: %lu words", (unsigned long)watermark);
}
