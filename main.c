/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakhmae <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 15:46:57 by mmakhmae          #+#    #+#             */
/*   Updated: 2026/06/09 15:47:08 by mmakhmae         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

pthread_mutex_t mutex;

void* test_func(void* arg) {
    pthread_mutex_lock(&mutex);
    printf("Hello from the new thread!\n");
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main()
{
    printf("\n===Codexion===\n\n");
    pthread_t thid;
    void *ret;
    thid = malloc(sizeof(pthread_t) * 1);
    
    pthread_create(&thid, NULL, test_func, NULL);
    pthread_join(thid, &ret);

    printf("thread exited with '%p'\n", ret);
    return 0;
}