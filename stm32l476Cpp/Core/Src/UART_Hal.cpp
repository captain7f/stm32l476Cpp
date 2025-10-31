/*
 * UART_Hal.cpp
 *
 * Implementation of UartBase and UartIT classes
 *
 *  Created on: Oct 30, 2025
 *      Author: FA
 */

#include "UART_Hal.h"

// ----------- UartBase implementation -----------
// Track which UARTs have been initialized
bool UartBase::isInit[]= {false};

UartBase::UartBase(USART_TypeDef* Instance, UART_HandleTypeDef* huart):_Instance(Instance), _huart(huart){
	UartBase::init();
}

UartBase::~UartBase(){
	if(_Instance == USART2){
		if(!isInit[0]) return;
		isInit[0]= false;
		HAL_UART_MspDeInit(_huart);// Initialize UART if needed
	}
}
// Initialize UART only once
void UartBase::init(){
	 // Deinitialize if instance is USART2 (can be extended for others)
	if(_Instance == USART2){
		if(isInit[0]) return;
		isInit[0]= true;
		MX_USART2_UART_Init();
	}
}
// Blocking transmit
HAL_StatusTypeDef UartBase::write(uint8_t* pData, uint16_t Size, uint32_t Timeout){
	return HAL_UART_Transmit(_huart, pData, Size, Timeout);
}


// ----------- UartIT implementation -----------
ISR<UartIT, UART_ISR_MAX> UartIT::ISR_LIST;

UartIT::UartIT(USART_TypeDef* Instance, UART_HandleTypeDef* huart)
	:UartBase(Instance, huart), _is_tx_done(true){
#if USE_HAL_UART_REGISTER_CALLBACKS != 1
#error "USE_HAL_UART_REGISTER_CALLBACKS must be set 1 "
#endif
	 // Register callbacks
	huart->TxCpltCallback = TxCpltCallback;
	huart->RxEventCallback = RxEventCallback;
	// Add this instance to ISR list
	(void)ISR_LIST.add(this);
}

UartIT::~UartIT(void){
	 // No heap, so no delete needed
	(void)ISR_LIST.remove(this);
}
// TX complete callback (called by HAL)
void UartIT::TxCpltCallback(UART_HandleTypeDef* huart){
	for(uint16_t i=0; i<ISR_LIST.size(); i++){
		if(ISR_LIST.get(i)->_huart == huart){
			ISR_LIST.get(i)->_is_tx_done= true;
		}
	}
}
// RX event callback (called by HAL when idle or buffer full)
void UartIT::RxEventCallback(UART_HandleTypeDef* huart, uint16_t Pos){
	for(uint16_t i=0; i<ISR_LIST.size(); i++){
		if(ISR_LIST.get(i)->_huart == huart){
			ISR_LIST.get(i)->put(0,Pos); // store received data
		}
	}
}
// Non-blocking transmit
HAL_StatusTypeDef UartIT::write(uint8_t *pData, uint16_t Size){
	if(!_is_tx_done) return HAL_BUSY;
	_is_tx_done= false;
	return HAL_UART_Transmit_IT(_huart, pData, Size);
}
// Read available data from circular buffer
uint16_t UartIT::read(uint8_t *pData, uint16_t Size){
	return _buffer.pull(pData, Size);
}
// Start UART reception using ReceiveToIdle interrupt mode
HAL_StatusTypeDef UartIT::start_read(void){
	memset(_read_buffer,0,sizeof(_read_buffer));
	return HAL_UARTEx_ReceiveToIdle_IT(_huart, _read_buffer, sizeof(_read_buffer));
}
// Move new RX data into the circular buffer
void UartIT::put(uint16_t index, uint16_t size){
	_buffer.put(&_read_buffer[index], size);
	start_read();// restart reception immediately
}
// Return TX done flag
bool UartIT::is_tx_done(void){
	return _is_tx_done;
}



