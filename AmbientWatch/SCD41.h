#include <SensirionI2cScd4x.h>

/* Classe per la gestione e la lettura del sensore SCD41 per la co2, temperatura e umidità */
class SCD41: public SensirionI2cScd4x {
  public:
    SCD41();

    /*
    Inizializzazione sensore.

    @return stato inizializzazione
    */
    bool begin();

    /*
    Effettua tre tentativi di lettura del sensore e in caso di successo scrive nei riferimenti dati dai parametri della funzione i risultati.

    @param co2 puntatore alla variabile che in caso di successo conterrà il risultato
    @param temperature puntatore alla variabile che in caso di successo conterrà il risultato
    @param humidity puntatore alla variabile che in caso di successo conterrà il risultato
    @return bool sarà false in caso di errore, true altrimenti 
    */
    bool getValues(uint16_t& co2, float& temperature, float& humidity);
};