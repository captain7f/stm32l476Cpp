/*
 * GPIO_Hal.h
 *
 *  Created on: Oct 29, 2025
 *      Author: FA
 */

#ifndef INC_GPIO_HAL_H_
#define INC_GPIO_HAL_H_

#include "gpio.h"
#include "stm32l4xx_hal.h"
// FA includes
#include "ISR_Hal.h"

// User callback type for ButtonISR
typedef void(*gpio_isr_cb)(void);

// ----------------------------------------
// Base class for any GPIO-backed object
// - Holds port/pin
// - Ensures MX_GPIO_Init() is called once
// ----------------------------------------
class GpioBase{
public:
	GpioBase(GPIO_TypeDef* port, uint16_t pin);
	 virtual ~GpioBase() {}

	static void init(void);
	uint16_t get_pin(void);

	 uint16_t      pin()  const { return m_pin; }
	 GPIO_TypeDef* port() const { return m_port; }
protected:
	GPIO_TypeDef* m_port;
	uint16_t m_pin;
private:
	static bool s_inited;
};

// ----------------------------------------
// LED wrapper (Output)
// ----------------------------------------
class Led : public GpioBase{
public:
	Led(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState state= GPIO_PIN_RESET);
	void on() const;
	void off() const;
	void toggle() const;
	GPIO_PinState read(void);
	void write(GPIO_PinState state);
private:

};

// ----------------------------------------
// Button wrapper (Input)
// ----------------------------------------
class Button: public GpioBase{
public:
	Button(GPIO_TypeDef* port, uint16_t pin);
	virtual ~Button() {}
	GPIO_PinState read(void);
private:

};


// ----------------------------------------
// Button with EXTI support (heap-free registry)
// - Registers/unregisters itself in a static ISR<ButtonISR,MAX>
// - HAL_GPIO_EXTI_Callback dispatches to trigger_pin()
// ----------------------------------------

// Max number of interrupt-driven buttons (tune to your needs)


#ifndef BUTTON_ISR_MAX
  #define BUTTON_ISR_MAX 8
#endif

class ButtonISR: public Button{
public:
	ButtonISR(GPIO_TypeDef* port, uint16_t pin);
	virtual ~ButtonISR();

	void set_isr_cb(gpio_isr_cb cb);
	void call_isr(void);

	static void trigger_pin(uint16_t gpio_pin);

private:
    gpio_isr_cb m_cb { nullptr };

    // Heap-free static registry
    static ISR<ButtonISR, BUTTON_ISR_MAX> ISR_LIST;
};


#endif /* INC_GPIO_HAL_H_ */
