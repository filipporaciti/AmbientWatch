
// Configurazione pin I2S
#define I2S_WS     27  // L/R word selector
#define I2S_SD     32  // SD data
#define I2S_SCK    26  // Bit clock

#define SAMPLE_BUFFER_SIZE 1024

#define DECIBEL_CALIB_OFFSET 0 // calibrazione decibel microfono

/* Classe per la gestione e lettura del microfono. Il microfono utilizza il protocollo I2S. ATTENZIONE!!! verificare se si deve tarare il microfono */
class INMP441 {
  private:
    /*
    Configurazione e inizializzazione I2S.
    */
    void setupI2SMic();

  public:
    INMP441();

    /*
    Prende un campione ogni 100 millisecondi per 300 volte (~30 secondi) e ritorna la media di questi valori.

    @return media dei valori presi
    */
    float readAvgValue();
  
};
