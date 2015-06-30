/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"
/*
 * Regular GPIO driven LEDs
 */
#define _LED_GPIO(i)  i
#define _LED_GPIO_PIN(i) i

#define LED_GPIO(i) _LED_GPIO(LED_ ## i ## _GPIO)
#define LED_GPIO_PIN(i) _LED_GPIO_PIN(LED_ ## i ## _GPIO_PIN)

#define LED_ON(i) palClearPad(LED_GPIO(i), LED_GPIO_PIN(i))
#define LED_OFF(i) palSetPad(LED_GPIO(i), LED_GPIO_PIN(i))
#define LED_TOGGLE(i) palTogglePad(LED_GPIO(i), LED_GPIO_PIN(i))
#define LED_PERIODIC() {}
/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED.
 */
static WORKING_AREA(waThread1, 128);
static WORKING_AREA(waThread2, 512);
__attribute__((noreturn)) static msg_t Thread1(void *arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (TRUE) {
    palSetPad(GPIOA, 8);       /* Orange.  */
    chThdSleepMilliseconds(500);
    palClearPad(GPIOA, 8);     /* Orange.  */
    chThdSleepMilliseconds(500);
  }
}

/*
 * Serial config for SBUS
 * 100k Baud rate, 2 stop bits, Even parity (default), 8 data bits (default)
 */
static const SerialConfig sbus_config =
{
100000,                                                /*     BITRATE    */
0,                                                      /*    USART CR1   */
USART_CR2_STOP1_BITS,                                   /*    USART CR2   */
0                                                       /*    USART CR3   */
};

/**
 * Replacement for sbus_decode_event()
 * @param decode_sbus_buffer - callback to be called once buffer is full
 * @return
 */
__attribute__((noreturn)) msg_t Thread2(void *arg) {
  chRegSetThreadName("pprz_sbus_rx");
  (void) arg;


  EventListener elSBUSdata;
  flagsmask_t flags;
  chEvtRegisterMask((EventSource *)chnGetEventSource(&SD1), &elSBUSdata, EVENT_MASK(1));
  while (TRUE) {
     chEvtWaitOneTimeout(EVENT_MASK(1), S2ST(1));
     flags = chEvtGetAndClearFlags(&elSBUSdata);

     if (flags & CHN_INPUT_AVAILABLE) {
       msg_t charbuf;
       charbuf = sdGetTimeout(&SD1, TIME_IMMEDIATE);
       LED_TOGGLE(2);
     }
  }

}

/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();


  sdStop(&SD1);
  sdStart(&SD1, &sbus_config);

  /*
   * Creates the example thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
  chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO, Thread2, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state, when the button is
   * pressed the test procedure is launched with output on the serial
   * driver 2.
   */
  while (TRUE) {
    chThdSleepMilliseconds(500);
  }
}
