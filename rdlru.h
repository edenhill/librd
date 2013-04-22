#pragma once

#include "rdsysqueue.h"



typedef struct rd_lru_elm_s {
	TAILQ_ENTRY(rd_lru_elm_s)  rlrue_link;
	void       *rlrue_ptr;
} rd_lru_elm_t;

TAILQ_HEAD(rd_lru_elm_head, rd_lru_elm_s);


typedef struct rd_lru_s {
	rd_mutex_t  rlru_lock;
	rd_cond_t   rlru_cond;
	int         rlru_cnt;
	struct rd_lru_elm_head rlru_elms;
} rd_lru_t;


#define RD_LRU_INITIALIZER(st) \
	{ rlru_elms: TAILQ_HEAD_INITIALIZER(st.rlru_elms) }

#define rd_lru_lock(rlru)   rd_mutex_lock(&(rlru)->rlru_lock)
#define rd_lru_unlock(rlru) rd_mutex_unlock(&(rlru)->rlru_lock)


/**
 * Destroys an LRU
 */
void rd_lru_destroy (rd_lru_t *rlru);


/**
 * Creates a new LRU
 */
rd_lru_t *rd_lru_new (void);


/**
 * Push a new entry to the LRU.
 */
void rd_lru_push (rd_lru_t *rlru, void *ptr);

/**
 * Returns and removes the oldest entry in the LRU, or NULL.
 */
void *rd_lru_pop (rd_lru_t *rlru);

/**
 * Returns and removes the youngest entry in the LRU, or NULL.
 */
void *rd_lru_shift (rd_lru_t *rlru);
