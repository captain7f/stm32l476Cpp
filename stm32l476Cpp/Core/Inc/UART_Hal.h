/*
 * UART_Hal.h
 *
 *  Created on: Oct 30, 2025
 *      Author: FA
 */

#ifndef INC_UART_HAL_H_
#define INC_UART_HAL_H_

//-------includes------------
#include "main.h"
#include "usart.h"

#include "dma_Hal.h"
#include "ISR_Hal.h"
#include "CircularBuffer.h"
//-------defines------------
#define UART_COUNT		(1)
#define RX_SIZE_BUFFER	(64)

#ifndef UART_ISR_MAX
  #define UART_ISR_MAX	(2)
#endif

// ------------- UartBase -------------
class UartBase{
public:
	UartBase(USART_TypeDef* Instance, UART_HandleTypeDef* huart);
	~UartBase();

	void init();
	HAL_StatusTypeDef write(uint8_t* pData, uint16_t Size, uint32_t Timeout =1000);
protected:
	USART_TypeDef* _Instance;			// UART instance (USART1, USART2, etc.)
	UART_HandleTypeDef* _huart;			// HAL UART handle
private:
	static bool  isInit[UART_COUNT];
};



// ---------- Interrupt-based UART (non-blocking RX/TX) ----------


class UartIT: public UartBase{
public:
	UartIT(USART_TypeDef* Instance, UART_HandleTypeDef* huart);
	~UartIT();

	virtual HAL_StatusTypeDef write(uint8_t *pData, uint16_t Size);
	virtual HAL_StatusTypeDef start_read(void);
	uint16_t read(uint8_t *pData, uint16_t Size);
	bool is_tx_done(void);
protected:
    bool _is_tx_done;    // Flag: transmission complete or not
    uint8_t _read_buffer[RX_SIZE_BUFFER];   // Temporary HAL RX buffer
    CircularBuffer<uint8_t, RX_SIZE_BUFFER * 2> _buffer; // static circular buffer

    static ISR<UartIT, UART_ISR_MAX> ISR_LIST; // List of ISR instances

private:
    static void TxCpltCallback(UART_HandleTypeDef* huart);        // TX complete callback
    static void RxEventCallback(UART_HandleTypeDef* huart, uint16_t Pos); // RX event callback
    virtual void put(uint16_t index, uint16_t size);                      // Push RX data into circular buffer
};



// ----------- UartDMA implementation -----------

class UartDMA : public UartIT{
public:
	UartDMA(USART_TypeDef* Instance, UART_HandleTypeDef* huart);
	~UartDMA();
	virtual HAL_StatusTypeDef write(uint8_t *pData, uint16_t Size);
	virtual HAL_StatusTypeDef start_read(void);
private:

    virtual void put(uint16_t index, uint16_t size);                      // Push RX data into circular buffer
};





#endif /* INC_UART_HAL_H_ */
