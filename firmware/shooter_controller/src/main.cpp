#include <Arduino.h>
#include "DShot.h"
#include "AS5048A.h"
#include "SCSCL.h"

#define MOTOR_0_PIN 0
#define MOTOR_1_PIN 2
hw_timer_t *motor_timer = NULL;
dshot_t motor_0;
dshot_t motor_1;

#define ENCODER_MISO 12
#define ENCODER_MOSI 13
#define ENCODER_CLK 14
#define ENCODER_0_PIN 25
#define ENCODER_1_PIN 26
SPIClass encoder_spi(HSPI);
AS5048A encoder_0(&encoder_spi, ENCODER_0_PIN, ENCODER_MISO, ENCODER_MOSI, ENCODER_CLK);
AS5048A encoder_1(&encoder_spi, ENCODER_1_PIN, ENCODER_MISO, ENCODER_MOSI, ENCODER_CLK);

#define SERVO_TX_PIN 15
#define SERVO_RX_PIN 16
HardwareSerial servo_serial(2);
SCSCL servo;

void IRAM_ATTR onTimer()
{
	dshotOutput(motor_0.throttle, motor_0.dshot_packet, motor_0.rmt_send);
	dshotOutput(motor_1.throttle, motor_1.dshot_packet, motor_1.rmt_send);
}

void setup()
{
	Serial.begin(115200);
	Serial.println("init starting");

	// Servos init
	servo_serial.begin(1000000, SERIAL_8N1, SERVO_RX_PIN, SERVO_TX_PIN);
	servo.pSerial = &servo_serial;

	// Encoders init
	encoder_0.begin();
	encoder_1.begin();

	// Motors init
	dshotCreateMotor(MOTOR_0_PIN, &motor_0);
	dshotCreateMotor(MOTOR_1_PIN, &motor_1);

	// Timer init, prescaler 80, divider 500 => 2khz
	motor_timer = timerBegin(0, 80, true);
	timerAttachInterrupt(motor_timer, &onTimer, true);
	timerAlarmWrite(motor_timer, 1000, true);
	timerAlarmEnable(motor_timer);

	// ESC init
	vTaskDelay(500 / portTICK_PERIOD_MS);
	dshotSetThrottle(0, &motor_0);
	dshotSetThrottle(0, &motor_1);
}

void loop()
{
	int throttle = Serial.parseInt();

	if (throttle == 9999)
	{
		Serial.println("stop");
		dshotSetThrottle(0, &motor_0);
		dshotSetThrottle(0, &motor_1);
	}
	else if (throttle > 0)
	{
		Serial.printf("val: %d\n", throttle);
		dshotSetThrottle(throttle, &motor_0);
		dshotSetThrottle(throttle, &motor_1);
	}
}
