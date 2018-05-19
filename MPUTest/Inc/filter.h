/******************************* SOURCE LICENSE *********************************
Copyright (c) 2015 MicroModeler.

A non-exclusive, nontransferable, perpetual, royalty-free license is granted to the Licensee to
use the following Information for academic, non-profit, or government-sponsored research purposes.
Use of the following Information under this License is restricted to NON-COMMERCIAL PURPOSES ONLY.
Commercial use of the following Information requires a separately executed written license agreement.

This Information is distributed WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

******************************* END OF LICENSE *********************************/

// A commercial license for MicroModeler DSP can be obtained at http://www.micromodeler.com/launch.jsp

// Begin header file, filter.h

#ifndef FILTER_H_ // Include guards
#define FILTER_H_

#define ARM_MATH_CM4	// Use ARM Cortex M4
#include <arm_math.h>	// Include CMSIS header

// Link with library: libarm_cortexM4_mathL.a (or equivalent)
// Add CMSIS/Lib/GCC to the library search path
// Add CMSIS/Include to the include search path
//extern float32_t FilterTemp_coefficients[5];
//static const int FilterTemp_numStages = 1;

typedef struct
{
	arm_biquad_casd_df1_inst_f32 instance;
	float32_t state[4];
	float32_t output;
} FilterTempType;

void FilterTemp_init( FilterTempType * pThis );
void FilterTemp_reset( FilterTempType * pThis );
void FilterTemp_writeInput( FilterTempType * pThis, float input);
#define FilterTemp_readOutput( pThis )  \
	pThis->output


 int FilterTemp_filterBlock( FilterTempType * pThis, float * pInput, float * pOutput, unsigned int count );
#define FilterTemp_outputToFloat( output )  \
	(output)

#define FilterTemp_inputFromFloat( input )  \
	(input)

#endif // FILTER_H_

