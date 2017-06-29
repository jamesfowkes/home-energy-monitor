#ifndef _BUFFER_H_
#define _BUFFER_H_

class SerialBuffer
{
public:
	SerialBuffer() : m_buf(NULL), m_index(0), m_length(0) {}
	SerialBuffer(char * buf, uint8_t length) :
		m_buf(buf), m_index(0), m_length(length) {}

	void init(char * buf, uint8_t length);

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
