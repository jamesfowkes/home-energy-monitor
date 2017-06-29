#include <stdint.h>
#include <stdlib.h>

#include "buffer.h"

void SerialBuffer::init(char * buf, uint8_t length)
{
	this->m_buf = buf;
	this->m_index = 0;
	this->m_length = length;
}

void SerialBuffer::reset()
{
	this->m_index = 0;
	this->m_buf[0] = '\0';
}

void SerialBuffer::add_to_buffer(char c)
{
	if (this->m_index < this->m_length)
	{
		this->m_buf[this->m_index++] = c;
		this->m_buf[this->m_index] = '\0';
	}
}

char SerialBuffer::last()
{
	return (this->m_index > 0) ? this->m_buf[this->m_index-1] : '\0';
}
