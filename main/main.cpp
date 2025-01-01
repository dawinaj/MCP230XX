// #define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>
#include <esp_check.h>

#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <driver/i2c_master.h>

#include "MCP230XX.h"

#include "bitwise_print.h"

static const char *TAG = "Test";

i2c_master_bus_handle_t init_i2c()
{
	i2c_master_bus_config_t bus_cfg = {
		.i2c_port = -1,
		.sda_io_num = GPIO_NUM_21,
		.scl_io_num = GPIO_NUM_22,
		.clk_source = I2C_CLK_SRC_DEFAULT,
		.glitch_ignore_cnt = 7,
		.intr_priority = 0,
		.trans_queue_depth = 0,
		.flags = {
			.enable_internal_pullup = true,
		},
	};

	i2c_master_bus_handle_t bus_handle;
	ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus_handle));

	return bus_handle;
}

//

extern "C" void app_main(void)
{
	esp_log_level_set("*", ESP_LOG_DEBUG); // ESP_LOG_VERBOSE
	
	i2c_master_bus_handle_t i2c_host = init_i2c();

	MCP23017 expander(i2c_host);

	expander.init();

	uint16_t pins;
	while (1)
	{
		expander.get_pins(pins);

		ESP_LOGW(TAG, PRIbit16, PRItobit16(pins));
	}
}
