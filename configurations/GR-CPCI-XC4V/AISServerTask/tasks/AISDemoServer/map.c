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
// Project:     Turbo-Code Decoder for AIS Demonstrator
//              MAP Decoder
//
//              blocklength (info bits)   512
//              bitwidth input              6
//
// ===================================================================

#include "orcos.hh"
#include <stdlib.h>
#include "map.h"


map_thread_data map_td[MAX_NUM_THREADS];


map_memory* alloc_map_memory(long ws, int tasks, int tid)
{
	int aacq = 0;
	int bacq = DEPTH;

	if (tid>0) aacq = AL;
	if (tid<=tasks-1) bacq = AL;

	//map_memory* mem = (map_memory*) malloc(sizeof(map_memory));
	map_memory* mem = (map_memory*) mallocp(sizeof(map_memory), 15);

	mem->alpha_1 = (long*)mallocp(ws * sizeof(long), 3);
	mem->alpha_2 = (long*)mallocp(ws * sizeof(long), 3);
	mem->alpha_3 = (long*)mallocp(ws * sizeof(long), 3);
	mem->alpha_4 = (long*)mallocp(ws * sizeof(long), 3);
	mem->alpha_5 = (long*)mallocp(ws * sizeof(long), 3);
	mem->alpha_6 = (long*)mallocp(ws * sizeof(long), 3);
	mem->alpha_7 = (long*)mallocp(ws * sizeof(long), 3);
	mem->alpha_8 = (long*)mallocp(ws * sizeof(long), 3);
	mem->beta_1 = (long*)mallocp((ws+1) * sizeof(long), 3);
	mem->beta_2 = (long*)mallocp((ws+1) * sizeof(long), 3);
	mem->beta_3 = (long*)mallocp((ws+1) * sizeof(long), 3);
	mem->beta_4 = (long*)mallocp((ws+1) * sizeof(long), 3);
	mem->beta_5 = (long*)mallocp((ws+1) * sizeof(long), 3);
	mem->beta_6 = (long*)mallocp((ws+1) * sizeof(long), 3);
	mem->beta_7 = (long*)mallocp((ws+1) * sizeof(long), 3);
	mem->beta_8 = (long*)mallocp((ws+1) * sizeof(long), 3);
	mem->gamma_plus_beta = (long*)mallocp(17 * sizeof(long), 3);
	mem->gamma0 = (long*)mallocp(ws * sizeof(long), 3);
	mem->gamma1 = (long*)mallocp(ws * sizeof(long), 3);
	mem->gamma2 = (long*)mallocp(ws * sizeof(long), 3);
	mem->gamma3 = (long*)mallocp(ws * sizeof(long), 3);
	mem->gamma0a = (long*)mallocp(aacq * sizeof(long), 3);
	mem->gamma1a = (long*)mallocp(aacq * sizeof(long), 3);
	mem->gamma2a = (long*)mallocp(aacq * sizeof(long), 3);
	mem->gamma3a = (long*)mallocp(aacq * sizeof(long), 3);
	mem->gamma0b = (long*)mallocp(bacq * sizeof(long), 3);
	mem->gamma1b = (long*)mallocp(bacq * sizeof(long), 3);
	mem->gamma2b = (long*)mallocp(bacq * sizeof(long), 3);
	mem->gamma3b = (long*)mallocp(bacq * sizeof(long), 3);
	mem->ws = ws;
	mem->tasks = tasks;
	mem->tid = tid;
	return mem;
}

void map_memory_free(map_memory* mem)
{
	if (!mem)
		return;

	// free memory
	free(mem->alpha_1);
	free(mem->alpha_2);
	free(mem->alpha_3);
	free(mem->alpha_4);
	free(mem->alpha_5);
	free(mem->alpha_6);
	free(mem->alpha_7);
	free(mem->alpha_8);
	free(mem->beta_1);
	free(mem->beta_2);
	free(mem->beta_3);
	free(mem->beta_4);
	free(mem->beta_5);
	free(mem->beta_6);
	free(mem->beta_7);
	free(mem->beta_8);
	free(mem->gamma_plus_beta);
	free(mem->gamma0);
	free(mem->gamma1);
	free(mem->gamma2);
	free(mem->gamma3);
	free(mem->gamma0a);
	free(mem->gamma1a);
	free(mem->gamma2a);
	free(mem->gamma3a);
	free(mem->gamma0b);
	free(mem->gamma1b);
	free(mem->gamma2b);
	free(mem->gamma3b);

	free(mem);
}

