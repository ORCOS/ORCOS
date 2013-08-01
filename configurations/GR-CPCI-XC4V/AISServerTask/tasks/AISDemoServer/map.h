#ifndef MAP_H_
#define MAP_H_


#define BW_IO                   6
#define DEPTH                   3
#define AL                     20
#define MAXLOG // comment out for log-map
#define ESF    // comment out for switching off
#define MAX_NUM_THREADS     4


typedef struct {
	long *alpha_1;
	long *alpha_2;
	long *alpha_3;
	long *alpha_4;
	long *alpha_5;
	long *alpha_6;
	long *alpha_7;
	long *alpha_8;
	long *beta_1;
	long *beta_2;
	long *beta_3;
	long *beta_4;
	long *beta_5;
	long *beta_6;
	long *beta_7;
	long *beta_8;
	long *gamma_plus_beta;
	long *gamma0;
	long *gamma1;
	long *gamma2;
	long *gamma3;
	long *gamma0a;
	long *gamma1a;
	long *gamma2a;
	long *gamma3a;
	long *gamma0b;
	long *gamma1b;
	long *gamma2b;
	long *gamma3b;
	long ws;
	int tid;
	int tasks;
} map_memory;

typedef struct {
	map_memory *mem;
	long *x;
	long *y;
	long *x_tail;
	long *y_tail;
	long *z;
	long *extr;
	long *llr;
	int block_id;
} map_thread_data;

extern map_thread_data map_td[MAX_NUM_THREADS];

map_memory* alloc_map_memory(long blocklength, int tasks, int tid);

void map_memory_free(map_memory* mem);

void map_decoder( map_memory* mem, long *x, long *y, long *x_tail, long *y_tail, long *z, long *extr, long *llr );

void *map_decoder_t(void *argp);


#endif /*MAP_H_*/
