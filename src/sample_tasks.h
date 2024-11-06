#ifndef __appl_SerialCommunication_h__
#define __appl_SerialCommunication_h__

#include "Arduino.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "appl_MyLogger.h"

#define BUFFER_SIZE_CIRCULAR 256  // Tamanho do buffer circular

class SampleTasks {
public:
    SampleTasks(Stream* serialInterface);  // Construtor com a interface serial como parâmetro

    void initialize();                        // Inicializa a fila e o buffer circular
    void setQueue(QueueHandle_t queue);     // Define a fila da comunicação serial
    void startTask();                        // Inicia as tarefas FreeRTOS para recepção e transmissão

private:
    Stream* serial;                           // Interface serial para comunicação
    QueueHandle_t __queue__;                 // Fila para dados de saída serial
    void sample_task_lib();

    // Função estática para utilizar FreeRtos
    static void taskWrapper(void* pvParameters);
};

#endif
