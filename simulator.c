#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* constante com o tamanho da tabela de processos
    (o maximo que ele suporta em numero de processos)
    e o espaço de frames de memória destinado aos processos
    gerados */
#define PROCESS_TABLE_SIZE 200
#define SYSTEM_MEMORY 64

/* constantes para definir o range maximo e minimo
    da geração de pid, o máximo de frames que cada
    processo pode ter e o máximo de frames que cada
    processo pode ter na memória */
#define MAX_PID 9999
#define MIN_PID 1000
#define MAX_FRAMES 20
#define WORKING_SET_LIMIT 4

/* struct com as informações do processo e seus frames */
typedef struct {
    int pid;                            // pid do processo
    int *frames;                        // frames do processo
    int frames_in_memory;               // quantidade de frames que estão alocadas na memória
    int frames_length;                  // quantidade de frames que o processo possui
} Process;

/* struct para montar a lista dos processos que estão ativos no "sistema".
    é necessário para não ser possível criar processos com pid's repetidos. */
typedef struct {
    Process running_processes[PROCESS_TABLE_SIZE];  // tabela dos processos que estão rodando no sistema
    int length;                                     // quantidade desses processos
} ProcessTable;

/* struct para servir como as posição do vetor LRU */
typedef struct {
    int pid;
    int frame_index;
} ProcessFrame;

/* struct da fila LRU */
typedef struct {
    ProcessFrame frames[SYSTEM_MEMORY];
    int rear;
} LRUQueue;

/* struct da memória do sistema */
typedef struct {
    int frames[SYSTEM_MEMORY];
} Memory;


void initProcessTable();

void initMemory();
void allocateMemory(Process *process, int frame);
void deallocateMemory(Process *process, int frame);

void initQueue(LRUQueue *queue);
void enqueue(LRUQueue *queue, Process process, int frame);
void dequeueFrame(LRUQueue *queue, Process process, int frame);
ProcessFrame dequeueOldest(LRUQueue *queue);

Process createProcess();
int indexProcessTable(int pid);
void destroyProcess(Process process);

int generateRandomNumber(int lower_limit, int upper_limit);

ProcessTable table;
Memory memory;

int main() {
    LRUQueue queue;

    srand(time(NULL));      // inicialização para randomizar os números

    initProcessTable();
    initMemory();
    initQueue(&queue);
    
    Process processo1 = createProcess();
    Process processo2 = createProcess();
    Process processo3 = createProcess();
    Process processo4 = createProcess();

    for(int i = 0; i < 4; i++) {
        printf("Processo %d:\n pid - %d\n frames na memoria: %d\n numero de frames: %d\n", i, 
            table.running_processes[i].pid, table.running_processes[i].frames_in_memory, table.running_processes[i].frames_length);
    }

    printf("\n");

    enqueue(&queue, processo2, 0);
    enqueue(&queue, processo1, 0);
    enqueue(&queue, processo4, 0);
    enqueue(&queue, processo3, 0);

    for(int i = 0; i <= queue.rear; i++) {
        printf("%d. pid %d frame %d, ", i, queue.frames[i].pid, queue.frames[i].frame_index);
    }
    printf("\n");

    dequeueOldest(&queue);

    for(int i = 0; i <= queue.rear; i++) {
        printf("%d. pid %d frame %d, ", i, queue.frames[i].pid, queue.frames[i].frame_index);
    }
    printf("\n");

    dequeueFrame(&queue, processo3, 0);

    for(int i = 0; i <= queue.rear; i++) {
        printf("%d. pid %d frame %d, ", i, queue.frames[i].pid, queue.frames[i].frame_index);
    }
    printf("\n");

    allocateMemory(&processo1, 0);
    allocateMemory(&processo4, 0);

    printf("pid %d, frames in memory %d: ", processo1.pid, processo1.frames_in_memory);
    for(int i = 0; i < processo1.frames_length; i++) {
        printf("%d ", processo1.frames[i]);
    }
    printf("\n");

    printf("pid %d, frames in memory %d: ", processo4.pid, processo4.frames_in_memory);
    for(int i = 0; i < processo4.frames_length; i++) {
        printf("%d ", processo4.frames[i]);
    }
    printf("\n");

    for(int i = 0; i < SYSTEM_MEMORY; i++) {
        printf("%d: %d\n", i, memory.frames[i]);
    }
    printf("\n");

    deallocateMemory(&processo1, 0);

    printf("pid %d, frames in memory %d: ", processo1.pid, processo1.frames_in_memory);
    for(int i = 0; i < processo1.frames_length; i++) {
        printf("%d ", processo1.frames[i]);
    }
    printf("\n");

    printf("pid %d, frames in memory %d: ", processo4.pid, processo4.frames_in_memory);
    for(int i = 0; i < processo4.frames_length; i++) {
        printf("%d ", processo4.frames[i]);
    }
    printf("\n");

    for(int i = 0; i < SYSTEM_MEMORY; i++) {
        printf("%d: %d\n", i, memory.frames[i]);
    }

}

/* Inicializa a tabela de processos */
void initProcessTable() {
    table.length = 0;
}

/* Inicializa a memória. 0 se ela estiver vazia e 1 se ela estiver ocupada.
    Num contexto real, ela seria preenchida com instruções e endereços físicos
    ao invés desses valores */
void initMemory() {
    for(int i = 0; i < SYSTEM_MEMORY; i++) {
        memory.frames[i] = 0;
    }
}

/* Aloca um frame de um processo na memória. Ele preenche no frame do processo
    a posição da memória que ele está ocupando, atualiza o frame da memória que 
    está ocupado e atualiza a variável do processo que indica quantos frames ele 
    possui na memória */
