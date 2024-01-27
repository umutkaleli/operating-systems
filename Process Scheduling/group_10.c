/*
 * Umut Kalelioglu - 20190808016
 * Arjin Kavalci   - 20190808050
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define MAX_LINE_LENGTH 256

// Process structure
typedef struct {
    char name[50];
    int ID;
    int cpuBursts[50];
    int IOBursts[50];
    int returnTime;
    int waitingTime;
    int turnaroundTime;
    int burstSum;
    int cpuBurstsSize;
    int IOBurstsSize;
    int countIdle;
} Process;

// Define the Node structure
typedef struct Node {
    Process data;
    struct Node* next;
} Node;

// Define the Queue structure
typedef struct {
    Node* front;
    Node* rear;
} Queue;

// Function prototypes
void initQueue(Queue* queue);
int isEmpty(const Queue* queue);
void enqueue(Queue* queue, Process process);
Process dequeue(Queue* queue);
Process front(Queue* queue);
void formatLines(FILE* file, char formattedLines[][MAX_LINE_LENGTH], int* lineCount);
Process initProcess(char name[], char formattedLines[][MAX_LINE_LENGTH], int lineIndex, Queue* waitingList);
void FCFS(Queue* waitingList, char formattedLines[][MAX_LINE_LENGTH], int lineCount, Process idle,Process* processArray);
int countWaitingTime(Process* process);
void setReturnTime(Process* process, int returnTime);
void calculateAverages(Process* processList, int size);
void setBurstSum(Process* processList,int lines);
void setTurnAroundTime(Process* processList,Process currentProcess, int lines);
void removeElement(int array[], int* size, int index);

// Function implementations
void initQueue(Queue* queue) {
    queue->front = NULL;
    queue->rear = NULL;
}

void removeElement(int array[], int* size, int index) {
    if (index < 0 || index >= *size) {
        // Index out of bounds
        return;
    }

    for (int i = index; i < *size - 1; i++) {
        array[i] = array[i + 1];
    }

    (*size)--;
}

int isEmpty(const Queue* queue) {
    return queue->front == NULL;
}

// Enqueue a process
void enqueue(Queue* queue, Process process) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        perror("Memory allocation error");
        exit(1);
    }
    newNode->data = process;
    newNode->next = NULL;

    if (isEmpty(queue)) {
        queue->front = newNode;
        queue->rear = newNode;
    } else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
}

// Dequeue a process
Process dequeue(Queue* queue) {
    assert(!isEmpty(queue));

    Node* frontNode = queue->front;
    Process data = frontNode->data;

    queue->front = frontNode->next;
    free(frontNode);

    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    return data;
}

// Get the front process without dequeuing
Process front(Queue* queue) {
    assert(!isEmpty(queue));
    return queue->front->data;
}

Process initProcess(char name[], char formattedLines[][MAX_LINE_LENGTH], int lineIndex, Queue* waitingList) {
    Process process;
    char* token;
    if(formattedLines == NULL){
        token = "0";
    }else{token = strtok(formattedLines[lineIndex], " ");
    }

    snprintf(process.name, sizeof(process.name), "%s%d", name, atoi(token));
    process.ID = atoi(token);

    // Initialize cpuBursts and IOBursts arrays
    memset(process.cpuBursts, 0, sizeof(process.cpuBursts));
    memset(process.IOBursts, 0, sizeof(process.IOBursts));

    // Extract CPU burst and I/O burst values from formattedLines
    token = strtok(NULL, " ");
    int index =0;

    while (token != NULL) {
        if (index % 2 == 0) {
            process.cpuBursts[index / 2] = atoi(token);
        } else {
            process.IOBursts[index / 2] = atoi(token);
        }
        index++;
        token = strtok(NULL, " ");
    }
    process.cpuBurstsSize = index / 2;
    process.IOBurstsSize = index / 2;
    process.burstSum = 0;
    process.countIdle = 0;

    if(waitingList == NULL){

    }else {
    enqueue(waitingList,process);
    }
    return process;
}


int countWaitingTime(Process* process) {
    process->waitingTime = process->turnaroundTime - process->burstSum;
    return process->waitingTime;
}

void setReturnTime(Process* process, int returnTime) {
    process->returnTime = returnTime;
}

void FCFS(Queue* waitingList, char formattedLines[][MAX_LINE_LENGTH], int lineCount, Process idle,Process* processArray) {
    int currentTime = 0;
    int round = 0;
    int control = 1; // for using in the while loop

    while (control > 0) {
        for (int i = 0; i < lineCount; i++) {
            if (isEmpty(waitingList)) // if all processes terminated
                control--;

            if (!isEmpty(waitingList)) {
                Process currentProcess = front(waitingList);
                if (currentProcess.ID == -1) { // avoid exception
                    break;
                }

                if (round == 0) { // first round, every process runs simply, no need to control
                    currentTime += currentProcess.cpuBursts[0];
                    setReturnTime(&currentProcess, currentTime + currentProcess.IOBursts[0]);
                    removeElement(currentProcess.cpuBursts,&currentProcess.cpuBurstsSize, 0);
                    removeElement(currentProcess.IOBursts, &currentProcess.IOBurstsSize, 0);
                    dequeue(waitingList); // remove the process from the queue
                    enqueue(waitingList, currentProcess); // now, the process is waiting for the next round
                } else {
                    if (currentProcess.returnTime > currentTime) { // if the process is not ready (waiting for I/O)
                        if (i == lineCount - 1) {
                            assert(waitingList->front != NULL); // avoid exception
                            // calculate early return time of all processes
                            int returnMin = front(waitingList).returnTime;
                            for (Node* temp = waitingList->front; temp != NULL; temp = temp->next) {
                                if (returnMin > temp->data.returnTime) {
                                    returnMin = temp->data.returnTime;
                                }
                            }
                            if (returnMin > currentTime) { // if the current time is less than returnMin
                                idle.countIdle++; // idle is executing until the current time equals returnMin
                                currentTime = returnMin;
                            }
                            dequeue(waitingList); // remove the process from the queue now.
                            enqueue(waitingList, currentProcess);
                            round--; // round is not increasing
                        }
                    } else if (currentProcess.IOBursts[0] == -1) { // if the process is about to terminate
                        currentTime += currentProcess.cpuBursts[0]; // it is running for the last time
                        currentProcess.turnaroundTime = currentTime;// turnaroundTime is equal to finish running
                        setTurnAroundTime(processArray,currentProcess,lineCount);
                        setReturnTime(&currentProcess, 0); // it will not return
                        dequeue(waitingList); // remove the process from the queue
                    } else { // execute normally
                        currentTime += currentProcess.cpuBursts[0]; // it is running
                        setReturnTime(&currentProcess, currentTime + currentProcess.IOBursts[0]);
                        removeElement(currentProcess.cpuBursts, &currentProcess.cpuBurstsSize, 0);
                        removeElement(currentProcess.IOBursts,  &currentProcess.IOBurstsSize, 0);
                        dequeue(waitingList); // remove the process from the queue
                        enqueue(waitingList, currentProcess); // now, the process is waiting for the next round
                    }
                }
            }
        }
        round++;
    }
}

void formatLines(FILE* file, char formattedLines[][MAX_LINE_LENGTH], int* lineCount) {
    char line[MAX_LINE_LENGTH];
    // Read lines from the file
    while (fgets(line, sizeof(line), file) != NULL) {
        // Replace non-digit and non-hyphen characters with a space
        for (int i = 0; i < strlen(line); i++) {
            if (!(line[i] >= '0' && line[i] <= '9') && line[i] != '-') {
                line[i] = ' ';
            }
        }
        // Replace multiple spaces with a single space
        char* input = line;
        char* output = formattedLines[*lineCount];
        for (; *input != '\0'; ++input) {
            if (*input != ' ' || (input > line && *(input - 1) != ' ')) {
                *output++ = *input;
            }
        }
        *output = '\0';
        (*lineCount)++;
    }
}

void calculateAverages(Process* processList, int size) {
    double waitingSum = 0;   // sum of waiting times of all processes,  first initilaize it 0.
    double turnAroundSum = 0; // sum of turnaround times of all processes, first initilaize it 0.

    for (int i = 0; i < size; i++) {
        waitingSum += countWaitingTime(&processList[i]);
        turnAroundSum += processList[i].turnaroundTime;
    }
    // averages of waiting times and turnaround times
    double avgWaiting = waitingSum / size;
    double avgTurnaround = turnAroundSum / size;
    printf("Average turnaround time: %.2f\n", avgTurnaround);
    printf("Average waiting time: %.2f\n", avgWaiting);
}

void setBurstSum(Process* processList,int lines){
    for(int i = 0; i < lines; i++){
        for (int j = 0; j < sizeof(processList[i].cpuBursts) / sizeof(processList[i].cpuBursts[0]); j++) {
            processList[i].burstSum += processList[i].cpuBursts[j];
        }
    }
}
void setTurnAroundTime(Process* processList,Process currentProcess, int lines){
    for(int i = 0; i < lines; i++){
        if(processList[i].ID == currentProcess.ID)
            processList[i].turnaroundTime = currentProcess.turnaroundTime;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    Queue waitingList;
    initQueue(&waitingList);
    char formattedLines[MAX_LINE_LENGTH][MAX_LINE_LENGTH];
    int lineCount = 0;

    FILE* file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error opening the file");
        exit(1);
    }
    formatLines(file, formattedLines, &lineCount);
    fclose(file);

    Process* processArray = (Process*)malloc(lineCount * sizeof(Process));
    if (processArray == NULL) {
        perror("Memory allocation error");
        exit(1);
    }

    // Populate processArray with processes
    for (int i = 0; i < lineCount; i++) {
        processArray[i] = initProcess("Process",  formattedLines, i, &waitingList);
    }

    Process idle = initProcess("IDLE", NULL, 0, NULL);
    FCFS(&waitingList, formattedLines, lineCount, idle,processArray);
    setBurstSum(processArray,lineCount);
    calculateAverages(processArray, lineCount);

    // Free allocated memory
    while (!isEmpty(&waitingList)) {
         dequeue(&waitingList);
    }
    printf("HALT!");
    return 0;
}
