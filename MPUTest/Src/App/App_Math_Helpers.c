/**
 * @file App_Math_Helpers.c
 * @author Tyler
 * @brief Math helper functions for control
 *
 * @defgroup Math_Helpers Math helpers
 * @ingroup Control
 * @brief Helper functions for generating periodic waveforms
 * @{
 */




/********************************** Includes *********************************/
#include "App/App_Math_Helpers.h"




/***************************** Public Functions ******************************/

/**
 * @brief  Function with which a periodic trapezoidal waveform can be evaluated
 * @param  time The input time at which the function is to be evaluated
 * @param  period The period of the trapezoid in ms
 * @param  percentOn The % of the period for which the amplitude is constant
 *         Value must be in range [0.0, 100.0]
 * @param  amplitude The amplitude in [-1.0, 1.0]
 * @return The value of the function given valid parameters, otherwise NAN. The
 *         return value will always be in range [-1.0, 1.0] given valid
 *         parameters
 */
float trapezoid(uint32_t time, uint32_t period, float percentOn, float amplitude){
    if(percentOn < 0 || percentOn > 100.0 ||
       amplitude < -1.0 || amplitude > 1.0
    )
    {
        return NAN;
    }

    uint32_t n = time % period;
    float t_rise = (1 - percentOn / 100.0) * period / 2;
    float t_fall_start = period - t_rise;
    float retval;

    if(n < t_rise){
        retval = n / t_rise;
    }
    else if(n >= t_rise && n <= t_fall_start){
        retval = 1.0;
    }
    else{
        float b = period / t_rise; // y = m*x + b
        retval = b - (n / t_rise);
    }

    return amplitude * retval;
}

/**
 * @brief   Implements an AC trapezoid
 * @details The waveform looks something like this over 1 period, centered about
 *          the value 0:
 *
 * @verbatim
 *           /--------\
 *          /          \
 *                      \          /
 *                       \--------/
 * @endverbatim
 *
 * @param  time The input time at which the function is to be evaluated
 * @param  period The period of the whole AC form in ms
 * @param  percentOn The % of the period for which the amplitude is constant
 *         Value must be in range [0.0, 100.0]
 * @param  amplitude The amplitude of the first "peak" in [-1.0, 1.0] (the
 *         amplitude of the second "peak" will be the opposite sign)
 * @return The value of the function given valid parameters, otherwise NAN. The
 *         return value will always be in range [-1.0, 1.0] given valid
 *         parameters
 */
float acTrapezoid(uint32_t time, uint32_t period, float percentOn, float amplitude){
    uint32_t n = time % period;

    if(n < period / 2){
        return trapezoid(time, period / 2, percentOn, amplitude);
    }
    else{
        return trapezoid(time, period / 2, percentOn, -1.0 * amplitude);
    }
}

/**
 * @}
 */
/* end - Math_Helpers */
