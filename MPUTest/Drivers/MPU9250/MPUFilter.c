/**
 * @file MPUFilter.c
 * @author Tyler
 */

/******************************* SOURCE LICENSE *********************************
Copyright (c) 2018 MicroModeler.

A non-exclusive, nontransferable, perpetual, royalty-free license is granted to the Licensee to
use the following Information for academic, non-profit, or government-sponsored research purposes.
Use of the following Information under this License is restricted to NON-COMMERCIAL PURPOSES ONLY.
Commercial use of the following Information requires a separately executed written license agreement.

This Information is distributed WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

******************************* END OF LICENSE *********************************/

// A commercial license for MicroModeler DSP can be obtained at http://www.micromodeler.com/launch.jsp

#include "MPUFilter.h"

#include <stdlib.h> // For malloc/free
#include <string.h> // For memset

float32_t MPUFilter_coefficients[21] = {-0.018872079, -0.0011102221,
        0.0030367336, 0.014906744, 0.030477359, 0.049086205, 0.070363952,
        0.089952103, 0.10482875, 0.11485946, 0.11869398, 0.11485946, 0.10482875,
        0.089952103, 0.070363952, 0.049086205, 0.030477359, 0.014906744,
        0.0030367336, -0.0011102221, -0.018872079};

void MPUFilter_reset(MPUFilterType * pThis){
    memset(&pThis->state, 0, sizeof(pThis->state)); // Reset state to 0
    pThis->output = 0; // Reset output
}

void MPUFilter_init(MPUFilterType * pThis){
    arm_fir_init_f32(
            &pThis->instance,
            MPUFilter_numTaps,
            MPUFilter_coefficients,
            pThis->state,
            MPUFilter_blockSize
    );

    MPUFilter_reset(pThis);
}

int MPUFilter_filterBlock(
        MPUFilterType * pThis,
        float * pInput,
        float * pOutput,
        unsigned int count
){
    arm_fir_f32(&pThis->instance,
            (float32_t*)pInput,
            (float32_t*)pOutput,
            count
    );
    return count;
}

MPUFilterType azFilter, ayFilter, axFilter;
void initAllMPU9250Filters(void){
    MPUFilter_init(&axFilter);
    MPUFilter_init(&ayFilter);
    MPUFilter_init(&azFilter);
}

void filterAccelMPU9250(MPU9250_t* myMPU9250){
    // Filter the signals along each axis
    MPUFilter_writeInput(&axFilter, myMPU9250->ax);
    myMPU9250->ax = MPUFilter_readOutput(&axFilter);

    MPUFilter_writeInput(&ayFilter, myMPU9250->ay);
    myMPU9250->ay = MPUFilter_readOutput(&ayFilter);

    MPUFilter_writeInput(&azFilter, myMPU9250->az);
    myMPU9250->az = MPUFilter_readOutput(&azFilter);
}
