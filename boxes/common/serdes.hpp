#pragma once
#include "logging.hpp"
#include <cstdint>
#include <span.hpp>
#include <vector>

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

    tcb::span<uint8_t> span() {
        return _buffer.subspan(0, _size);
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

    tcb::span<uint8_t> remaining() {
        return _buffer.subspan(_offset);
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

class Base64 {
public:
    static const std::string& get_base64_chars() {
        static const std::string base64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";
        return base64_chars;
    }

    static const std::vector<int>& get_base64_index() {
        static const std::vector<int> base64_index = [](){
            std::vector<int> index(256, -1);
            for (int i = 0; i < 64; ++i) {
                index[Base64::get_base64_chars()[i]] = i;
            }
            return index;
        }();
        return base64_index;
    }
};

template <typename OutputFunction>
class Base64Encoder {
public:
    explicit Base64Encoder(OutputFunction output_func)
        : output_func_(output_func), buffer_(), buffer_length_(0) {}

    void push_byte(uint8_t byte) {
        buffer_ = (buffer_ << 8) | byte;
        buffer_length_ += 8;

        while (buffer_length_ >= 6) {
            buffer_length_ -= 6;
            uint8_t b64_char_index = (buffer_ >> buffer_length_) & 0x3F;
            output_func_(Base64::get_base64_chars()[b64_char_index]);
        }
    }

    void finalize() {
        if (buffer_length_ > 0) {
            buffer_ <<= (6 - buffer_length_);
            uint8_t b64_char_index = buffer_ & 0x3F;
            output_func_(Base64::get_base64_chars()[b64_char_index]);

            while (buffer_length_ < 6) {
                output_func_('=');
                buffer_length_ += 2;
            }
        }
    }

    // This makes the encoder compatible with the serialization buffer
    template<typename T>
    void push(const T& value) {
        for (int i = 0; i != sizeof(value); i++) {
            push_byte(reinterpret_cast<const uint8_t*>(&value)[i]);
        }
    }


private:
    OutputFunction output_func_;
    uint32_t buffer_;
    int buffer_length_;
};

template <typename OutputFunction>
class Base64Decoder {
public:
    explicit Base64Decoder(OutputFunction output_func)
        : output_func_(output_func), buffer_(0), buffer_length_(0), pad_count_(0) {}

    void push_byte(uint8_t byte) {
        if (byte == '=') {
            pad_count_++;
            return;
        }

        int index = Base64::get_base64_index()[byte];
        if (index == -1) {
            rg_log_e("B54", "Invalid base64 character %c", byte);
            throw std::invalid_argument("Invalid base64 character");
        }

        buffer_ = (buffer_ << 6) | index;
        buffer_length_ += 6;

        while (buffer_length_ >= 8) {
            buffer_length_ -= 8;
            uint8_t decoded_byte = (buffer_ >> buffer_length_) & 0xFF;
            output_func_(decoded_byte);
        }
    }

    void finalize() {
        if (pad_count_ > 0) {
            buffer_ <<= (6 * pad_count_);
            buffer_length_ -= (8 * pad_count_);
        }

        while (buffer_length_ >= 8) {
            buffer_length_ -= 8;
            uint8_t decoded_byte = (buffer_ >> buffer_length_) & 0xFF;
            output_func_(decoded_byte);
        }
    }

private:
    OutputFunction output_func_;
    uint32_t buffer_;
    int buffer_length_;
    int pad_count_;
};
