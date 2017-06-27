#ifndef _BUFFER_H_
#define _BUFFER_H_

class SerialBuffer
{
public:
	SerialBuffer(char * buf, uint8_t index, uint8_t length) :
		m_buf(buf), m_index(index), m_length(length) {}

	void add_to_buffer(char c);
	void reset();
	char last();
	char &operator[] (int x) { return m_buf[x]; }
private:
	char * m_buf;
	uint8_t m_index;
	uint8_t m_length;
};


#endif
