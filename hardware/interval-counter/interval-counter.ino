#include "ringbuf.h"

#include "interval.h"
#include "buffer.h"
#include "time.h"

static char tx_serial_buffer[16];

static char s_rx_chars[16];
static SerialBuffer s_rx_buffer(s_rx_chars, 0, 16);

static void send_next_interval()
{
	bool has_data = interval_fifo_pop(tx_serial_buffer);

	if (has_data)
	{
		Serial.println(tx_serial_buffer);
	}
	else
	{
		Serial.println("--");
	}
}

static void print_time()
{
	TIME t;
	time_get(t);
	time_print(t, tx_serial_buffer);
	Serial.println(tx_serial_buffer);
}

static void print_count()
{
	Serial.println(interval_fifo_count());
}

static void handle_query(SerialBuffer& rx_serial_buffer)
{
	switch(rx_serial_buffer[1])
	{
	case 'I':
		send_next_interval();
		break;
	case 'T':
		print_time();
		break;
	case 'C':
		print_count();
		break;
	}
}

static char get_command(SerialBuffer& rx_serial_buffer)
{
	return rx_serial_buffer[0];
}

static char get_setting(SerialBuffer& rx_serial_buffer)
{
	return rx_serial_buffer[1];
}

static bool handle_setting(SerialBuffer& rx_serial_buffer)
{
	bool handled = true;
	switch (get_setting(rx_serial_buffer))
	{
	case 'T':
		handle_timestamp(rx_serial_buffer);
		break;
	case 'F':
		interval_on_pulse();
		break;
	default:
		handled = false;
		break;
	}
	return handled;
}

static void handle_timestamp(SerialBuffer& rx_serial_buffer)
{
	TIME timestamp;
	char * p_timestamp = &rx_serial_buffer[2];
	timestamp.h = (p_timestamp[0]-'0')*10 + (p_timestamp[1]-'0');
	timestamp.m = (p_timestamp[3]-'0')*10 + (p_timestamp[4]-'0');
	timestamp.s = (p_timestamp[6]-'0')*10 + (p_timestamp[7]-'0');
	timestamp.ms = (p_timestamp[9]-'0')*100 + (p_timestamp[10]-'0')*10 + (p_timestamp[11]-'0');
	time_set(timestamp);
}

static void handle_command(SerialBuffer& rx_serial_buffer)
{
	switch (get_command(rx_serial_buffer))
	{
	case '?':
		handle_query(rx_serial_buffer);
		break;
	case '!':
		if (handle_setting(rx_serial_buffer))
		{
			Serial.println("OK");
		}
		else
		{
			Serial.println("NOK");
		}
		break;
	default:
		break;
	}
}

static void handle_serial_char(char c, SerialBuffer& rx_serial_buffer)
{
	rx_serial_buffer.add_to_buffer(c);
}

static void handle_serial(Stream& ser, SerialBuffer& rx_serial_buffer)
{
	while(ser.available())
	{
		handle_serial_char(ser.read(), rx_serial_buffer);
	}
}

static bool command_is_pending(SerialBuffer& rx_serial_buffer)
{
	return rx_serial_buffer.last() == '\n';
}

void setup()
{
	Serial.begin(115200);
	pinMode(2, INPUT_PULLUP);
	pinMode(13, OUTPUT);
	interval_init();

	attachInterrupt(digitalPinToInterrupt(2), interval_on_pulse, FALLING);

	Serial.println("RDY");
	
}

void loop()
{
	handle_serial(Serial, s_rx_buffer);
	if (command_is_pending(s_rx_buffer))
	{
		handle_command(s_rx_buffer);
		s_rx_buffer.reset();
	}

	interval_tick();
	time_tick();
}