const long powers_of_two [] =
{
  1, 2, 4, 8, 16, 32, 64, 128, 256, 512,
  1024, 2048, 4096, 8192, 16384, 32768,
  65536, 131072, 262144, 524288, 1048576
};

long saturate( long a, long bw )
{
  long min = -powers_of_two[bw-1];
  long max = powers_of_two[bw-1]-1;

  if( a < min )
    return -powers_of_two[bw-1];
  else if( a > max )
    return powers_of_two[bw-1]-1;
  else
    return a;
}

long minstar( long a, long b )
{
	#ifdef MAXLOG
		long min;

		if (a >= b)
			min = b;
		else
			min = a;

		return min;

	#else

		long lut[9] ={ 3, 2, 2, 2, 1, 1, 1, 1, 0 };
		long diff,min,d;

		diff = a - b;
		if (diff  >= 0)
		{
			min = b;
			if (diff > 8)
				d = 0;
			else
				d = lut[diff];
		} else {
			min = a;
			diff = -diff;
			if (diff > 8)
				d = 0;
			else
				d = lut[diff];
		}

		return min - d;
	#endif
}

long minstar4( long a, long b, long c, long d )
{
  return minstar( minstar( a, b ),
		          minstar( c, d ));
}

long minstar8( long a, long b, long c, long d,
               long e, long f, long g, long h)
{
  return minstar( minstar4( a, b, c, d ),
                  minstar4( e, f, g, h ));
}


