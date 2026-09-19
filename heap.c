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
