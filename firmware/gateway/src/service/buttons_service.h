#ifndef BUTTONS_SERVICE_H
#define BUTTONS_SERVICE_H

/**
 * @brief Buttons service — runs a FreeRTOS task that polls ADC HAL
 *        with debouncing and posts button events to the event bus.
 */

void buttons_service_init(void);

#endif // BUTTONS_SERVICE_H