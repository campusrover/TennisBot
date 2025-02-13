#include <Arduino.h>
#include "DShot.h"

void dshotCreateMotor(int pin, dshot_t *motor)
{
	rmt_obj_t *rmt_send;
	if ((rmt_send = rmtInit(pin, true, RMT_MEM_64)) == NULL)
	{
		Serial.println("init sender failed\n");
		return;
	}

	rmtSetTick(rmt_send, 12.5);

	motor->rmt_send = rmt_send;
	motor->throttle = DSHOT_IDLE;
}

void dshotSetThrottle(uint16_t throttle, dshot_t *motor)
{
	if (throttle > 2000 || throttle < 0)
	{
		return;
	}

	motor->throttle = throttle + DSHOT_MIN;
}

void dshotOutput(uint16_t value, rmt_data_t *dshot_packet, rmt_obj_t *rmt_send)
{
	uint16_t packet;

	// telemetry bit
	packet = (value << 1) | 0;

	// https://github.com/betaflight/betaflight/blob/09b52975fbd8f6fcccb22228745d1548b8c3daab/src/main/drivers/pwm_output.c#L523
	int csum = 0;
	int csum_data = packet;
	for (int i = 0; i < 3; i++)
	{
		csum ^= csum_data;
		csum_data >>= 4;
	}
	csum &= 0xf;
	packet = (packet << 4) | csum;

	// durations are for dshot600
	// https://blck.mn/2016/11/dshot-the-new-kid-on-the-block/
	// Bit length (total timing period) is 1.67 microseconds (T0H + T0L or T1H + T1L).
	// For a bit to be 1, the pulse width is 1250 nanoseconds (T1H – time the pulse is high for a bit value of ONE)
	// For a bit to be 0, the pulse width is 625 nanoseconds (T0H – time the pulse is high for a bit value of ZERO)
	for (int i = 0; i < 16; i++)
	{
		if (packet & 0x8000)
		{
			dshot_packet[i].level0 = 1;
			dshot_packet[i].duration0 = 100;
			dshot_packet[i].level1 = 0;
			dshot_packet[i].duration1 = 34;
		}
		else
		{
			dshot_packet[i].level0 = 1;
			dshot_packet[i].duration0 = 50;
			dshot_packet[i].level1 = 0;
			dshot_packet[i].duration1 = 84;
		}
		packet <<= 1;
	}

	rmtWrite(rmt_send, dshot_packet, 16);

	return;
}
