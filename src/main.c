#include "initialization.h"
#include "dac-samples.h"
#include "generator.h"

int main(void)
{
	mcuInitialization();
	
	calculateSamples(1000);
	
    runGenerator();
}
