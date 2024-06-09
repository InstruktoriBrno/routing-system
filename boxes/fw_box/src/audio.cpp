#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include <cstring>
#include <atomic>
#include <driver/dac.h>
#include <driver/timer.h>
#include <freertos/ringbuf.h>
#include <driver/i2s.h>
#include <fstream>

static constexpr int WAV_SAMPLE_RATE = 11025;
static constexpr int BLOCK_SIZE = 128;
static constexpr dac_channel_t DAC_CHANNEL = DAC_CHANNEL_2;

char file_path[64];
uint8_t block_buffer[BLOCK_SIZE];

std::atomic<bool> request_stop = false;
std::atomic<bool> is_playing = false;

RingbufHandle_t ring_buffer;

void i2s_init() {
    i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
        .sample_rate = WAV_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 16,
        .dma_buf_len = BLOCK_SIZE,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_PIN_NO_CHANGE,
        .ws_io_num = I2S_PIN_NO_CHANGE,
        .data_out_num = 26,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_set_dac_mode(I2S_DAC_CHANNEL_LEFT_EN);
}

void play_wav_routine() {
    File file = SPIFFS.open(file_path);
    if (!file) {
        log_e("Failed to open file for reading");
        vTaskDelete(nullptr);
        is_playing = false;
        return;
    }

    // Skip the WAV header
    for (int i = 0; i < 44; i++) {
        file.read();
    }

    is_playing = true;

    size_t bytes_written = 0;

    // Fade in to start the sound
    uint8_t first_sample = file.read();
    for (int i = 0; i != BLOCK_SIZE; i++) {
        block_buffer[i] = first_sample * i / BLOCK_SIZE;
    }
    i2s_write_expand(I2S_NUM_0, block_buffer, BLOCK_SIZE, 8, 16, &bytes_written, 100);

    int samples = 0;
    int success = 0;
    uint8_t last_sample = 0;
    while (file.available() && !request_stop) {
        size_t bytes_read = file.read(block_buffer, BLOCK_SIZE);
        samples += bytes_read;
        last_sample = block_buffer[bytes_read - 1];
        while (true) {
            if (i2s_write_expand(I2S_NUM_0, block_buffer, bytes_read, 8, 16, &bytes_written, 100) == ESP_OK) {
                break;
            }
            vTaskDelay(1);
        }
        success++;
    }

    // Fade out to stop the sound
    for (int i = 0; i != BLOCK_SIZE; i++) {
        block_buffer[i] = last_sample * (BLOCK_SIZE - i) / BLOCK_SIZE;
    }
    i2s_write_expand(I2S_NUM_0, block_buffer, BLOCK_SIZE, 8, 16, &bytes_written, 100);

    file.close();
    is_playing = false;
}

void play_wav(const char* path) {
    request_stop = true;
    dac_output_voltage(DAC_CHANNEL, 0);
    while (is_playing)
        vTaskDelay(1);
    request_stop = false;

    strcpy(file_path, path);

    xTaskCreate([](void *) {
        play_wav_routine();
        vTaskDelete(nullptr);
    }, "ReadWAVTask", 4096, nullptr, 4, nullptr);
}

void setup_audio() {
    dac_output_enable(DAC_CHANNEL);
    i2s_init();
}
