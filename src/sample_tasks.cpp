#include "sample_tasks.h"

SampleTasks::SampleTasks(Stream* serialInterface) 
    : serial(serialInterface), __queue__(nullptr)/*, bufferMutex(nullptr)*/ {}

void SampleTasks::initialize() {
    LOG_INFO(serial, "SampleTasks: Inicializado.");

    // Inicializa o mutex para o buffer
    /*bufferMutex = xSemaphoreCreateMutex();
    if (bufferMutex == nullptr) {
        LOG_ERROR(serial, "SerialCommunication: Falha ao criar mutex para buffer.");
    }*/
}

void SampleTasks::setQueue(QueueHandle_t queue) {
    LOG_INFO(serial, "SampleTasks: Set queue.");
    //__queue__ = queue;
}

void SampleTasks::startTask() {
   LOG_INFO(serial, "SampleTasks: Task iniciado.");
   xTaskCreate(taskWrapper, "Lib Task 3",                               // A name just for humans
                2048,                                                   // The stack size can be checked by calling `uxHighWaterMark = 
                                                                        // uxTaskGetStackHighWaterMark(NULL);`
                this,                                                   // Task parameter which can modify the task behavior. This must 
                                                                        // be passed as pointer to void.
                1,                                                      // Priority
                NULL);                                                  // Task handle is not used here - simply pass NULL
}

void SampleTasks::taskWrapper(void* pvParameters) {
    // Converte o parâmetro de volta para o tipo SampleTasks
    SampleTasks* instance = static_cast<SampleTasks*>(pvParameters);
    // Chama o método membro sample_task_lib
    instance->sample_task_lib();
}

#define _QUEUEELEMENTSIZE_ 128

void SampleTasks::sample_task_lib() {
    LOG_INFO(serial, "SampleTasks: Iniciando loop.");
    char receiveMessage[_QUEUEELEMENTSIZE_];  // Buffer para armazenar a mensagem da fila
    int i = 0;
    char command[_QUEUEELEMENTSIZE_];

    for (;;) 
    {
        delay(20000);
        //if (__queue__ != nullptr && *__queue__ != nullptr)
        if ( __queue__ != nullptr )
        {
            // Ler a fila
            if (xQueueReceive(__queue__, &receiveMessage, pdMS_TO_TICKS(100)) == pdPASS) {
                //LOG_INFO(&Serial, "Mensagem recebida da fila: %s\n", receivedMessage);
                LOG_INFO(&Serial, (String("Mensagem recebida da fila: ") + String(receiveMessage)).c_str());
            } else {
                LOG_INFO(&Serial, "Nenhuma mensagem na fila.");
            }

            delay(random(500));
            delay(10000);

            // adiciona dados a fila
            snprintf(command, sizeof(command), "Task_3=%d", i++);
            xQueueSend( __queue__ , &command, portMAX_DELAY);  // Envia o comando para WiFiManager
            LOG_INFO(&Serial, (String("Adicionado dados à fila. Dados: ") + String(command)).c_str());

            delay(random(500));
            delay(10000);
            // Aguarda 10 segundos antes de verificar novamente
            //vTaskDelay(pdMS_TO_TICKS(10000));
        }
    }
}