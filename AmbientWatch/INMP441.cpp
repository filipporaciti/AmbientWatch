#include "INMP441.h"
#include <driver/i2s.h>
#include <Arduino.h>

INMP441::INMP441(){
  setupI2SMic();
}

float INMP441::readAvgValue(){
  float tot_valori = 0;
  int num_valori = 300;

  for (int i=0; i<num_valori; i++) {

    int32_t samples[SAMPLE_BUFFER_SIZE];
    size_t bytesRead;

    i2s_read(I2S_NUM_0, (char*)samples, sizeof(samples), &bytesRead, portMAX_DELAY);
    int sampleCount = bytesRead / sizeof(int32_t);

    // Calcola valore RMS come misura dell'ampiezza
    uint64_t sum = 0;
    for (int i = 0; i < sampleCount; i++) {
      int32_t sample = samples[i] >> 14; // riduzione di bit utile
      sum += sample * sample;
    }

    float rms = sqrt((float)sum / sampleCount);
    float db = 20.0 * log10(rms);
    db += DECIBEL_CALIB_OFFSET;

    tot_valori += db;
    delay(100);
  }

  float media = tot_valori/num_valori;
  return media;
}


void INMP441::setupI2SMic() {
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),  // esp32 genera i segnali di clock e setta esp32 in modalità ricezione
    .sample_rate = 16000,                               // frequenza di campionamento
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,       // numero di bit per campione
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,        // INMP441 invia dati solo sul canale sinistro, quindi leggo solo da quello
    .communication_format = I2S_COMM_FORMAT_I2S,        // formato di protocollo
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,           // livello di priorità dell'interrupt
    .dma_buf_count = 4,                                 // numero di buffer circolari. quando uno è pieno si passa al prossimo
    .dma_buf_len = 256,                                 // lunghezza di ogni buffer
    .use_apll = false,                                  // abilita o meno Audio Phase-Locked Loop. false è disabilitato e va bene così perchè consuma più energia. serve per migliorare la stabilità della frequenza
    .tx_desc_auto_clear = false,                        // in modalità ricezione viene messo a false
    .fixed_mclk = 0                                     // clock preso dal clock interno di esp32 e non dall'esterno
  };

  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);  // attiva e configura I2S con i parametri dati
  i2s_set_pin(I2S_NUM_0, &pin_config);                  // configura i pin fisici
  i2s_zero_dma_buffer(I2S_NUM_0);                       // azzera il buffer DMA associato a quel canale I2S
}