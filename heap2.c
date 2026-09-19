#include "codexion.h"

/* strict "a goes before b": smaller key first, ticket breaks ties. */
int	hn_before(t_hn *a, t_hn *b)
{
	if (a->key != b->key)
		return (a->key < b->key);
	return (a->tie < b->tie);
}

void	heap_free(t_h *heap)
{
	free(heap->array);
	heap->array = NULL;
	heap->size = 0;
}

int	heap_init(t_h *heap, int capacity)
{
	heap->array = malloc(sizeof(t_hn) * capacity);
	if (!heap->array)
		return (printf("Error malloc heap\n"), -1);
	heap->size = 0;
	heap->capacity = capacity;
	return (0);
}

int	heap_peek_id(t_h *heap)
{
	if (heap->size == 0)
		return (-1);
	return (heap->array[0].id);
}

int	heap_find(t_h *heap, int id)
{
	int	i;

	i = 0;
	while (i < heap->size)
	{
		if (heap->array[i].id == id)
			return (i);
		i++;
	}
	return (-1);
}