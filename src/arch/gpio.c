#include "gpio.h"

#include <stdint.h>
#include <stddef.h>

#include "mcu.h"

void initializePin(PinSettings *settings)
{
	if (settings == NULL)
		return;
	
	GPIO_TypeDef *port = settings->port;
	uint16_t pinNumber = (uint16_t)settings->pinNumber;
	uint8_t mode = (uint8_t)settings->mode;
	uint8_t type = (uint8_t)settings->type;
	uint8_t speed = (uint8_t)settings->speed;
	uint8_t pull = (uint8_t)settings->pull;
	uint32_t alternateFunctionNumber = (uint32_t)settings->alternateFunctionNumber;
	
	if (port == GPIOA)
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	else if (port == GPIOB)
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	else if (port == GPIOC)
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	else if (port == GPIOD)
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	else if (port == GPIOE)
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
	
	for (uint8_t i = 0; i < 16; i++)
	{
		if ((pinNumber & (1 << i)) == 0)
			continue;
		
		port->MODER &= ~((uint8_t)pinModeMask << (i * 2));
		port->MODER |= (mode << (i * 2));
		
		port->OTYPER &= ~((uint8_t)pinTypeMask << i);
		port->OTYPER |= (type << i);
		
		port->OSPEEDR &= ~((uint8_t)pinSpeedMask << (i * 2));
		port->OSPEEDR |= (speed << (i * 2));
		
		port->PUPDR &= ~((uint8_t)pinPullMask << (i * 2));
		port->PUPDR |= (pull << (i * 2));
		
		if (mode == alternateFunctionPinMode)
		{
			uint8_t index = (i >= 8) ? 1 : 0;
			uint8_t position = ((i & 7) * 4);
			
			port->AFR[index] &= ~((uint32_t)alternateFunctionMask << position);
			port->AFR[index] |= (alternateFunctionNumber << position);
		}
	}
}

void setPin(GPIO_TypeDef *port, PinNumber pinNumber)
{
	if (port == NULL)
		return;
	
	port->BSRR |= (uint32_t)pinNumber;
}

void resetPin(GPIO_TypeDef *port, PinNumber pinNumber)
{
	if (port == NULL)
		return;
	
	port->BSRR |= ((uint32_t)pinNumber << 16);
}
