typedef struct EVENT
{
    int assigned;
    char location[100];
    int assignPRS;
    int pgNum;
} EVENT;

typedef struct event_list
{
    EVENT *events;
    int count;
    int size;
} event_list;

event_list *createListOfEvents(int count, int size)
{
    int i;

    event_list *e;

    e = malloc(sizeof(event_list));

    e->events = (EVENT *)malloc(sizeof(EVENT) * count);
    e->size = size;
    e->count = count;

    for (i = 0; i < e->count; i += 1)
    {
        e->events[i].assigned = 0;
    }

    return e;
}

int isMemoryFit(event_list *ls, PROCESS *prs)
{
    int freeCount = 0;

    for (int i = 0; i < ls->count; i += 1)
    {
        if (!ls->events[i].assigned)
        {
            freeCount += 1;
        }
    }

    return (freeCount * ls->size) >= prs->mem_reqs;
}

void fitPrsToMemory(event_list *ls, PROCESS *prs)
{
    int memLeft, currPageNum = 1;

    memLeft = prs->mem_reqs;

    for (int i = 0; i < ls->count; i += 1)
    {

        if (!ls->events[i].assigned)
        {
            ls->events[i].assigned = 1;

            ls->events[i].pgNum = currPageNum;

            ls->events[i].assignPRS = prs->pid;

            currPageNum++;
            memLeft -= ls->size;
        }

        if (memLeft <= 0)
        {
            break;
        }
    }
}

void printEventList(event_list *ls)
{
    int i, freeBlock = 0, start;

    printf("\tMemory allocation for each process:\n");

    for (i = 0; i < ls->count; i += 1)
    {
        if (!freeBlock && !ls->events[i].assigned)
        {
            freeBlock = 1;
            start = i;
        }
        else if (freeBlock && ls->events[i].assigned)
        {
            freeBlock = 0;
            printf("\t\t%d-%d: Free event(s)\n",
                   start * ls->size,
                   (i * ls->size) - 1);
        }

        if (ls->events[i].assigned)
        {
            printf("\t\t%d-%d: Process %d, page no. %d\n",
                   i * ls->size,
                   ((i + 1) * ls->size) - 1,
                   ls->events[i].assignPRS,
                   ls->events[i].pgNum);
        }
    }

    if (freeBlock)
    {
        printf("\t\t%d-%d: Free events(s)\n",
               start * ls->size,
               ((i)*ls->size) - 1);
    }
}

bool isInEventList(event_list *list)
{

    bool flag = false;
    for (int i = 0; i < list->count; i += 1)
    {
        if (list->events[i].assigned)
        {
            flag = true;
            break;
        }
    }

    return flag;
}

void freePid(event_list *ls, int pid)
{

    EVENT *event;

    for (int i = 0; i < ls->count; i += 1)
    {
        event = &ls->events[i];

        if (event->assignPRS == pid)
        {
            event->assignPRS = 0;
            event->pgNum = 0;
            event->assigned = 0;
        }
    }
}
