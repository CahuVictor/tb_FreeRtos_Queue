#include <Arduino.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "appl_MyLogger.h"
#include "sample_tasks.h"

#define QUEUEELEMENTSIZE  128
#define BUFFER_SIZE (QUEUEELEMENTSIZE * 2)  // Tamanho total do buffer para armazenar as mensagens em sequência

// Buffers globais para armazenar os dados escritos e lidos
int bufferSize = BUFFER_SIZE;
char bufferEscrita[BUFFER_SIZE] = "";
char bufferLeitura[BUFFER_SIZE] = "";
int indexEscrita = 0;
int indexLeitura = 0;

char bufferEscritaCircular[BUFFER_SIZE] = "";
char bufferLeituraCircular[BUFFER_SIZE] = "";
int indexEscritaCricular = 0;
int indexLeituraCircular = 0;


// Instâncias globais das classes
SampleTasks sampletask(&Serial);  // Passa a interface Serial como parâmetro

QueueHandle_t __queue__;          // Fila compartilhada entre as tasks
                                  // Vários escrevem, porém só 1 ler, pois quando ler apaga a fila
const int QueueElementSize = QUEUEELEMENTSIZE;

void updateBuffer(char* buffer, int& index, const char* mensagem) {
    int mensagemTamanho = strlen(mensagem) + 1;  // Inclui o caractere de nova linha
    LOG_INFO(&Serial, (String("Tamanho do Buffer: ") + String(bufferSize) +
      	              String("; Index: ") +  String(index) +
                      String("; Tamanho Mensagem: ") +  String(mensagemTamanho)).c_str());

    // Se não houver espaço suficiente, reseta o índice para sobrescrever do início
    if (index + mensagemTamanho >= BUFFER_SIZE) {
        LOG_INFO(&Serial, "Buffer cheio");
        index = 0;  // Reseta o índice para sobrescrever dados mais antigos
        //buffer[0] = '/0';
    }

    snprintf(buffer + index, BUFFER_SIZE - index, "%s\n", mensagem);
    index += mensagemTamanho;  // Atualiza o índice
}

void updateBufferCircular(char* buffer, int& index, const char* mensagem) {
    int mensagemTamanho = strlen(mensagem) + 1;  // Inclui o caractere de nova linha
    LOG_INFO(&Serial, (String("Tamanho do Buffer: ") + String(bufferSize) +
      	              String("; Index: ") +  String(index) +
                      String("; Tamanho Mensagem: ") +  String(mensagemTamanho)).c_str());

    // Se não houver espaço suficiente no buffer, ajusta para sobrescrever a partir do início
    if (index + mensagemTamanho >= BUFFER_SIZE) {
        LOG_INFO(&Serial, "Buffer cheio, sobrescrevendo dados antigos");

        // Calcula o tamanho da parte da mensagem que cabe no final do buffer
        int BufferBreak = bufferSize - index;

        // Trunca a mensagem e salva a parte que cabe no final do buffer
        strncpy(buffer + index, mensagem, BufferBreak - 1);
        buffer[index + BufferBreak - 1] = '\0';  // Adiciona o terminador de string

        // Reinicia o índice para o início do buffer
        index = 0;

        // Salva o restante da mensagem no início do buffer
        strncpy(buffer + index, mensagem + BufferBreak - 1, mensagemTamanho - BufferBreak);
        buffer[index + (mensagemTamanho - BufferBreak)] = '\0';  // Adiciona o terminador de string

        // Atualiza o índice após a parte restante da mensagem
        index += (mensagemTamanho - BufferBreak);
    }
    else
    {
        snprintf(buffer + index, BUFFER_SIZE - index, "%s\n", mensagem);
        index += mensagemTamanho;  // Atualiza o índice
    }
}

// Função para substituir '\n' por ';' no bufferLeitura e retornar a string formatada
String formatarBuffer(char* buffer) {
    String bufferFormatado = String(buffer);
    bufferFormatado.replace("\n", ";");  // Substitui todos os '\n' por ';'
    return bufferFormatado;
}

// Função da nova tarefa para imprimir o conteúdo da fila
void MainTaskSend_1(void* pvParameters) {
    int i = 0;
    char command[QUEUEELEMENTSIZE];

    for (;;) 
    {
        if ( __queue__ != nullptr )
        {
            snprintf(command, sizeof(command), "TaskSend_1=%d", i++);
            xQueueSend( __queue__ , &command, portMAX_DELAY);  // Envia o comando para WiFiManager

            // Armazena a mensagem no buffer de escrita com sobrescrita se necessário
            updateBuffer(bufferEscrita, indexEscrita, command);

            LOG_INFO(&Serial, (String("Adicionado dados à fila. Dados: ") + String(command)).c_str());
            delay(random(4000) + 1000);  // Aguarda entre 2,5 e 5 segundos
        }
    }
}

void MainTaskSend_2(void* pvParameters) {
    int i = 0;
    char command[QUEUEELEMENTSIZE];

    for (;;) 
    {
        if ( __queue__ != nullptr )
        {
            snprintf(command, sizeof(command), "TaskSend_2=%d", i++);
            xQueueSend( __queue__ , &command, portMAX_DELAY);  // Envia o comando para WiFiManager
            
            // Armazena a mensagem no buffer de escrita com sobrescrita se necessário
            updateBuffer(bufferEscrita, indexEscrita, command);

            LOG_INFO(&Serial, (String("Adicionado dados à fila. Dados: ") + String(command)).c_str());
            delay(random(4000) + 1000);  // Aguarda entre 2,5 e 5 segundos
        }
    }
}

