#define MMM_SIZE	(1024*4)

static int	hunsoo_malloc_limit;

struct MMM
{
    void	*pointer;
    char	where[64];
    int	line;
};

static struct MMM	MMMS[MMM_SIZE];
static void	*here_semaphore;

static void
malloc_insert(char *function, int line, void *pointer_given)
{
    int	i;
    struct MMM	*m, *found;

    semTake(here_semaphore, WAIT_FOREVER);
    found = NULL;
    for (i=0; i < hunsoo_malloc_limit; i++)
    {
        m = &MMMS[i];
        if (m->pointer == NULL)   // find empty
        {
            found = m;
            break;
        }
    }
    if (found == NULL)
    {
        found = &MMMS[hunsoo_malloc_limit];
        hunsoo_malloc_limit++;
    }
    found->pointer = pointer_given;
    found->line = line;
    strcpy(found->where, function);
    semGive(here_semaphore);
}

static void
malloc_delete(void *pointer_given)
{
    int	i;
    struct MMM	*m, *found;

    semTake(here_semaphore, WAIT_FOREVER);
    found = NULL;
    for (i=0; i < hunsoo_malloc_limit; i++)
    {
        m = &MMMS[i];
        if (m->pointer == pointer_given)
        {
            found = m;
            break;
        }
    }
    if (found == NULL)
    {
        fprintf(stderr, "%s(): ERR\n", __FUNCTION__);
    }
    else
    {
        found->pointer = NULL;
        strcpy(found->where, "");
    }
    semGive(here_semaphore);
}

void
hunsoo_malloc_display(void)
{
    int	i;
    struct MMM	*m;

    fprintf(stderr, "----- %d-----------------\n",
            hunsoo_malloc_limit);
    semTake(here_semaphore, WAIT_FOREVER);

    for (i=0; i < hunsoo_malloc_limit; i++)
    {
        m = &MMMS[i];
        if (m->pointer)
            fprintf(stderr, "%3d %4d %s\n", i, m->line,
                    m->where);
    }
    fprintf(stderr, "----- %d-----------------\n",
            hunsoo_malloc_limit);
    semGive(here_semaphore);
    return;
}


void *
hunsoo_malloc(char *function, int line, int size)
{
    void	*ret;

    ret = malloc(size+10);
    if (ret == NULL)
    {
        fprintf(stderr, "%s(): %s malloc err\n", __FUNCTION__,
                function);
        exit(1);
    }
#if 1
    if (here_semaphore == NULL)
    {
        fprintf(stderr, "%s(): here_semaphore create\n",
                __FUNCTION__);
        here_semaphore = VP_SEM_CREATE();
        VP_SEM_INIT(here_semaphore);
        semGive(here_semaphore);
    }
#endif

    malloc_insert(function, line, ret);
    return ret;
}


void
hunsoo_free(void *p)
{
    malloc_delete(p);
    free(p);
}


이렇게 해놓고
#define malloc(x)	hunsoo_malloc(__FUNCTION__, __LINE__, x)
#define free(x) hunsoo_free(x)
이렇게 한다음에

주기적으로 hunsoo_malloc_display()를 불러주곤 해서




