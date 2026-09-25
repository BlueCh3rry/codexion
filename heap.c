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

static void	bubble_up(t_request *requests, int i)
{
	int	parent_idx;

	while (i > 0)
	{
		parent_idx = (i - 1) / 2;
		if (requests[i].metric < requests[parent_idx].metric)
			swap(&requests[parent_idx], &requests[i]);
		else
			break ;
		i = parent_idx;
	}
}

static void	heapify(t_request *requests, int parent, int size)
{
	int	left_child_idx;
	int	right_child_idx;
	int	least;

	least = parent;
	while (1)
	{
		left_child_idx = (parent * 2) + 1;
		right_child_idx = (parent * 2) + 2;
		if (left_child_idx < size && requests[left_child_idx].metric
			< requests[parent].metric)
			least = left_child_idx;
		if (right_child_idx < size && requests[right_child_idx].metric
			< requests[least].metric)
			least = right_child_idx;
		if (least == parent)
			break ;
		swap(&requests[least], &requests[parent]);
		parent = least;
	}
}

void	add_request(t_heap *heap, int id, long metric)
{
	if (!heap || (heap->size == 2))
		return ;
	heap->requests[heap->size].id = id;
	heap->requests[heap->size].metric = metric;
	bubble_up(heap->requests, heap->size);
	heap->size += 1;
}

void	remove_request_top(t_heap *heap)
{
	if (!heap || !heap->size)
		return ;
	if (&heap->requests[0] != &heap->requests[heap->size - 1])
		swap(&heap->requests[0], &heap->requests[heap->size - 1]);
	heap->size -= 1;
	heapify(heap->requests, 0, heap->size);
	return ;
}

/* heaps are protected by state_mutex (held by the caller), not by the
** dongle mutex, because the dongle mutex is held during a whole compile. */
void	pre_register_heaps(t_d *first, t_d *second,
	t_c *coder, long metric)
{
	add_request(&first->heap, coder->id, metric);
	add_request(&second->heap, coder->id, metric);
}