void MainTaskReceive_1(void* pvParameters) {
    char receiveMessage[QUEUEELEMENTSIZE];  // Buffer para armazenar a mensagem da fila

    for (;;) 
    {
        if ( __queue__ != nullptr )
        {
            // Ler a fila
            receiveMessage[0] = '\0';
            if (xQueueReceive(__queue__, &receiveMessage, pdMS_TO_TICKS(100)) == pdPASS) {
                //LOG_INFO(&Serial, "Mensagem recebida da fila: %s\n", receivedMessage);
                LOG_INFO(&Serial, (String("Mensagem recebida da fila: ") + String(receiveMessage)).c_str());

                // Armazena a mensagem no buffer de leitura com sobrescrita se necessário
                updateBuffer(bufferLeitura, indexLeitura, receiveMessage);
            }
            else {
                LOG_INFO(&Serial, "Nenhuma mensagem na fila.");
            }
            delay(random(4000) + 1000);  // Aguarda entre 2,5 e 5 segundos
        }
    }
}

void MainTaskReceive_2(void* pvParameters) {
    char receiveMessage[QUEUEELEMENTSIZE];  // Buffer para armazenar a mensagem da fila

    for (;;) 
    {
        if ( __queue__ != nullptr )
        {
            // Ler a fila
            receiveMessage[0] = '\0';
            if (xQueueReceive(__queue__, &receiveMessage, pdMS_TO_TICKS(100)) == pdPASS) {
                //LOG_INFO(&Serial, "Mensagem recebida da fila: %s\n", receivedMessage);
                LOG_INFO(&Serial, (String("Mensagem recebida da fila: ") + String(receiveMessage)).c_str());
                
                // Armazena a mensagem no buffer de leitura com sobrescrita se necessário
                updateBuffer(bufferLeitura, indexLeitura, receiveMessage);
            }
            else {
                LOG_INFO(&Serial, "Nenhuma mensagem na fila.");
            }
            delay(random(4000) + 1000);  // Aguarda entre 2,5 e 5 segundos
        }
    }
}

void setup() {
    // Inicialização da comunicação serial
    Serial.begin(115200);
    while (!Serial);  // Aguarda a inicialização da porta serial

    MyLogger::initialize();  // Inicializa o logger com proteção de mutex se FreeRTOS estiver disponível
    LOG_INFO(&Serial, "Sistema inicializado com sucesso.");

    // Inicializa e configura a fila
    __queue__ = xQueueCreate(QueueElementSize, sizeof(char) * QueueElementSize);        // Create the queue which will have <QueueElementSize>
                                                                                            // number of elements, each of size `message_t` and 
                                                                                            // pass the address to <QueueHandle>.
    // Check if the queue was successfully created
    if (__queue__ == NULL)
    {
        LOG_INFO(&Serial, "Queue could not be created. Halt.");
        while (1)
        {
            delay(1000);  // Halt at this point as is not possible to continue
        }
    }

    // Passa a fila para os módulos libs
    //sampletask.setQueue(__queue__);

    // Inicialização dos módulos
    //sampletask.initialize();

    // Configurações de teste
    //serialComm.sendMessage("Hello from SerialCommunication!");

    // Inicia as tarefas
    // Criar task para gerenciar a fila pois sempre que é lido é apagada a fila
    xTaskCreate(MainTaskSend_1, "Main Task 1", 2048, NULL, 1, NULL);
    xTaskCreate(MainTaskSend_2, "Main Task 2", 2048, NULL, 1, NULL);
    xTaskCreate(MainTaskReceive_1, "Main Task 3", 2048, NULL, 1, NULL);
    xTaskCreate(MainTaskReceive_2, "Main Task 4", 2048, NULL, 1, NULL);
    //sampletask.startTask();
}

void loop() 
{
    // O loop principal não será usado, pois as tarefas são gerenciadas pelo FreeRTOS
    // Exemplo de log periódico
    LOG_INFO(&Serial, "Loop principal em execução.");

    // Imprime os dados do buffer de escrita
    //LOG_INFO(&Serial, String("Buffer de Escrita:") + String(bufferEscrita));
    // Imprime os dados do buffer de escrita
    LOG_INFO(&Serial, (String("Buffer de Escrita:") + formatarBuffer(bufferEscrita)).c_str());
    LOG_INFO(&Serial, (String("Buffer de Escrita Circular:") + formatarBuffer(bufferEscritaCircular)).c_str());

    // Imprime os dados do buffer de leitura
    //LOG_INFO(&Serial, String()"Buffer de Leitura:") + String(bufferLeitura));
    // Imprime os dados do buffer de leitura
    LOG_INFO(&Serial, (String("Buffer de Leitura:") + formatarBuffer(bufferLeitura)).c_str());
    LOG_INFO(&Serial, (String("Buffer de Leitura Circular:") + formatarBuffer(bufferLeituraCircular)).c_str());

    delay(5000);
}
