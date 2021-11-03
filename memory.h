typedef struct EVENT
{
    int assigned;
    char location[40];
    int proc_assign;
    int page_num;
} EVENT;

typedef struct event_list
{
    EVENT *events;
    int count;
    int size;
} event_list;

event_list *create_event_list(int count, int size)
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

int proc_can_fit_into_memory(event_list *list, PROCESS *proc)
{
    int i, num_free_frames = 0;

    for (i = 0; i < list->count; i += 1)
    {
        if (!list->events[i].assigned)
        {
            num_free_frames += 1;
        }
    }

    // if the number of free frames * the page size is greater than the mem req
    // for the process in question we can fit it in.
    return (num_free_frames * list->size) >= proc->mem_reqs;
}

void fit_proc_into_memory(event_list *list, PROCESS *proc)
{
    // this assumes you've already checked that you *can* fit the proc into mem
    int i, remaining_mem, current_page = 1;

    remaining_mem = proc->mem_reqs;

    for (i = 0; i < list->count; i += 1)
    {
        // if this frame is not assigned
        if (!list->events[i].assigned)
        {
            // assign it
            list->events[i].assigned = 1;
            // set the page number
            list->events[i].page_num = current_page;
            // set the proc num
            list->events[i].proc_assign = proc->pid;

            current_page++;
            remaining_mem -= list->size;
        }

        if (remaining_mem <= 0)
        {
            break;
        }
    }
}

void print_frame_list(event_list *list)
{
    int i, in_free_block = 0, start;

    printf("\tMemory map:\n");

    for (i = 0; i < list->count; i += 1)
    {
        if (!in_free_block && !list->events[i].assigned)
        {
            in_free_block = 1;
            start = i;
        }
        else if (in_free_block && list->events[i].assigned)
        {
            in_free_block = 0;
            printf("\t\t%d-%d: Free frame(s)\n",
                   start * list->size,
                   (i * list->size) - 1);
        }

        if (list->events[i].assigned)
        {
            printf("\t\t%d-%d: Process %d, Page %d\n",
                   i * list->size,
                   ((i + 1) * list->size) - 1,
                   list->events[i].proc_assign,
                   list->events[i].page_num);
        }
    }

    if (in_free_block)
    {
        printf("\t\t%d-%d: Free frame(s)\n",
               start * list->size,
               ((i)*list->size) - 1);
    }
}

int frame_list_is_empty(event_list *list)
{
    int i;

    for (i = 0; i < list->count; i += 1)
    {
        if (list->events[i].assigned)
        {
            return 0;
        }
    }

    return 1;
}

void free_memory_for_pid(event_list *list, int pid)
{
    int i;
    EVENT *frame;

    for (i = 0; i < list->count; i += 1)
    {
        frame = &list->events[i];

        if (frame->proc_assign == pid)
        {
            frame->proc_assign = 0;
            frame->page_num = 0;
            frame->assigned = 0;
        }
    }
}
