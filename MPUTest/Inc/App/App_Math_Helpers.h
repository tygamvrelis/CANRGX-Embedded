/**
 * @file App_Math_Helpers.h
 * @author Tyler
 *
 * @defgroup Math_Helpers_Header Header
 * @ingroup Math_Helpers
 * @{
 */

#ifndef APP_APP_MATH_HELPERS_H_
#define APP_APP_MATH_HELPERS_H_




/********************************** Includes *********************************/
#include <math.h>
#include <stdint.h>




/********************************* Functions *********************************/
float trapezoid(uint32_t time, uint32_t period, float percentOn, float amplitude);
float acTrapezoid(uint32_t time, uint32_t period, float percentOn, float amplitude);

/**
 * @}
 */
/* end - Math_Helpers_Header */

#endif /* APP_APP_MATH_HELPERS_H_ */
