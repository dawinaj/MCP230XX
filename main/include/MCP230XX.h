#ifndef MCP230XX_H
#define MCP230XX_H

#include <array>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>
#include <esp_check.h>

#include <driver/i2c_master.h>

#include "bitwise_print.h"

//

template <typename intT>
class MCP230XX
{
	static constexpr const char *const TAG = "MCP230xx";

public:
	static constexpr size_t BYTES = sizeof(intT);

private:
	i2c_master_bus_handle_t i2c_host;
	uint16_t address;
	uint32_t clk_hz;
	i2c_master_dev_handle_t i2c_hdl = nullptr;

public:
	intT gpio = 0;

private:
	enum class Register : uint8_t
	{
		IODIR = 0x00,
		IPOL = 0x01,
		GPINTEN = 0x02,
		DEFVAL = 0x03,
		INTCON = 0x04,
		IOCON = 0x05,
		GPPU = 0x06,
		INTF = 0x07,
		INTCAP = 0x08,
		GPIO = 0x09,
		OLAT = 0x0A,
	};

public:
	MCP230XX(i2c_master_bus_handle_t ih, uint16_t a = 0b000, uint32_t chz = 400'000) : i2c_host(ih), address((a & 0b111) | 0b0100000), clk_hz(chz) //
	{
		ESP_LOGI(TAG, "Constructed with address: %d", address); // port: %d, , ih
	}

	~MCP230XX() = default;

	esp_err_t init()
	{
		assert(!i2c_hdl);

		i2c_device_config_t dev_cfg = {
			.dev_addr_length = I2C_ADDR_BIT_LEN_7,
			.device_address = address,
			.scl_speed_hz = clk_hz,
			.scl_wait_us = 0,
			.flags = {
				.disable_ack_check = false,
			},
		};

		ESP_RETURN_ON_ERROR(
			i2c_master_bus_add_device(i2c_host, &dev_cfg, &i2c_hdl),
			TAG, "Failed to i2c_master_bus_add_device!");

		ESP_RETURN_ON_ERROR(
			set_config(),
			TAG, "Failed to set_config!");

		return ESP_OK;
	}
	esp_err_t deinit()
	{
		assert(i2c_hdl);

		ESP_RETURN_ON_ERROR(
			i2c_master_bus_rm_device(i2c_hdl),
			TAG, "Failed to i2c_master_bus_rm_device!");

		i2c_hdl = nullptr;

		return ESP_OK;
	}

	//
	esp_err_t set_pins(intT val)
	{
		gpio = val;
		ESP_RETURN_ON_ERROR(
			write_pins(),
			TAG, "Failed to write_pins!");
		return ESP_OK;
	}
	esp_err_t get_pins(intT &val)
	{
		ESP_RETURN_ON_ERROR(
			read_pins(),
			TAG, "Failed to read_pins!");
		val = gpio;
		return ESP_OK;
	}

	esp_err_t write_pins()
	{
		return write_reg(Register::OLAT, gpio);
	}
	esp_err_t read_pins()
	{
		return read_reg(Register::GPIO, gpio);
	}

	//

	// When a bit is set, the corresponding pin becomes an input. When a bit is clear, the corresponding pin becomes an output.
	esp_err_t set_direction(intT val)
	{
		return write_reg(Register::IODIR, val);
	}
	// esp_err_t get_direction(intT &val)
	// {
	// 	return read_reg(Register::IODIR, val);
	// }

	// If a bit is set, the corresponding GPIO register bit will reflect the inverted value on the pin.
	esp_err_t set_input_polarity(intT val)
	{
		return write_reg(Register::IPOL, val);
	}
	// esp_err_t get_input_polarity(intT &val)
	// {
	// 	return read_reg(Register::IPOL, val);
	// }

	// If a bit is set, the corresponding pin is enabled for interrupt-on-change.
	// The DEFVAL and INTCON registers must also be configured if any pins are enabled for interrupt-on-change.
	esp_err_t set_interrupt_enabled(intT val)
	{
		return write_reg(Register::GPINTEN, val);
	}
	// esp_err_t get_interrupt_enabled(intT &val)
	// {
	// 	return read_reg(Register::GPINTEN, val);
	// }

	// The INTCON register controls how the associated pin value is compared for the interrupt-on-change feature.
	// If a bit is set, the corresponding I/O pin is compared against the associated bit in the DEFVAL register.
	// If a bit value is clear, the corresponding I/O pin is compared against the previous value.
	esp_err_t set_interrupt_control(intT val)
	{
		return write_reg(Register::INTCON, val);
	}
	// esp_err_t get_interrupt_control(intT &val)
	// {
	// 	return read_reg(Register::INTCON, val);
	// }

	// The default comparison value is configured in the DEFVAL register.
	// If enabled (via GPINTEN and INTCON) to compare against the DEFVAL register, an opposite value on the associated pin will cause an interrupt to occur.
	esp_err_t set_default(intT val)
	{
		return write_reg(Register::DEFVAL, val);
	}
	// esp_err_t get_default(intT &val)
	// {
	// 	return read_reg(Register::DEFVAL, val);
	// }

	// The IOCON register contains several bits for configuring the device: ...
	// INTPOL - interrupt polarity
	// ODR - interrupt as open drain, overrides INTPOL
	// DSSLW - disables SDA line rise monitoring
	// MIRROR - logical OR of interrupt pins
	esp_err_t set_config(bool intpl = false, bool odr = false, bool dsslw = false, bool mirror = false)
	{
		bool bit0 = false; // unused
		bool bank = false; // default, regAs and regBs are intertwined (not banked)
		bool haen = false; // spi hardware address enable
		bool seqop = true; // address pointer does not increment
		uint8_t cfg = (bank << 7) | (mirror << 6) | (seqop << 5) | (dsslw << 4) | (haen << 3) | (odr << 2) | (intpl << 1) | (bit0 << 0);

		return write_reg(Register::IOCON, cfg);
	}

	// If a bit is set and the corresponding pin is configured as an input, the corresponding PORT pin is internally pulled up with a 100 kOhm resistor.
	esp_err_t set_pullup(intT val)
	{
		return write_reg(Register::GPPU, val);
	}
	// esp_err_t get_pullup(intT &val)
	// {
	// 	return read_reg(Register::GPPU, val);
	// }

	// A 'set' bit indicates that the associated pin caused the interrupt.
	// This register is 'read-only'. Writes to this register will be ignored.
	esp_err_t read_interrupt_flag(intT &val)
	{
		return read_reg(Register::INTF, val);
	}

	// The register is 'read-only' and is updated only when an interrupt occurs.
	// The register will remain unchanged until the interrupt is cleared via a read of INTCAP or GPIO.
	esp_err_t read_interrupt_captured(intT &val)
	{
		return read_reg(Register::INTCAP, val);
	}

	// Idk why this would ever be read. And Writing to GPIO writes to this. So anyway, useless.
	// esp_err_t set_output_latch(uint8_t val)
	// {
	// 	return write_reg(Register::OLAT, val);
	// }
	// esp_err_t read_output_latch(uint8_t& val)
	// {
	// 	return read_reg(Register::OLAT, val);
	// }

	// Single bit manipulation
	void set_bit(size_t b)
	{
		gpio |= (1 << b);
	}
	void clear_bit(size_t b)
	{
		gpio &= ~(1 << b);
	}
	void flip_bit(size_t b)
	{
		gpio ^= (1 << b);
	}

	// helpers
private:
	esp_err_t read_reg(Register reg, intT &d)
	{
		std::array txbuf = {
			static_cast<uint8_t>(static_cast<uint8_t>(reg) * BYTES),
		};

		ESP_RETURN_ON_ERROR(
			i2c_master_transmit_receive(i2c_hdl, txbuf.data(), txbuf.size(), static_cast<uint8_t *>(static_cast<void *>(&d)), BYTES, -1),
			TAG, "Failed to i2c_master_transmit_receive!");

		return ESP_OK;
	}

	esp_err_t write_reg(Register reg, intT d)
	{
		std::array<uint8_t, 1 + BYTES> txbuf = {
			static_cast<uint8_t>(static_cast<uint8_t>(reg) * BYTES),
		};

		// Reinterpret d as a byte array
		const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&d);
		std::copy(bytes, bytes + BYTES, txbuf.begin() + 1);

		ESP_RETURN_ON_ERROR(
			i2c_master_transmit(i2c_hdl, txbuf.data(), txbuf.size(), -1),
			TAG, "Failed to i2c_master_transmit!");

		return ESP_OK;
	}
};

using MCP23008 = MCP230XX<uint8_t>;
using MCP23009 = MCP230XX<uint8_t>;

using MCP23016 = MCP230XX<uint16_t>;
using MCP23017 = MCP230XX<uint16_t>;
using MCP23018 = MCP230XX<uint16_t>;

#endif
