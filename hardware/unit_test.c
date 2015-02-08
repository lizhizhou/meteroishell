/*
 * unit_test.c
 *
 *  Created on: Apr 30, 2013
 *      Author: lizhizhou
 */
#include <stdio.h>
#include <unistd.h>
#include "platform.h"
#include "../lib/coordinate.h"
#include "led.h"

void AD7940_test()
{
	int i;
	AD7490_init(ADC7490_0);
    while(1)
    {
        sleep(1);
        for(i = 0; i < 16; i++)
        	printf("ADC channel %d voltage is %f data %d\n", i,
        			AD7490_get_voltage(ADC7490_0, i), AD7490_get_data(ADC7490_0, i));
    }
}

void AM2301_test()
{
    AM2301_init(AM2301_0);
    AM2301_init(AM2301_1);
    while(1)
    {
        sleep(1);
        printf("Temperature is %.2f C", AM2301_get_temperature(AM2301_0));
        printf("\tTemperature is %.2f C\n", AM2301_get_temperature(AM2301_1));
        printf("Moisture is %.2f%%", AM2301_get_moisture(AM2301_0));
        printf("\tMoisture is %.2f%% \n", AM2301_get_moisture(AM2301_1));
    }
}

void step_motmor_test()
{
    int i;
    step_motor_init(STEP_MOTOR_0, 50000, 20);
    step_motor_init(STEP_MOTOR_1, 50000, 20);
    step_motor_init(STEP_MOTOR_2, 50000, 20);
    step_motor_init(STEP_MOTOR_3, 10000, 30);
    for(i = 0; i < 1000; i++) {
        step_motor_move_step_forward(STEP_MOTOR_0);
        step_motor_move_step_forward(STEP_MOTOR_1);
        step_motor_move_step_forward(STEP_MOTOR_2);
        step_motor_move_step_forward(STEP_MOTOR_3);
    }
    for(i = 0; i < 1000; i++) {
        step_motor_move_step_back(STEP_MOTOR_0);
        step_motor_move_step_back(STEP_MOTOR_1);
        step_motor_move_step_back(STEP_MOTOR_2);
        step_motor_move_step_back(STEP_MOTOR_3);
    }

}

void brush_motor_test()
{
    brush_motor_init(BRUSH_MOTOR_0, 1000, 30);
    brush_motor_init(BRUSH_MOTOR_1, 1000, 30);
    brush_motor_back(BRUSH_MOTOR_1);
    brush_motor_init(BRUSH_MOTOR_2, 1000, 30);
    brush_motor_init(BRUSH_MOTOR_3, 1000, 30);
}

void PID_test()
{
    int i;
    float target =100;
    float current = 0;
    float current_d =0;
    float current_d_d = 0;
    float error;
    float error_d;
    float error_d_d;
    for(i=0; i < 100; i++)
    {
        current_d_d = current_d;
        current_d = current;
        error_d_d = error_d;
        error_d = error;
        error = target - current_d_d;
        current = current + PID(error, error_d, error_d_d, 0.2, 0.01, 0.03);
        printf("current is %f\n",current);
    }
}

void PIO_test()
{
    IOA_OE      = 0xffffffff;  // init IO
    IOB_OE      = 0xffffffff;  // init IO
    IOA_IO_0    = 0;
    IOB_IO_0    = 1;
    IOA_IO_1    = 0;
    IOB_IO_1    = 1;
}

void subdivision_step_motor_test()
{
	int i;
	step_motor_init(SUBDIVISION_STEP_MOTOR_0, 10000, 30);
	step_motor_move_step_back(SUBDIVISION_STEP_MOTOR_0);
	while(1) {
		for (i = 0; i< 64 ; i++) {
			setp_motor_subdivision(SUBDIVISION_STEP_MOTOR_0, i, 64 ,30);
			usleep(100000);
		}
		for (i = 64; i >=0 ; i--) {
			setp_motor_subdivision(SUBDIVISION_STEP_MOTOR_0, i, 64 ,30);
			usleep(100000);
		}
	}
}

