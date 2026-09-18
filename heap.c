/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakhmae <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/18 18:51:04 by mmakhmae          #+#    #+#             */
/*   Updated: 2026/09/18 18:51:06 by mmakhmae         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* strict "a goes before b": smaller key first, ticket breaks ties. */
int	hn_before(t_hn *a, t_hn *b)
{
	if (a->key != b->key)
		return (a->key < b->key);
	return (a->tie < b->tie);
}

static void	hn_swap(t_hn *a, t_hn *b)
{
	t_hn	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

static void	sift_up(t_h *heap, int i)
{
	while (i > 0 && hn_before(&heap->array[i], &heap->array[(i - 1) / 2]))
	{
		hn_swap(&heap->array[i], &heap->array[(i - 1) / 2]);
		i = (i - 1) / 2;
	}
}

static void	sift_down(t_h *heap, int i)
{
	int	best;

	while (1)
	{
		best = i;
		if (2 * i + 1 < heap->size
			&& hn_before(&heap->array[2 * i + 1], &heap->array[best]))
			best = 2 * i + 1;
		if (2 * i + 2 < heap->size
			&& hn_before(&heap->array[2 * i + 2], &heap->array[best]))
			best = 2 * i + 2;
		if (best == i)
			return ;
		hn_swap(&heap->array[i], &heap->array[best]);
		i = best;
	}
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

void	heap_free(t_h *heap)
{
	free(heap->array);
	heap->array = NULL;
	heap->size = 0;
}

void	heap_push(t_h *heap, int id, long key, long tie)
{
	if (heap->size >= heap->capacity)
		return ;
	heap->array[heap->size].id = id;
	heap->array[heap->size].key = key;
	heap->array[heap->size].tie = tie;
	heap->size++;
	sift_up(heap, heap->size - 1);
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

void	heap_remove_id(t_h *heap, int id)
{
	int	i;

	i = heap_find(heap, id);
	if (i < 0)
		return ;
	heap->size--;
	if (i == heap->size)
		return ;
	heap->array[i] = heap->array[heap->size];
	sift_down(heap, i);
	sift_up(heap, i);
}

int	heap_peek_id(t_h *heap)
{
	if (heap->size == 0)
		return (-1);
	return (heap->array[0].id);
}
