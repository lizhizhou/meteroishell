/*
 * lophilo.c
 *
 *  Created on: April 14, 2014
 *      Author: zhizhouli
 */
#include "../hardware/platform.h"
#include "../hardware/PIO26.h"
#include "../hardware/shield_ctrl.h"
#include "../hardware/FPGA.h"
#include "../hardware/AM2301.h"
#include "../hardware/sht1x.h"
#include "../hardware/position_sensor.h"
#include "../hardware/step_motor.h"
#include "../hardware/brush_motor.h"
#include "../hardware/fan_motor.h"
void led(int id, char r, char g, char b)
{
	void* address;
	switch (id) {
		case 0:
			address = LED_0;
			break;
		case 1:
			address = LED_1;
			break;
		case 2:
			address = LED_2;
			break;
		case 3:
			address = LED_3;
			break;
		default:
		    return;
	}
	set_rgb_led(address,r,g,b);
}

void ioa(int id, int value)
{
	IOA_OE |= 0x01 << id;
	if(0x1 & value)
		IOA_DATA |= 0x01 << id;
	else
		IOA_DATA &= ~(0x01 << id);
}

void iob(int id, int value)
{
	IOB_OE |= 0x01 << id;
	if(0x1 & value)
		IOB_DATA |= 0x01 << id;
	else
		IOB_DATA &= ~(0x01 << id);
}

void pwm(int id, int pwm)
{
	//PWM_init();
	void* address;
	switch (id) {
		case 0:
			address = PWM_0;
			break;
		case 1:
			address = PWM_1;
			break;
		case 2:
			address = PWM_2;
			break;
		case 3:
			address = PWM_3;
			break;
		default:
		    return;
	}

}

float temp(int id)
{
	switch (id) {
		case 0:
			return sht1x_get_temperature(SHT1X_0);
		case 1:
			return sht1x_get_temperature(SHT1X_1);
		default:
		    return 0;
	}
}

float mois(int id)
{
	switch (id) {
		case 0:
			return sht1x_get_moisture(SHT1X_0);
		case 1:
			return sht1x_get_moisture(SHT1X_1);
		default:
		    return 0;
	}
}
