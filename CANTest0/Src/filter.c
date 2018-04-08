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

#include <filter.h>
#include <stdlib.h> // For malloc/free
#include <string.h> // For memset

/*static const float32_t FilterTemp_coefficients[5] =
{
// Scaled for floating point
    0.0002783747986899248, 0.0005567495973798496, 0.0002783747986899248, 1.954630185443905, -0.9557431221349063// b0, b1, b2, a1, a2

};*/
const float32_t FilterTemp_coefficients[5] =
{
// Scaled for floating point

    0.0002499595769283905, 0.000499919153856781, 0.0002499595769283905, 1.9554710490783258, -0.9564713740685975// b0, b1, b2, a1, a2

};
static const int FilterTemp_numStages = 1;

 void FilterTemp_init( FilterTempType * pThis )
{
	arm_biquad_cascade_df1_init_f32(	&pThis->instance, FilterTemp_numStages, FilterTemp_coefficients, pThis->state );
	FilterTemp_reset( pThis );

}

 void FilterTemp_reset( FilterTempType * pThis )
{
	memset( &pThis->state, 0, sizeof( pThis->state ) ); // Reset state to 0
	pThis->output = 0;									// Reset output
}
void FilterTemp_writeInput( FilterTempType * pThis, float input)
{
	arm_biquad_cascade_df1_f32( &pThis->instance, &input, &pThis->output, 1);
}
 int FilterTemp_filterBlock( FilterTempType * pThis, float * pInput, float * pOutput, unsigned int count )
{
	arm_biquad_cascade_df1_f32( &pThis->instance, pInput, pOutput, count );
	return count;

}

