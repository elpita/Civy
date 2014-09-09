#include "civycontext.h"

int CVThreads_push(Q *self, PyGreenlet *data)
/* CVThread method: Push greenlets onto mini-queue */
{
    whatever *new_entry = (whatever *)malloc(sizeof(whatever));
    
    if (new_entry == NULL)
    {
        return -1;
        }

    new_entry->cvthread = data;
    Q_push(self, (QEntry *)new_entry);
    return 0;
    }


PyGreenlet* CVThreads_pop(Q *self)
/* CVThread method: Pop greenlets from mini-queue */
{
    whatever *entry = (whatever *)Q_pop(self)

    if (entry == NULL)
    {
        return NULL;
        }

    PyGreenlet *result = entry->cvthread;
    free(entry);
    return result;
