/*
 * CircularBuffer.h
 *
 * A static (no-heap) circular buffer template.
 * Used to safely store incoming UART data from interrupts
 * until the main loop reads it.
 */

#ifndef INC_CIRCULARBUFFER_H_
#define INC_CIRCULARBUFFER_H_

#include "main.h"
#include <string.h>

template <class T, uint16_t N>
class CircularBuffer {
public:
    // Constructor: initialize indices and clear memory
    CircularBuffer() : _in_pos(0), _out_pos(0), _stored(0) {
        memset(_buffer, 0, sizeof(_buffer));
    }

    // Add "size" elements into the buffer
    void put(const T *data, uint16_t size) {
        for (uint16_t i = 0; i < size; i++) {
            _buffer[_in_pos++] = data[i];      // write data
            if (_in_pos >= N) _in_pos = 0;     // wrap-around

            if (_stored < N) {
                _stored++;                     // increment count
            } else {
                // If full: overwrite oldest data
                _out_pos++;
                if (_out_pos >= N) _out_pos = 0;
            }
        }
    }

    // Read up to "size" elements from the buffer
    uint16_t pull(T *data, uint16_t size) {
        uint16_t read_size = 0;
        for (uint16_t i = 0; i < size; i++) {
            if (_stored == 0) break;           // no more data
            data[i] = _buffer[_out_pos++];     // read element
            if (_out_pos >= N) _out_pos = 0;   // wrap-around
            _stored--;                         // decrease stored count
            read_size++;
        }
        return read_size;
    }

    // Return how many elements are stored
    uint16_t count() const { return _stored; }

    // Return buffer capacity
    uint16_t capacity() const { return N; }

    // Clear entire buffer
    void flush() {
        _in_pos = 0;
        _out_pos = 0;
        _stored = 0;
        memset(_buffer, 0, sizeof(_buffer));
    }

private:
    T _buffer[N];          // Static array (no heap)
    uint16_t _in_pos;      // Write index
    uint16_t _out_pos;     // Read index
    uint16_t _stored;      // Number of elements currently stored
};

#endif /* INC_CIRCULARBUFFER_H_ */
