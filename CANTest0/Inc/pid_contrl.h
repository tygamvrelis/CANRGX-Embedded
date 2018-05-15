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


struct TEC_PID_Ctrl{
	arm_pid_instance_f32 tec_pid;
	float current_temp;
	float target_temp;
	float output;
};

void TEC_PID_init(struct TEC_PID_Ctrl *ctrl){
	ctrl->tec_pid.Kp=0.35;
	ctrl->tec_pid.Kd=0.00;
	ctrl->tec_pid.Ki=0.00000;
	arm_pid_init_f32(&(ctrl->tec_pid),1);
	//Provide the PID parameters, and initialize the internal coefficients, and reset internal states.
}
float TEC_PID_update(struct TEC_PID_Ctrl *ctrl, float fTemp_ADC_filtered){
	ctrl->current_temp=(fTemp_ADC_filtered/4096.0*3.3-0.4)/0.0195;
	float output_tmp=arm_pid_f32(&(ctrl->tec_pid),(ctrl->target_temp)-(ctrl->current_temp));
	if (output_tmp>4.0){
			ctrl->output=1;
	}
	else if (output_tmp<0.0){
			ctrl->output=0;
	}
	else{
			float output_tmps=output_tmp*output_tmp;
			ctrl->output=(output_tmps+0.25*output_tmp)/(1+output_tmps);
	}
	//Some transfer function to scale the output approximately between 0 and 1, and have approximately linear slope near the origin.
    //((x^2+0.25x)/(1+x^2)))
	return ctrl->output;
}

#endif // FILTER_H_