void allocateMemory(Process *process, int frame) {
    int memory_pos = generateRandomNumber(0, SYSTEM_MEMORY - 1);
    while (memory.frames[memory_pos]) {
        memory_pos = generateRandomNumber(0, SYSTEM_MEMORY - 1);

    }

    process->frames[frame] = memory_pos;
    memory.frames[memory_pos] = 1;
    process->frames_in_memory++;
}

/* Desaloca um frame de um processo da memória. Ele apaga no frame do processo
    a posição da memória que ele estava ocupando, esvazia o frame da memória e 
    atualiza a variável do processo que indica quantos frames ele 
    possui na memória */
void deallocateMemory(Process *process, int frame) {
    int memory_pos = process->frames[frame];
    memory.frames[memory_pos] = 0;
    process->frames[frame] = -1;
    process->frames_in_memory--;
}

/* Inicializa a estrutura de dados de fila */
void initQueue(LRUQueue *queue) {
    queue->rear = -1;
}

/* Adiciona um frame de um processo numa fila LRU. Quando está cheia,
    printa na tela uma mensagem e não adiciona. */
void enqueue(LRUQueue *queue, Process process, int frame) {
    if (queue->rear == SYSTEM_MEMORY - 1) {
        printf("Fila cheia! Nao e possivel inserir.\n");
    } else {
        queue->rear++;
        queue->frames[queue->rear].pid = process.pid;
        queue->frames[queue->rear].frame_index = frame;
    }
}

/* Remove um frame de um processo da fila LRU e anda com todos os frames pra 
    frente. Essa função é para situações onde deseja-se tirar um frame que 
    está no meio da fila para colocá-lo no final. A função executa somente 
    quando possui frames na fila. */
void dequeueFrame(LRUQueue *queue, Process process, int frame) {
    if (queue->rear >= 0) {
        for(int i = 0; i <= queue->rear; i++) {
            if (queue->frames[i].pid == process.pid && queue->frames[i].frame_index == frame) {
                for(int j = i; j < queue->rear; j++) {
                    queue->frames[j] = queue->frames[j + 1];
                }
            }
        }

        queue->rear--;
    }
}

/* Remove um frame de um processo da primeira posição de uma fila e
    retorna ele. Na remoção, ele anda com todos os processos uma posição 
    para frente. É para situações onde deseja-se tirar o frame mais antigo
    da memória

    Quando a fila está vazia, retorna um processframe com pid inválido,
    indicando que não foi possível fazer a operação */
ProcessFrame dequeueOldest(LRUQueue *queue) {
    if (queue->rear == -1) {
        ProcessFrame empty;
        empty.pid = -1;
        return empty;

    } else {
        ProcessFrame oldest = queue->frames[0];

        for (int i = 0; i < queue->rear; i++) {
            queue->frames[i] = queue->frames[i + 1];
        }

        queue->rear--;
        return oldest;
    }
}


/* Cria um processo, inicializando todos os dados necessários e
    adicionando-o na tabela de processos.

    Se a tabela estiver cheia, printa na tela uma mensagem
    e retorna um processo com pid inválido, indicando que
    não foi possível fazer a operação.

    Dados inicializados:
        pid: um número aleatório de 4 digitos, único na tabela de processos
        frames_in_memory: 0
        frames_length: um número aleatório entre 1 e MAX_FRAMES, definindo
            a quantidade de frames que o processo possui
        frames: array de int para indicar a posição da memória que o frame
            está ocupando. se ele não estiver ocupando, ele é indicado com 
            o valor -1
    */
Process createProcess() {
    if (table.length == PROCESS_TABLE_SIZE) {
        printf("Tabela cheia! Não é possível criar mais processo.\n");

        Process empty;
        empty.pid = -1;
        return empty;
    }

    Process new_process;
    new_process.pid = generateRandomNumber(MIN_PID, MAX_PID);
    while (indexProcessTable(new_process.pid) != -1) {
        new_process.pid = generateRandomNumber(MIN_PID, MAX_PID);

    }
    new_process.frames_in_memory = 0;
    new_process.frames_length = generateRandomNumber(1, MAX_FRAMES);
    new_process.frames = (int *)malloc(new_process.frames_length * sizeof(int));
    if(new_process.frames == NULL) {
        printf("Erro de malloc na criação dos frames para o processo\n");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < new_process.frames_length; i++) {
        new_process.frames[i] = -1;
    }

    table.running_processes[table.length] = new_process;
    table.length++;

    return new_process;
}

/* Retorna a posição do processo na tabela de processos. 
    Caso não encontre retorna -1 */
int indexProcessTable(int pid) {
    for (int i = 0; i < table.length; i++) {
        if (table.running_processes[i].pid == pid) {
            return i;
        }
    }

    return -1;
}

/* Remove o processo da tabela de processos. Na remoção,
    ele anda com todos os processos que estavam atrás da posição
    dele na tabela uma posição para frente.

    Caso não encontre o processo, exibe uma mensagem de erro. */
void destroyProcess(Process process) {
    int index = -1;

    for (int i = 0; i < table.length; i++) {
        if (table.running_processes[i].pid == process.pid) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        printf("Processo não encontrado!");
        return;
    }

    for (int i = index; i < table.length - 1; i++) {
        table.running_processes[i] = table.running_processes[i+1];
    }
    table.length--;
}


/* Função auxiliar para gerar número aleatórios entre um
    limite mínimo e um limite máximo */
int generateRandomNumber(int lower_limit, int upper_limit) {
    return (rand() % (upper_limit - lower_limit + 1)) + lower_limit;
}