#include "rd.h"
#include "rdlru.h"


static void rd_lru_elm_destroy (rd_lru_t *rlru, rd_lru_elm_t *rlrue) {
	if (rlru)
		TAILQ_REMOVE(&rlru->rlru_elms, rlrue, rlrue_link);
	free(rlrue);
}


void rd_lru_destroy (rd_lru_t *rlru) {
	rd_lru_elm_t *rlrue;

	while ((rlrue = TAILQ_FIRST(&rlru->rlru_elms)))
		rd_lru_elm_destroy(rlru, rlrue);

	free(rlru);
}


rd_lru_t *rd_lru_new (void) {
	rd_lru_t *rlru;

	rlru = calloc(1, sizeof(*rlru));

	TAILQ_INIT(&rlru->rlru_elms);

	return rlru;
}


void rd_lru_push (rd_lru_t *rlru, void *ptr) {
	rd_lru_elm_t *rlrue;

	rlrue = calloc(1, sizeof(*rlrue));
	rlrue->rlrue_ptr = ptr;

	TAILQ_INSERT_HEAD(&rlru->rlru_elms, rlrue, rlrue_link);
	rlru->rlru_cnt++;
}


void *rd_lru_pop (rd_lru_t *rlru) {
	rd_lru_elm_t *rlrue;
	void *ptr;

	if ((rlrue = TAILQ_LAST(&rlru->rlru_elms, rd_lru_elm_head))) {
		ptr = rlrue->rlrue_ptr;
		rd_lru_elm_destroy(rlru, rlrue);
	} else
		ptr = NULL;

	return ptr;
}


void *rd_lru_shift (rd_lru_t *rlru) {
	rd_lru_elm_t *rlrue;
	void *ptr;

	if ((rlrue = TAILQ_FIRST(&rlru->rlru_elms))) {
		ptr = rlrue->rlrue_ptr;
		rd_lru_elm_destroy(rlru, rlrue);
	} else
		ptr = NULL;

	return ptr;
}
