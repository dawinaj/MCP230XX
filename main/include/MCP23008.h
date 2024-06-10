#ifndef MCP23XXX_H
#define MCP23XXX_H

#include <esp_log.h>
#include <esp_check.h>

#include "i2c_manager.h"

//

class MCP23008
{
	static constexpr const char *const TAG = "MCP230xx";

	i2c_port_t port; // I2C_NUM_0 or I2C_NUM_1
	uint8_t address; // Hardware address of the device

public:
	uint8_t gpio = 0;

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
	MCP23008(i2c_port_t p, uint8_t a = 0b000) : port(p), address((a & 0b111) | 0b0100000), gpio(0)
	{
		ESP_LOGI(TAG, "Constructed with port: %d, address: %d", port, address);
	}
	~MCP23008()
	{
		ESP_LOGI(TAG, "Destructed with port: %d, address: %d", port, address);
	}

	esp_err_t init(bool out = false)
	{
		if (out)
			ESP_RETURN_ON_ERROR(
				set_direction(0),
				TAG, "Failed to set_direction!");

		ESP_RETURN_ON_ERROR(
			set_pins(),
			TAG, "Failed to set_pins!");
		return ESP_OK;
	}
	esp_err_t deinit()
	{
		gpio = 0;
		ESP_RETURN_ON_ERROR(
			set_pins(),
			TAG, "Failed to set_pins!");
		return ESP_OK;
	}

	//
	esp_err_t set_pins(uint8_t val)
	{
		gpio = val;
		ESP_RETURN_ON_ERROR(
			set_pins(),
			TAG, "Failed to set_pins!");
		return ESP_OK;
	}
	esp_err_t get_pins(uint8_t &val)
	{
		ESP_RETURN_ON_ERROR(
			get_pins(),
			TAG, "Failed to get_pins!");
		val = gpio;
		return ESP_OK;
	}

	esp_err_t set_pins()
	{
		return write_reg(Register::OLAT, gpio);
	}
	esp_err_t get_pins()
	{
		return read_reg(Register::GPIO, gpio);
	}

	//

	// When a bit is set, the corresponding pin becomes an input. When a bit is clear, the corresponding pin becomes an output.
	esp_err_t set_direction(uint8_t val)
	{
		return write_reg(Register::IODIR, val);
	}
	// esp_err_t get_direction(uint8_t &val)
	// {
	// 	return read_reg(Register::IODIR, val);
	// }

	// If a bit is set, the corresponding GPIO register bit will reflect the inverted value on the pin.
	esp_err_t set_input_polarity(uint8_t val)
	{
		return write_reg(Register::IPOL, val);
	}
	// esp_err_t get_input_polarity(uint8_t &val)
	// {
	// 	return read_reg(Register::IPOL, val);
	// }

	// If a bit is set, the corresponding pin is enabled for interrupt-on-change.
	// The DEFVAL and INTCON registers must also be configured if any pins are enabled for interrupt-on-change.
	esp_err_t set_interrupt_enabled(uint8_t val)
	{
		return write_reg(Register::GPINTEN, val);
	}
	// esp_err_t get_interrupt_enabled(uint8_t &val)
	// {
	// 	return read_reg(Register::GPINTEN, val);
	// }

	// The INTCON register controls how the associated pin value is compared for the interrupt-on-change feature.
	// If a bit is set, the corresponding I/O pin is compared against the associated bit in the DEFVAL register.
	// If a bit value is clear, the corresponding I/O pin is compared against the previous value.
	esp_err_t set_interrupt_control(uint8_t val)
	{
		return write_reg(Register::INTCON, val);
	}
	// esp_err_t get_interrupt_control(uint8_t &val)
	// {
	// 	return read_reg(Register::INTCON, val);
	// }

	// The default comparison value is configured in the DEFVAL register.
	// If enabled (via GPINTEN and INTCON) to compare against the DEFVAL register, an opposite value on the associated pin will cause an interrupt to occur.
	esp_err_t set_default(uint8_t val)
	{
		return write_reg(Register::DEFVAL, val);
	}
	// esp_err_t get_default(uint8_t &val)
	// {
	// 	return read_reg(Register::DEFVAL, val);
	// }

	// The IOCON register contains several bits for configuring the device: ...
	// No idea what the hell all that even does
	esp_err_t set_config(bool s, bool d, bool o, bool i)
	{
		uint8_t val = s << 5 | d << 4 | o << 2 | i << 1;
		return write_reg(Register::IOCON, val);
	}
	esp_err_t set_config(uint8_t val)
	{
		return write_reg(Register::IOCON, val);
	}
	// esp_err_t get_config(uint8_t &val)
	// {
	// 	return read_reg(Register::IOCON, val);
	// }

	// If a bit is set and the corresponding pin is configured as an input, the corresponding PORT pin is internally pulled up with a 100 kOhm resistor.
	esp_err_t set_pullup(uint8_t val)
	{
		return write_reg(Register::GPPU, val);
	}
	// esp_err_t get_pullup(uint8_t &val)
	// {
	// 	return read_reg(Register::GPPU, val);
	// }

	// A 'set' bit indicates that the associated pin caused the interrupt.
	// This register is 'read-only'. Writes to this register will be ignored.
	esp_err_t read_interrupt_flag(uint8_t &val)
	{
		return read_reg(Register::INTF, val);
	}

	// The register is 'read-only' and is updated only when an interrupt occurs.
	// The register will remain unchanged until the interrupt is cleared via a read of INTCAP or GPIO.
	esp_err_t read_interrupt_captured(uint8_t &val)
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
	void set_bit(uint8_t b)
	{
		gpio |= (1 << b);
	}
	void clear_bit(uint8_t b)
	{
		gpio &= ~(1 << b);
	}
	void flip_bit(uint8_t b)
	{
		gpio ^= (1 << b);
	}

	// helpers
private:
	esp_err_t read_reg(Register reg, uint8_t &d)
	{
		uint8_t r = static_cast<uint8_t>(reg);
		ESP_RETURN_ON_ERROR(
			i2c_manager_read(port, address, r, &d, 1),
			TAG, "Failed to i2c_manager_read! Reg: %d", r);
		return ESP_OK;
	}

	esp_err_t write_reg(Register reg, uint8_t d)
	{
		uint8_t r = static_cast<uint8_t>(reg);
		ESP_RETURN_ON_ERROR(
			i2c_manager_write(port, address, r, &d, 1),
			TAG, "Failed to i2c_manager_write! Reg: %d, Data: %d", r, d);
		return ESP_OK;
	}
};

#endif
