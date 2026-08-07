#include "servo.h"

#include "hardware_config.h"

#include "esp_log.h"
#include "iot_servo.h"

static const char *TAG = "SERVO";

static servo_handle_t servo = NULL;

static float current_angle = SERVO_CLOSE_ANGLE;

esp_err_t servo_init(void)
{
    servo_config_t config = SERVO_CONFIG_DEFAULT(
        LEDC_LOW_SPEED_MODE,
        LEDC_TIMER_0,
        LEDC_CHANNEL_0,
        SERVO_GPIO);

    ESP_ERROR_CHECK(iot_servo_new(&config, &servo));

    current_angle = SERVO_CLOSE_ANGLE;

    ESP_LOGI(TAG, "Servo initialized");

    return servo_close();
}

esp_err_t servo_set_angle(float angle)
{
    current_angle = angle;

    ESP_ERROR_CHECK(iot_servo_write_angle(servo, angle));

    ESP_LOGI(TAG, "Angle -> %.1f°", angle);

    return ESP_OK;
}

esp_err_t servo_open(void)
{
    return servo_set_angle(SERVO_OPEN_ANGLE);
}

esp_err_t servo_close(void)
{
    return servo_set_angle(SERVO_CLOSE_ANGLE);
}

float servo_get_angle(void)
{
    return current_angle;
}

esp_err_t servo_deinit(void)
{
    ESP_ERROR_CHECK(iot_servo_del(servo));

    servo = NULL;

    ESP_LOGI(TAG, "Servo deinitialized");

    return ESP_OK;
}