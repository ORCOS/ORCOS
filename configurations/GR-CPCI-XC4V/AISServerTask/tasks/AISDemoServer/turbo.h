// ===================================================================
// (C)opyright 2008
// 
// Institute of Microelectronic Systems
// Prof. Wehn
// University of Kaiserslautern, Germany
// 
// ===================================================================
// 
// CONFIDENTIAL
// 
// ===================================================================
// 
// Author(s):   May
// 
// ===================================================================
// 
// Project:     Turbo-Code Decoder for AIS AP2 Demonstrator
//
//              blocklength (info bits)   512
//              bitwidth input              6
// 
// ===================================================================


#ifndef TURBO_H_
#define TURBO_H_


#define MAX_HALFITERATIONS     16
#define LLRSUMTHRF             92
#define ITCTRL


#include "map.h"


typedef struct {
	 long* llr_il;
	 long* x_il;
	 long* extr_i;
	 long* extr_o;
	 long blocklength;
	 int tasks;
	 map_memory* map_mem[MAX_NUM_THREADS];
} turbo_memory;


turbo_memory* alloc_turbo_memory(long blocklen, int tasks);

void free_turbo_memory(turbo_memory* mem);

int turbo_decoder(turbo_memory* mem, long *x, long *y1, long *y2, long *x1_tail, long *x2_tail, long *y1_tail, long *y2_tail,
                   long *il, long *llr);

void generate_interleaver(long blockl, long interleaver_pattern[]);


#endif /*TURBO_H_*/
