// Begin header file, filter.h

#ifndef PID_CONTRL_H_ // Include guards
#define PID_CONTRL_H_

#define ARM_MATH_CM4	// Use ARM Cortex M4
#define __FPU_PRESENT 1		// Does this device have a floating point unit?
#include <arm_math.h>	// Include CMSIS header

// Link with library: libarm_cortexM4_mathL.a (or equivalent)
// Add CMSIS/Lib/GCC to the library search path
// Add CMSIS/Include to the include search path
//extern float32_t FilterTemp_coefficients[5];
//static const int FilterTemp_numStages = 1;




arm_pid_instance_f32 tec_pid_a;
arm_pid_instance_f32 tec_pid_b;

void init_tec_pid(){
	tec_pid_a.Kp=1.0;
	tec_pid_a.Kd=0.1;
	tec_pid_a.Ki=0.001;

	tec_pid_b.Kp=1;
	tec_pid_b.Kd=0.1;
	tec_pid_b.Ki=0.001;

}

#endif // FILTER_H_

