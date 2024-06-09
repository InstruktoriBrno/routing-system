#pragma once
#include <cstdint>
#include <span.hpp>

class SerializationBuffer {
    tcb::span<uint8_t> _buffer;
    int _size;
public:
    SerializationBuffer(tcb::span<uint8_t> buffer) : _buffer(buffer), _size(0) {}

    template<typename T>
    void push(const T& value) {
        if (_size + sizeof(T) > _buffer.size()) {
            return;
        }

        memcpy(_buffer.data() + _size, &value, sizeof(T));
        _size += sizeof(T);
    }

    int size() const {
        return _size;
    }

    uint8_t *buffer() {
        return _buffer.data();
    }
};

class DeserializationBuffer {
    tcb::span<uint8_t> _buffer;
    int _offset;
public:
    DeserializationBuffer(tcb::span<uint8_t> buffer) : _buffer(buffer), _offset(0) {}

    template <typename T>
    T pop() {
        assert(_offset + sizeof(T) <= _buffer.size());

        T t;
        pop(t);
        return t;
    }

    template <typename T>
    void pop(T& value) {
        assert(_offset + sizeof(T) <= _buffer.size());

        memcpy(&value, _buffer.data() + _offset, sizeof(T));
        _offset += sizeof(T);
    }
};

class COBSEncoder {
public:
    COBSEncoder(tcb::span<uint8_t> out_buffer)
        : _out_buffer(out_buffer)
        , _write_it(_out_buffer.begin() + 1)
        , _code_it(_out_buffer.begin())
        , _code(1)
        , _failed(false) {
        if (out_buffer.size() < 2) {
            _failed = true;
        }
    }

    void push(tcb::span<const uint8_t> input) {
        for (uint8_t c : input) {
            _ensure_in_bounds();
            if (_failed) {
                return;
            }

            if (c != 0) {
                *_write_it = c;
                _write_it++;
                _code++;
            }

            if (c == 0 || _code == 255) {
                *_code_it = _code;
                _code = 1;
                _code_it = _write_it;
                _write_it++;
            }
        }
    }

    template <typename T>
    void push(const T &item) {
        push(tcb::span<const uint8_t>(
            reinterpret_cast<const uint8_t *>(&item), sizeof(item)));
    }

    std::size_t finish() {
        _ensure_in_bounds();
        if (_failed) {
            return 0;
        }

        *_code_it = _code;
        *_write_it = 0;

        return (_write_it - _out_buffer.begin()) + 1;
    }

    bool good() const { return !_failed; }

private:
    void _ensure_in_bounds() {
        if (_write_it + 1 > _out_buffer.end()) {
            _failed = true;
        }
    }

    tcb::span<uint8_t> _out_buffer;

    tcb::span<uint8_t>::iterator _write_it;
    tcb::span<uint8_t>::iterator _code_it;
    uint8_t _code;
    bool _failed;
};

class COBSDecoder {
public:
    COBSDecoder(tcb::span<const uint8_t> in_buffer)
        : _read_it(in_buffer.begin())
        , _end_it(in_buffer.end())
        , _block_size(0)
        , _code(0xff) {}

    template <typename Iterator> bool pop(Iterator write_it, std::size_t limit) {
        while (limit != 0) {
            if (_read_it == _end_it) {
                return false;
            }

            if (_block_size != 0) {
                *write_it = *_read_it;
                write_it++;
                limit--;
                _read_it++;
            } else {
                // Fetch a new block
                _block_size = *_read_it;
                _read_it++;
                if (_block_size && (_code != 0xff)) {
                    *write_it = 0;
                    write_it++;
                    limit--;
                }
                _code = _block_size;
                if (_code == 0) {
                    return false;
                }
            }

            _block_size--;
        }
        return true;
    }

    bool pop(tcb::span<uint8_t> write_buffer) {
        return pop(write_buffer.begin(), write_buffer.size());
    }

    template <typename T> bool pop(T &t) {
        return pop(tcb::span{ reinterpret_cast<uint8_t *>(&t), sizeof(T) });
    }

private:
    tcb::span<const uint8_t>::iterator _read_it;
    tcb::span<const uint8_t>::iterator _end_it;

    uint8_t _block_size;
    uint8_t _code;
};