void coordinate_test()
{
    rectangular r;
    cylindroid  c;
    spheroid    s;
    r.x = 100;
    r.y = 100;
    r.z = 100;
    c = rectangular_to_cylindroid(r);
    r = cylindroid_to_rectangular(c);
    printf("x = %f, y = %f, z = %f\n", r.x, r.y, r.z);
    s = rectangular_to_spheroid(r);
    r = spheroid_to_rectangular(s);
    printf("x = %f, y = %f, z = %f\n", r.x, r.y, r.z);
    s = cylindroid_to_spheroid(c);
    c = spheroid_to_cylindroid(s);
    r = cylindroid_to_rectangular(c);
    printf("x = %f, y = %f, z = %f\n", r.x, r.y, r.z);
    printf("30 = %f", radian_to_angle(angle_to_radian(30)));
}

void led_unit_test(void)
{
	set_rgb_led(LED_0, 255, 0 , 0);
	set_rgb_led(LED_1, 0, 255 , 0);
	set_rgb_led(LED_2, 0, 0 , 255);
}

#define MSE_DATA       *((volatile int*) MSE_BASE)
#define MSE_DATA1      *((volatile int*) MSE_BASE+1)
#define MSE_DATA2      *((volatile int*) MSE_BASE+2)
#define MSE_DATA3      *((volatile int*) MSE_BASE+3)
#define MSE_DATA4      *((volatile int*) MSE_BASE+4)
#define MSE_DATA5      *((volatile int*) MSE_BASE+5)
#define TEST_REG_DATA  *((volatile int*) TEST_REG)

void qsys_serial_test(void)
{
	int i;
//	MSE_DATA4 = 30;
//	MSE_DATA5 = 35;
//	return;
//	while(1){
//		MSE_DATA4 = i;
//		MSE_DATA5 = i;
//		TEST_REG_DATA = i;
//		printf("The MSE_DATA is 0x%x\n",   MSE_DATA);
//		printf("The MSE_DATA1 is 0x%x\n",  MSE_DATA1);
//		printf("The MSE_DATA2 is 0x%x\n",  MSE_DATA2);
//		printf("The MSE_DATA3 is 0x%x\n",  MSE_DATA3);
//		printf("The MSE_DATA4 is 0x%x\n",  MSE_DATA4);
//		printf("The MSE_DATA5 is 0x%x\n",  MSE_DATA5);
//		i++;
//	}

	while(i<10000){
		//mode 1
		MSE_DATA4 = i;
		MSE_DATA5 = i;
		TEST_REG_DATA = i;
		if(MSE_DATA  != 0xea680001){printf("MSE_DATA 0x%x error", MSE_DATA); break;}
		if(MSE_DATA1 != 0){printf("MSE_DATA1 0x%x error", MSE_DATA1); break;}
		if(MSE_DATA2 != 0xa5a5a5a5){printf("MSE_DATA2 0x%x error", MSE_DATA2); break;}
		if(MSE_DATA3 != 0x5a5a5a5a){printf("MSE_DATA3 0x%x error", MSE_DATA3); break;}
		if(MSE_DATA4 != i){printf("MSE_DATA4 %d error", MSE_DATA4); break;}
		usleep(50);
		if(MSE_DATA5 != i){printf("MSE_DATA5 %d error", MSE_DATA5); break;}
		if(TEST_REG_DATA != i){printf("TEST_REG_DATA  %d error", TEST_REG_DATA); break;}
		//mode 2
		MSE_DATA4 = i;
		if(MSE_DATA  != 0xea680001){printf("MSE_DATA 0x%x error", MSE_DATA); break;}
		if(MSE_DATA1 != 0){printf("MSE_DATA1 0x%x error", MSE_DATA1); break;}
		TEST_REG_DATA = i;
		if(MSE_DATA2 != 0xa5a5a5a5){printf("MSE_DATA2 0x%x error", MSE_DATA2); break;}
		if(MSE_DATA3 != 0x5a5a5a5a){printf("MSE_DATA3 0x%x error", MSE_DATA3); break;}
		if(MSE_DATA4 != i){printf("MSE_DATA4 0x%x error", MSE_DATA4); break;}
		if(TEST_REG_DATA != i){printf("TEST_REG_DATA  0x%x error", TEST_REG_DATA); break;}
		if(MSE_DATA5 != i){printf("MSE_DATA5 %d error", MSE_DATA5); break;}
		i++;
	}
	printf("i = %d", i);
	sleep(1);
}