void *map_decoder_t(void *argp)
{
	long i;
	long xn, pn, zn;

	map_thread_data *map_ptr = (map_thread_data*)argp;
	long *x = map_ptr->x;
	long *y = map_ptr->y;
	long *x_tail = map_ptr->x_tail;
	long *y_tail = map_ptr->y_tail;
	long *z = map_ptr->z;
	long *extr = map_ptr->extr;
	long *llr = map_ptr->llr;

	long *alpha_1 = map_ptr->mem->alpha_1;
	long *alpha_2 = map_ptr->mem->alpha_2;
	long *alpha_3 = map_ptr->mem->alpha_3;
	long *alpha_4 = map_ptr->mem->alpha_4;
	long *alpha_5 = map_ptr->mem->alpha_5;
	long *alpha_6 = map_ptr->mem->alpha_6;
	long *alpha_7 = map_ptr->mem->alpha_7;
	long *alpha_8 = map_ptr->mem->alpha_8;
	long *beta_1 = map_ptr->mem->beta_1;
	long *beta_2 = map_ptr->mem->beta_2;
	long *beta_3 = map_ptr->mem->beta_3;
	long *beta_4 = map_ptr->mem->beta_4;
	long *beta_5 = map_ptr->mem->beta_5;
	long *beta_6 = map_ptr->mem->beta_6;
	long *beta_7 = map_ptr->mem->beta_7;
	long *beta_8 = map_ptr->mem->beta_8;
	long *gamma_plus_beta = map_ptr->mem->gamma_plus_beta;
	long *gamma0 = map_ptr->mem->gamma0;
	long *gamma1 = map_ptr->mem->gamma1;
	long *gamma2 = map_ptr->mem->gamma2;
	long *gamma3 = map_ptr->mem->gamma3;
	long *gamma0a = map_ptr->mem->gamma0a;
	long *gamma1a = map_ptr->mem->gamma1a;
	long *gamma2a = map_ptr->mem->gamma2a;
	long *gamma3a = map_ptr->mem->gamma3a;
	long *gamma0b = map_ptr->mem->gamma0b;
	long *gamma1b = map_ptr->mem->gamma1b;
	long *gamma2b = map_ptr->mem->gamma2b;
	long *gamma3b = map_ptr->mem->gamma3b;

	long ws = map_ptr->mem->ws;
	int tasks = map_ptr->mem->tasks;
	int tid = map_ptr->mem->tid;

	long extrinsic, llr0, llr1, tmp_llr, betamin, alphamin;

#ifdef CONSOLE_DEBUG
	printf("Calculate Branch-Metrics \n\r");
#endif

	// ===================================================================
	// Calculate Branch-Metrics (Gammas)
	// ===================================================================

	if (tid != 0) // alpha acq
	{
		for( i = 0; i < AL; i++ )
		{
			xn = x[i-AL];
			pn = y[i-AL];
			zn = z[i-AL];

			gamma0a[i]     = 0;
			gamma1a[i]     = pn;
			gamma2a[i]     = xn + zn;
			gamma3a[i]     = xn + zn + pn;
		}
	}

	if (tid < tasks-1) // beta acq
	{
		for( i = 0; i < AL; i++ )
		{
			xn = x[ws+i];
			pn = y[ws+i];
			zn = z[ws+i];

			gamma0b[i]     = 0;
			gamma1b[i]     = pn;
			gamma2b[i]     = xn + zn;
			gamma3b[i]     = xn + zn + pn;
		}
	}
	else  // tailing
	{
		for( i = 0; i < DEPTH; i++ )
		{
			xn = x_tail[i];
			pn = y_tail[i];

			gamma0b[i] = 0;
			gamma1b[i] = pn;
			gamma2b[i] = xn;
			gamma3b[i] = xn + pn;
		}
	}

	for( i = 0; i < ws; i++ )
	{
		xn = x[i];
		pn = y[i];
		zn = z[i];

		gamma0[i]     = 0;
		gamma1[i]     = pn;
		gamma2[i]     = xn + zn;
		gamma3[i]     = xn + zn + pn;
	}

#ifdef CONSOLE_DEBUG
	printf("Calculate State-Metrics (1. Alphas) \n\r");
#endif
	// ===================================================================
	// Calculate State-Metrics (1. Alphas)
	// ===================================================================

	// alpha[0] initilization
	if (tid != 0)  // alpha acq
	{
		alpha_1[0] = 0;
		alpha_2[0] = 0;
		alpha_3[0] = 0;
		alpha_4[0] = 0;
		alpha_5[0] = 0;
		alpha_6[0] = 0;
		alpha_7[0] = 0;
		alpha_8[0] = 0;
		for( i = 0 ; i < AL ; i++ )
		{
			alpha_1[i+1] = minstar( gamma0a[i] + alpha_1[i], gamma3a[i] + alpha_2[i]);
			alpha_2[i+1] = minstar( gamma1a[i] + alpha_4[i], gamma2a[i] + alpha_3[i]);
			alpha_3[i+1] = minstar( gamma1a[i] + alpha_5[i], gamma2a[i] + alpha_6[i]);
			alpha_4[i+1] = minstar( gamma0a[i] + alpha_8[i], gamma3a[i] + alpha_7[i]);
			alpha_5[i+1] = minstar( gamma0a[i] + alpha_2[i], gamma3a[i] + alpha_1[i]);
			alpha_6[i+1] = minstar( gamma1a[i] + alpha_3[i], gamma2a[i] + alpha_4[i]);
			alpha_7[i+1] = minstar( gamma1a[i] + alpha_6[i], gamma2a[i] + alpha_5[i]);
			alpha_8[i+1] = minstar( gamma0a[i] + alpha_7[i], gamma3a[i] + alpha_8[i]);

			alphamin = minstar8(alpha_1[i+1],alpha_2[i+1],alpha_3[i+1],alpha_4[i+1],alpha_5[i+1],alpha_6[i+1],alpha_7[i+1],alpha_8[i+1]);
			alpha_1[i+1] -=  alphamin;
			alpha_2[i+1] -=  alphamin;
			alpha_3[i+1] -=  alphamin;
			alpha_4[i+1] -=  alphamin;
			alpha_5[i+1] -=  alphamin;
			alpha_6[i+1] -=  alphamin;
			alpha_7[i+1] -=  alphamin;
			alpha_8[i+1] -=  alphamin;
		}
		alpha_1[0] = alpha_1[AL];
		alpha_2[0] = alpha_2[AL];
		alpha_3[0] = alpha_3[AL];
		alpha_4[0] = alpha_4[AL];
		alpha_5[0] = alpha_5[AL];
		alpha_6[0] = alpha_6[AL];
		alpha_7[0] = alpha_7[AL];
		alpha_8[0] = alpha_8[AL];
	}
	else
	{
		alpha_1[0] = 0;
		alpha_2[0] = powers_of_two[BW_IO+2];  //BW_SM - 2
		alpha_3[0] = powers_of_two[BW_IO+2];
		alpha_4[0] = powers_of_two[BW_IO+2];
		alpha_5[0] = powers_of_two[BW_IO+2];
		alpha_6[0] = powers_of_two[BW_IO+2];
		alpha_7[0] = powers_of_two[BW_IO+2];
		alpha_8[0] = powers_of_two[BW_IO+2];
	}

	// forward recursion
	for( i = 0 ; i < ws-1 ; i++ )
	{
		alpha_1[i+1] = minstar( gamma0[i] + alpha_1[i], gamma3[i] + alpha_2[i]);
		alpha_2[i+1] = minstar( gamma1[i] + alpha_4[i], gamma2[i] + alpha_3[i]);
		alpha_3[i+1] = minstar( gamma1[i] + alpha_5[i], gamma2[i] + alpha_6[i]);
		alpha_4[i+1] = minstar( gamma0[i] + alpha_8[i], gamma3[i] + alpha_7[i]);
		alpha_5[i+1] = minstar( gamma0[i] + alpha_2[i], gamma3[i] + alpha_1[i]);
		alpha_6[i+1] = minstar( gamma1[i] + alpha_3[i], gamma2[i] + alpha_4[i]);
		alpha_7[i+1] = minstar( gamma1[i] + alpha_6[i], gamma2[i] + alpha_5[i]);
		alpha_8[i+1] = minstar( gamma0[i] + alpha_7[i], gamma3[i] + alpha_8[i]);

		alphamin = minstar8(alpha_1[i+1],alpha_2[i+1],alpha_3[i+1],alpha_4[i+1],alpha_5[i+1],alpha_6[i+1],alpha_7[i+1],alpha_8[i+1]);
		alpha_1[i+1] -=  alphamin;
		alpha_2[i+1] -=  alphamin;
		alpha_3[i+1] -=  alphamin;
		alpha_4[i+1] -=  alphamin;
		alpha_5[i+1] -=  alphamin;
		alpha_6[i+1] -=  alphamin;
		alpha_7[i+1] -=  alphamin;
		alpha_8[i+1] -=  alphamin;
	}

	//printf("Calculate State-Metrics (2. Betas) \n\r");
	// ===================================================================
	// Calculate State-Metrics (2. Betas)
	// ===================================================================

	// beta initilization
	if (tid < tasks-1)  // beta acq
	{
		beta_1[AL] = 0;
		beta_2[AL] = 0;
		beta_3[AL] = 0;
		beta_4[AL] = 0;
		beta_5[AL] = 0;
		beta_6[AL] = 0;
		beta_7[AL] = 0;
		beta_8[AL] = 0;
		for ( i = AL-1; i >= 0; i-- )
		{
			beta_1[i] = minstar( gamma0b[i] + beta_1[i+1], gamma3b[i] + beta_5[i+1]);
			beta_2[i] = minstar( gamma0b[i] + beta_5[i+1], gamma3b[i] + beta_1[i+1]);
			beta_3[i] = minstar( gamma1b[i] + beta_6[i+1], gamma2b[i] + beta_2[i+1]);
			beta_4[i] = minstar( gamma1b[i] + beta_2[i+1], gamma2b[i] + beta_6[i+1]);
			beta_5[i] = minstar( gamma1b[i] + beta_3[i+1], gamma2b[i] + beta_7[i+1]);
			beta_6[i] = minstar( gamma1b[i] + beta_7[i+1], gamma2b[i] + beta_3[i+1]);
			beta_7[i] = minstar( gamma0b[i] + beta_8[i+1], gamma3b[i] + beta_4[i+1]);
			beta_8[i] = minstar( gamma0b[i] + beta_4[i+1], gamma3b[i] + beta_8[i+1]);

			betamin = minstar8(beta_1[i],beta_2[i],beta_3[i],beta_4[i],beta_5[i],beta_6[i],beta_7[i],beta_8[i]);
			beta_1[i] -=  betamin;
			beta_2[i] -=  betamin;
			beta_3[i] -=  betamin;
			beta_4[i] -=  betamin;
			beta_5[i] -=  betamin;
			beta_6[i] -=  betamin;
			beta_7[i] -=  betamin;
			beta_8[i] -=  betamin;
		}
	}
	else  // tailing
	{
		beta_1[DEPTH] = 0;
		beta_2[DEPTH] = powers_of_two[BW_IO+2];
		beta_3[DEPTH] = powers_of_two[BW_IO+2];
		beta_4[DEPTH] = powers_of_two[BW_IO+2];
		beta_5[DEPTH] = powers_of_two[BW_IO+2];
		beta_6[DEPTH] = powers_of_two[BW_IO+2];
		beta_7[DEPTH] = powers_of_two[BW_IO+2];
		beta_8[DEPTH] = powers_of_two[BW_IO+2];
		for ( i = DEPTH-1; i >= 0; i-- )
		{
			beta_1[i] = minstar( gamma0b[i] + beta_1[i+1], gamma3b[i] + beta_5[i+1]);
			beta_2[i] = minstar( gamma0b[i] + beta_5[i+1], gamma3b[i] + beta_1[i+1]);
			beta_3[i] = minstar( gamma1b[i] + beta_6[i+1], gamma2b[i] + beta_2[i+1]);
			beta_4[i] = minstar( gamma1b[i] + beta_2[i+1], gamma2b[i] + beta_6[i+1]);
			beta_5[i] = minstar( gamma1b[i] + beta_3[i+1], gamma2b[i] + beta_7[i+1]);
			beta_6[i] = minstar( gamma1b[i] + beta_7[i+1], gamma2b[i] + beta_3[i+1]);
			beta_7[i] = minstar( gamma0b[i] + beta_8[i+1], gamma3b[i] + beta_4[i+1]);
			beta_8[i] = minstar( gamma0b[i] + beta_4[i+1], gamma3b[i] + beta_8[i+1]);

			betamin = minstar8(beta_1[i],beta_2[i],beta_3[i],beta_4[i],beta_5[i],beta_6[i],beta_7[i],beta_8[i]);
			beta_1[i] -=  betamin;
			beta_2[i] -=  betamin;
			beta_3[i] -=  betamin;
			beta_4[i] -=  betamin;
			beta_5[i] -=  betamin;
			beta_6[i] -=  betamin;
			beta_7[i] -=  betamin;
			beta_8[i] -=  betamin;
		}
	}
	beta_1[ws] = beta_1[0];
	beta_2[ws] = beta_2[0];
	beta_3[ws] = beta_3[0];
	beta_4[ws] = beta_4[0];
	beta_5[ws] = beta_5[0];
	beta_6[ws] = beta_6[0];
	beta_7[ws] = beta_7[0];
	beta_8[ws] = beta_8[0];

	// backward recursion
	for ( i = ws-1; i >= 1 ; i-- )
	{
		beta_1[i] = minstar( gamma0[i] + beta_1[i+1], gamma3[i] + beta_5[i+1]);
		beta_2[i] = minstar( gamma0[i] + beta_5[i+1], gamma3[i] + beta_1[i+1]);
		beta_3[i] = minstar( gamma1[i] + beta_6[i+1], gamma2[i] + beta_2[i+1]);
		beta_4[i] = minstar( gamma1[i] + beta_2[i+1], gamma2[i] + beta_6[i+1]);
		beta_5[i] = minstar( gamma1[i] + beta_3[i+1], gamma2[i] + beta_7[i+1]);
		beta_6[i] = minstar( gamma1[i] + beta_7[i+1], gamma2[i] + beta_3[i+1]);
		beta_7[i] = minstar( gamma0[i] + beta_8[i+1], gamma3[i] + beta_4[i+1]);
		beta_8[i] = minstar( gamma0[i] + beta_4[i+1], gamma3[i] + beta_8[i+1]);

		betamin = minstar8(beta_1[i],beta_2[i],beta_3[i],beta_4[i],beta_5[i],beta_6[i],beta_7[i],beta_8[i]);
		beta_1[i] -=  betamin;
		beta_2[i] -=  betamin;
		beta_3[i] -=  betamin;
		beta_4[i] -=  betamin;
		beta_5[i] -=  betamin;
		beta_6[i] -=  betamin;
		beta_7[i] -=  betamin;
		beta_8[i] -=  betamin;
	}

#ifdef CONSOLE_DEBUG
	printf("Calculate the LLR-values \n\r");
#endif

	// ===================================================================
	// Calculate the LLR-values
	// ===================================================================

	for( i = 0; i < ws; i++ )
	{
		/* calculate the 16 different sums for llr calculation */
		gamma_plus_beta[1 ] = gamma3[i] + beta_1[i+1];
		gamma_plus_beta[2 ] = gamma2[i] + beta_2[i+1];
		gamma_plus_beta[3 ] = gamma2[i] + beta_3[i+1];
		gamma_plus_beta[4 ] = gamma3[i] + beta_4[i+1];
		gamma_plus_beta[5 ] = gamma3[i] + beta_5[i+1];
		gamma_plus_beta[6 ] = gamma2[i] + beta_6[i+1];
		gamma_plus_beta[7 ] = gamma2[i] + beta_7[i+1];
		gamma_plus_beta[8 ] = gamma3[i] + beta_8[i+1];
		gamma_plus_beta[9 ] = gamma0[i] + beta_1[i+1];
		gamma_plus_beta[10] = gamma1[i] + beta_2[i+1];
		gamma_plus_beta[11] = gamma1[i] + beta_3[i+1];
		gamma_plus_beta[12] = gamma0[i] + beta_4[i+1];
		gamma_plus_beta[13] = gamma0[i] + beta_5[i+1];
		gamma_plus_beta[14] = gamma1[i] + beta_6[i+1];
		gamma_plus_beta[15] = gamma1[i] + beta_7[i+1];
		gamma_plus_beta[16] = gamma0[i] + beta_8[i+1];

		llr1 = minstar8( alpha_2[i] + gamma_plus_beta[1],
				alpha_3[i] + gamma_plus_beta[2],
				alpha_6[i] + gamma_plus_beta[3],
				alpha_7[i] + gamma_plus_beta[4],
				alpha_1[i] + gamma_plus_beta[5],
				alpha_4[i] + gamma_plus_beta[6],
				alpha_5[i] + gamma_plus_beta[7],
				alpha_8[i] + gamma_plus_beta[8]);

		llr0 = minstar8( alpha_1[i] + gamma_plus_beta[ 9],
				alpha_4[i] + gamma_plus_beta[10],
				alpha_5[i] + gamma_plus_beta[11],
				alpha_8[i] + gamma_plus_beta[12],
				alpha_2[i] + gamma_plus_beta[13],
				alpha_3[i] + gamma_plus_beta[14],
				alpha_6[i] + gamma_plus_beta[15],
				alpha_7[i] + gamma_plus_beta[16]);

		tmp_llr   = llr1 - llr0;
		extrinsic = tmp_llr - gamma2[i];
		llr [i]   = tmp_llr;

#ifdef MAXLOG
#ifdef ESF
		if (extrinsic < 0) extrinsic = (3*extrinsic-3) / 4;
		else               extrinsic = (3*extrinsic  ) / 4;
#endif
#endif
		extr[i] = extrinsic;
	}
	//pthread_exit(NULL);
}
