#ifndef _GPIO_H_
#define _GPIO_H_

#include <stdint.h>

#include "mcu.h"

typedef enum
{
	pinNumber0 = 1,
	pinNumber1 = (pinNumber0 << 1),
	pinNumber2 = (pinNumber1 << 1),
	pinNumber3 = (pinNumber2 << 1),
	pinNumber4 = (pinNumber3 << 1),
	pinNumber5 = (pinNumber4 << 1),
	pinNumber6 = (pinNumber5 << 1),
	pinNumber7 = (pinNumber6 << 1),
	pinNumber8 = (pinNumber7 << 1),
	pinNumber9 = (pinNumber8 << 1),
	pinNumber10 = (pinNumber9 << 1),
	pinNumber11 = (pinNumber10 << 1),
	pinNumber12 = (pinNumber11 << 1),
	pinNumber13 = (pinNumber12 << 1),
	pinNumber14 = (pinNumber13 << 1),
	pinNumber15 = (pinNumber14 << 1),
} PinNumber;

typedef enum
{
	inputPinMode = 0,
	outputPinMode,
	alternateFunctionPinMode,
	analogPinMode,
	pinModeMask = analogPinMode,
} PinMode;

typedef enum
{
	pushPullPinType = 0,
	openDrainPinType,
	pinTypeMask = openDrainPinType,
} PinType;

typedef enum
{
	lowPinSpeed = 0,
	mediumPinSpeed,
	highPinSpeed,
	veryHighPinSpeed,
	pinSpeedMask = veryHighPinSpeed,
} PinSpeed;

typedef enum
{
	noPullsPinPull = 0,
	pullUpPinPull,
	pullDownPinPull,
	pinPullMask = (pullUpPinPull | pullDownPinPull),
} PinPull;

typedef enum
{
	alternateFunction0 = 0,
	alternateFunction1,
	alternateFunction2,
	alternateFunction3,
	alternateFunction4,
	alternateFunction5,
	alternateFunction6,
	alternateFunction7,
	alternateFunction8,
	alternateFunction9,
	alternateFunction10,
	alternateFunction11,
	alternateFunction12,
	alternateFunction13,
	alternateFunction14,
	alternateFunction15,
	alternateFunctionMask = alternateFunction15,
} AlternateFunctionNumber;

typedef struct
{
	PinNumber pinNumber;
	GPIO_TypeDef *port;
	PinMode mode;
	PinType type;
	PinSpeed speed;
	PinPull pull;
	AlternateFunctionNumber alternateFunctionNumber;
} PinSettings;

void initializePin(PinSettings *settings);
void setPin(GPIO_TypeDef *port, PinNumber pinNumber);
void resetPin(GPIO_TypeDef *port, PinNumber pinNumber);

#endif	//	_GPIO_H_
