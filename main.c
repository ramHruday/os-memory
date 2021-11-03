#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "process.h";
#include "queue.h";
#include "memory.h";

PROCESS *pushToList(const char *path);

int prsNum = 0, last_announcement = -1;
PROCESS *prsList;
proc_queue *prsQueue;
event_list *eventList;

void startTimeCounter()
{
    long time = 0;

    while (1)
    {
        pushNewQueue(time);
        terminateFinished(time);

        allocateMem(time);

        time++;

        if (time > 10000)
        {
            printf("Run time exceeded\n");
            break;
        }

        if (prsQueue->size == 0 && isInEventList(eventList))
        {
            break;
        }
    }

    printTime();
}

int main()
{
    int size = 0;
    int mem = 0;

    char *path = malloc(100 * sizeof(char));

    getInput(&mem, &size, path);
    prsList = pushToList(path);
    prsQueue = create_proc_queue(prsNum);
    eventList = createListOfEvents(mem / size, size);

    startTimeCounter();

    return 0;
}

void pushNewQueue(int time)
{
    int i;
    PROCESS *prs;

    for (i = 0; i < prsNum; i += 1)
    {
        prs = &prsList[i];

        if (prs->arrival_time == time)
        {
            printf("%sProcess %d arrives\n",
                   getPrefix(time),
                   prs->pid);

            enqueue_proc(prsQueue, prs);

            print_proc_queue(prsQueue);
            printEventList(eventList);
        }
    }
}

void terminateFinished(int time)
{
    int i, time_spent_in_memory;
    PROCESS *prs;

    // dequeue any procs that need it
    for (i = 0; i < prsNum; i += 1)
    {
        prs = &prsList[i];
        time_spent_in_memory = time - prs->time_added_to_memory;

        if (prs->is_active && (time_spent_in_memory >= prs->life_time))
        {
            printf("%sProcess %d completes\n",
                   getPrefix(time),
                   prs->pid);

            prs->is_active = 0;
            prs->time_finished = time;

            freePid(eventList, prs->pid);

            printEventList(eventList);
        }
    }
}

void allocateMem(int time)
{
    int i, index, limit;
    PROCESS *proc;

    limit = prsQueue->size;

    // enqueue any procs that can be put into mem
    for (i = 0; i < limit; i += 1)
    {
        index = iterate_queue_index(prsQueue, i);
        proc = prsQueue->elements[index];

        if (isMemoryFit(eventList, proc))
        {
            printf("%sMM moves Process %d to memory\n",
                   getPrefix(time),
                   proc->pid);

            fitPrsToMemory(eventList, proc);

            proc->is_active = 1;
            proc->time_added_to_memory = time;

            dequeue_proc_at_index(prsQueue, i);
            print_proc_queue(prsQueue);
            printEventList(eventList);
        }
    }
}

char *getPrefix(int time)
{
    char *result;

    result = malloc(20 * sizeof(char));

    if (last_announcement == time)
    {
        sprintf(result, "\t");
    }
    else
    {
        sprintf(result, "t = %d: ", time);
    }

    last_announcement = time;

    return result;
}

void printTime()
{
    int i;
    float total = 0;

    for (i = 0; i < prsNum; i += 1)
    {
        total += prsList[i].time_finished - prsList[i].arrival_time;
    }

    printf("Average Turnaround Time: %2.2f\n", total / prsNum);
}

int multiple_of_one_hundred(int t)
{
    return (t % 100) == 0 ? 1 : 0;
}

int is_one_two_or_three(int t)
{
    return (t >= 1 && t <= 3) ? 1 : 0;
}

void clear_stdin(char *buf)
{
    if (buf[strlen(buf) - 1] != '\n')
    {
        int ch;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            ;
    }
}

int process_numeric_input_from_user(const char *output, int (*func)(int))
{
    char buf[10];
    int success = 0;
    int res = 0;

    while (!success)
    {
        printf("%s: ", output);

        if (fgets(buf, 10, stdin) == NULL)
        {
            clear_stdin(buf);
            printf("ERROR: You didn't enter any data!\n");

            continue;
        }

        if (sscanf(buf, "%d", &res) <= 0)
        {
            clear_stdin(buf);
            printf("ERROR: You didn't enter a number!\n");

            continue;
        }

        if (!(success = (*func)(res)))
        {
            clear_stdin(buf);
            printf("ERROR: That number is not a valid choice\n");
        }
    }

    return res;
}

void fileInput(char *res)
{
    char buf[100];
    FILE *fp;

    while (1)
    {
        printf("Input file: ");

        if (sscanf(buf, "%s", res) <= 0)
        {
            clear_stdin(buf);
            printf("ERROR: You didn't enter a string!\n");

            continue;
        }

        if (!(fp = fopen(res, "r")))
        {
            perror("ERROR: Could not open file!\n");
        }
        else
        {
            break;
        }
    }
}

void getInput(int *mem, int *page, char *path)
{
    while (1)
    {
        *mem = process_numeric_input_from_user(
            "Memory size", multiple_of_one_hundred);

        *page = process_numeric_input_from_user(
            "Page size (1: 100, 2: 200, 3: 400)", is_one_two_or_three);

        switch (*page)
        {
        case 1:
            *page = 100;
            break;
        case 2:
            *page = 200;
            break;
        case 3:
            *page = 400;
            break;
        }

        if ((*mem) % (*page) == 0)
        {
            break;
        }

        printf("ERROR: Memory size must be a multiple of the page!");
        printf(" %d is not a multiple of %d, please retry.\n", *mem, *page);
    }

    fileInput(path);
}

// get number of processes from file
int get_number_of_processes_from_file(FILE *filePtr)
{
    int num = 0;

    fscanf(filePtr, "%d", &num);

    return num;
}

PROCESS *pushToList(const char *path)
{
    int numSpace;
    int tmp;
    int counter = 0;
    int totalSpace = 0;
    FILE *filePtr = fopen(path, "r");

    prsNum = get_number_of_processes_from_file(filePtr);

    PROCESS *procList = malloc(prsNum * sizeof(PROCESS));

    if (!filePtr)
    {
        printf("ERROR: Failed to open file %s", path);
        exit(1);
    }

    while (!feof(filePtr) && counter < prsNum)
    {
        // store values for processes
        fscanf(filePtr, "%d %d %d %d",
               &(procList[counter].pid),
               &(procList[counter].arrival_time),
               &(procList[counter].life_time),
               &numSpace);

        // get total memory requirements for process
        totalSpace = 0;
        for (int i = 0; i < numSpace; i++)
        {
            fscanf(filePtr, "%d", &tmp);
            totalSpace += tmp;
        }
        procList[counter].mem_reqs = totalSpace;

        procList[counter].is_active = 0;
        procList[counter].time_added_to_memory = -1;
        procList[counter].time_finished = -1;

        counter++;
    }

    fclose(filePtr);

    return procList;
}
