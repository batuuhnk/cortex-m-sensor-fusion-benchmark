/**
 * @file bench_port_stm32.c
 * @brief STM32 port of the benchmark harness (any Cortex-M with a DWT unit).
 *
 * Add this file and bench/ to a STM32CubeIDE project, then call
 * bench_run_all() once from main() after the clocks are configured.
 *
 * Log output:
 *   - define BENCH_UART to the name of a HAL UART handle (e.g. huart2) to send
 *     through HAL_UART_Transmit, or
 *   - leave it undefined and retarget printf (ITM/SWO, RTT, or _write()).
 */
#include "main.h"            /* pulls in the device header and CMSIS core */

#include <string.h>
#include <stdio.h>

#include "bench_timer.h"
#include "bench_runner.h"

void bench_timer_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;   /* enable trace block   */
#if defined(DWT_LAR_ACCESS) || defined(STM32H7) || defined(STM32F7)
    DWT->LAR = 0xC5ACCE55U;                           /* unlock on M7 parts   */
#endif
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;              /* start cycle counter  */
}

uint32_t bench_timer_now(void)
{
    return DWT->CYCCNT;
}

uint32_t bench_timer_hz(void)
{
    return SystemCoreClock;
}

#ifdef BENCH_UART
extern UART_HandleTypeDef BENCH_UART;
void bench_print(const char *s)
{
    (void)HAL_UART_Transmit(&BENCH_UART, (const uint8_t *)s, (uint16_t)strlen(s), 100U);
}
#else
void bench_print(const char *s)
{
    fputs(s, stdout);
    fflush(stdout);
}
#endif

/* Report which caches were enabled for this run. */
const char *bench_cache_tag(void)
{
#if defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1U)   /* Cortex-M7: L1 caches in SCB */
    const uint32_t ccr = SCB->CCR;
    const int ic = ((ccr & SCB_CCR_IC_Msk) != 0U);
    const int dc = ((ccr & SCB_CCR_DC_Msk) != 0U);
    return ic ? (dc ? "I+D" : "I") : (dc ? "D" : "off");
#elif defined(ICACHE) && defined(ICACHE_CR_EN)               /* STM32U5/L5/H5: ICACHE peripheral */
    return ((ICACHE->CR & ICACHE_CR_EN) != 0U) ? "ICACHE" : "off";
#elif defined(FLASH_ACR_ICEN)                                 /* STM32F4/L4: flash ART accelerator */
    return ((FLASH->ACR & FLASH_ACR_ICEN) != 0U) ? "ART" : "off";
#else
    return "n/a";
#endif
}
