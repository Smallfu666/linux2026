#define _GNU_SOURCE
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "list.h"

enum { NODE_COUNT = 32 };

static list_t g_runqueue;
static list_node_t g_nodes[NODE_COUNT];
static volatile sig_atomic_t g_ticks;
static int g_mode_safe;

static sigset_t g_mask;

static void spin_delay(unsigned count) {
    for (volatile unsigned i = 0; i < count; ++i) {
    }
}

static void runqueue_seed(void) {
    list_init(&g_runqueue);
    for (int i = 0; i < NODE_COUNT; ++i) {
        g_nodes[i].value = i;
        g_nodes[i].prev = NULL;
        g_nodes[i].next = NULL;
        list_add_back(&g_runqueue, &g_nodes[i]);
    }
}

static void simulate_context_switch(void) {
    list_node_t *head = g_runqueue.head;
    if (!head) {
        return;
    }
    list_remove(&g_runqueue, head);
    head->value ^= 1;
    list_add_back(&g_runqueue, head);
}

static void touch_runqueue(void) {
    for (list_node_t *node = g_runqueue.head; node; node = node->next) {
        node->value ^= 1;
    }
}

static void alarm_handler(int signo) {
    (void)signo;
    ++g_ticks;
    if (g_mode_safe) {
        simulate_context_switch();
    } else {
        list_node_t *tail = g_runqueue.tail;
        if (tail && tail->prev) {
            list_node_t *prev = tail->prev;
            list_remove_unchecked(&g_runqueue, tail);
            spin_delay(5000);
            list_remove_unchecked(&g_runqueue, prev);
            list_add_front(&g_runqueue, tail);
            list_add_back(&g_runqueue, prev);
        }
    }
}

static void set_timer_1ms(void) {
    struct itimerval timer;
    memset(&timer, 0, sizeof(timer));
    timer.it_interval.tv_usec = 1000;
    timer.it_value.tv_usec = 1000;
    if (setitimer(ITIMER_REAL, &timer, NULL) != 0) {
        perror("setitimer");
        exit(1);
    }
}

static void install_handler(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = alarm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGALRM, &sa, NULL) != 0) {
        perror("sigaction");
        exit(1);
    }
}

static void busy_mutate_unsafe(void) {
    unsigned long iter = 0;
    while (1) {
        list_node_t *node = &g_nodes[iter % NODE_COUNT];
        list_remove_unchecked(&g_runqueue, node);
        spin_delay(20000);
        touch_runqueue();
        list_add_front(&g_runqueue, node);
        ++iter;
        if ((iter & 0xFFFFFUL) == 0) {
            fprintf(stderr, "unsafe iter=%lu ticks=%d\n", iter, (int)g_ticks);
        }
    }
}

static void busy_mutate_safe(void) {
    unsigned long iter = 0;
    while (1) {
        if (sigprocmask(SIG_BLOCK, &g_mask, NULL) != 0) {
            perror("sigprocmask block");
            exit(1);
        }
        list_node_t *node = &g_nodes[iter % NODE_COUNT];
        list_remove(&g_runqueue, node);
        spin_delay(20000);
        touch_runqueue();
        list_add_front(&g_runqueue, node);
        if (sigprocmask(SIG_UNBLOCK, &g_mask, NULL) != 0) {
            perror("sigprocmask unblock");
            exit(1);
        }
        ++iter;
        if ((iter & 0xFFFFFUL) == 0) {
            fprintf(stderr, "safe iter=%lu ticks=%d\n", iter, (int)g_ticks);
        }
    }
}

int main(int argc, char **argv) {
    g_mode_safe = argc > 1 && strcmp(argv[1], "safe") == 0;
    if (argc > 1 && strcmp(argv[1], "unsafe") != 0 && strcmp(argv[1], "safe") != 0) {
        fprintf(stderr, "usage: %s [unsafe|safe]\n", argv[0]);
        return 1;
    }

    sigemptyset(&g_mask);
    sigaddset(&g_mask, SIGALRM);

    runqueue_seed();
    install_handler();
    set_timer_1ms();

    if (g_mode_safe) {
        puts("mode=safe");
        busy_mutate_safe();
    } else {
        puts("mode=unsafe");
        busy_mutate_unsafe();
    }
    return 0;
}
