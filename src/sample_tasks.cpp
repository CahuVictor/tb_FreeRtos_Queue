#include "sample_tasks.h"

SampleTasks::SampleTasks(Stream* serialInterface) 
    : serial(serialInterface), queue(nullptr)/*, bufferMutex(nullptr)*/ {}

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
    this->queue = queue;
}

void SampleTasks::setBuffer(char* buffer) {
    LOG_INFO(serial, "SampleTasks: Set buffer.");
    this->buffer = buffer;
}

void SampleTasks::startTask(int taskType) {
    if (taskType == 0)
    {
        LOG_INFO(serial, "SampleTasks: Task Send iniciado.");
        xTaskCreate(taskWrapperSend, "Lib Task 5",                               // A name just for humans
                        2048,                                                   // The stack size can be checked by calling `uxHighWaterMark = 
                                                                                // uxTaskGetStackHighWaterMark(NULL);`
                        this,                                                   // Task parameter which can modify the task behavior. This must 
                                                                                // be passed as pointer to void.
                        1,                                                      // Priority
                        NULL);                                                  // Task handle is not used here - simply pass NULL
    }
    if (taskType == 1)
    {
        LOG_INFO(serial, "SampleTasks: Task Receive iniciado.");
        xTaskCreate(taskWrapperReceive, "Lib Task 6",                               // A name just for humans
                        2048,                                                   // The stack size can be checked by calling `uxHighWaterMark = 
                                                                                // uxTaskGetStackHighWaterMark(NULL);`
                        this,                                                   // Task parameter which can modify the task behavior. This must 
                                                                                // be passed as pointer to void.
                        1,                                                      // Priority
                        NULL);                                                  // Task handle is not used here - simply pass NULL
    }
}


void SampleTasks::taskWrapperSend(void* pvParameters) {
    // Converte o parâmetro de volta para o tipo SampleTasks
    SampleTasks* instance = static_cast<SampleTasks*>(pvParameters);
    // Chama o método membro sample_task_lib
    instance->sample_task_send_lib();
}

void SampleTasks::taskWrapperReceive(void* pvParameters) {
    // Converte o parâmetro de volta para o tipo SampleTasks
    SampleTasks* instance = static_cast<SampleTasks*>(pvParameters);
    // Chama o método membro sample_task_lib
    instance->sample_task_receive_lib();
}

#define _QUEUEELEMENTSIZE_ 128

void SampleTasks::sample_task_send_lib() {
    LOG_INFO(serial, "SampleSendTasks: Iniciando loop.");
    char receiveMessage[_QUEUEELEMENTSIZE_];  // Buffer para armazenar a mensagem da fila
    int i = 0;
    char command[_QUEUEELEMENTSIZE_];

    for (;;) 
    {
        //if (this->queue != nullptr && *this->queue != nullptr)
        if ( this->queue != nullptr )
        {
            snprintf(command, sizeof(command), "TaskSend_3=%d", i++);
            xQueueSend( this->queue , &command, portMAX_DELAY);  // Envia o comando para WiFiManager
            
            // Armazena a mensagem no buffer de escrita com sobrescrita se necessário
            //updateBuffer(this->buffer, indexEscrita, command);

            LOG_INFO(&Serial, (String("Adicionado dados à fila. Dados: ") + String(command)).c_str());
            delay(random(4000) + 1000);  // Aguarda entre 2,5 e 5 segundos
        }
    }
}

void SampleTasks::sample_task_receive_lib() {
    LOG_INFO(serial, "SampleReceiveTasks: Iniciando loop.");
    char receiveMessage[_QUEUEELEMENTSIZE_];  // Buffer para armazenar a mensagem da fila

    for (;;) 
    {
        if ( this->queue != nullptr )
        {
            // Ler a fila
            receiveMessage[0] = '\0';
            if (xQueueReceive(this->queue, &receiveMessage, pdMS_TO_TICKS(100)) == pdPASS) {
                //LOG_INFO(&Serial, "Mensagem recebida da fila: %s\n", receivedMessage);
                LOG_INFO(&Serial, (String("SampleTasks: Mensagem recebida da fila: ") + String(receiveMessage)).c_str());

                // Armazena a mensagem no buffer de leitura com sobrescrita se necessário
                //updateBuffer(this->buffer, indexLeitura, receiveMessage);
            }
            else {
                LOG_INFO(&Serial, "Nenhuma mensagem na fila.");
            }
            delay(random(4000) + 1000);  // Aguarda entre 2,5 e 5 segundos
        }
    }
}