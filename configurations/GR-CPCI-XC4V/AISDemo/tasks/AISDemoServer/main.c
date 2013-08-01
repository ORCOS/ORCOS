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
// Project:     Turbo-Coding Simulation Chain for AIS Demonstrator
//
// ===================================================================



#include "orcos.hh"
#include "turbo.h"
#include <math.h>
#include <stdlib.h>

#define DEMOBOARD

//#define DEMOBOARD         // use hardware Turbo Decoding simulation chain
//#define CONSOLE_DEBUG     // print debug messages (use only with a small number of blocks!)
#define BLOCKLENGTH   512   // do not change (unless you change the hardware)
#define BLOCKS_MIN    50     // minimum number of blocks to simulate
#define MIN_FE        0     // minimum number of frame errors
#define SNR_FROM  -4.0      // SNR start point (of AWGN channel in Turbo Decoding simulation chain)
#define SNR_TO    -4.0      //
#define SNR_STEP  0.5         // increment SNR by SNR_STEP
#define IFR_START 5         // Injected Failure Rate: if 3<=IFR_START<=8: 1e-IFR_START, else: 0; start point
#define IFR_STOP  3         //

#ifdef DEMOBOARD
#define DIN_START_ADDR     0xc0000000
#define DOUT_START_ADDR    0xc8000000
#define ERROR_NUMBER       0xcf000000
#define SNR_ADDR           0xce000000
#define AWGN_VALID_ADDR    0xcd000000
#define PROT_ADDR          0xcc000000
#define FINJ_ADDR          0xcc100000
#define FAIL_ADDR          0xcb000000
#define CPC_ADDR           0xca000000

#else
#define CWLENGTH (3*BLOCKLENGTH+4*DEPTH)
#endif

#ifndef DEMOBOARD
#include "encoder.h"
#endif


typedef struct {
	int i;
	unsigned int block;
	unsigned int blocks;
	unsigned int message;
	unsigned long block_errors;
	unsigned long bit_errors;
	unsigned long hisum;
	long est_fac;
	long errors;
	double snr;
	double snr_calc;
	long snr_xilinx;
	double actual_snr_xilinx;
	int ifr;
	volatile long *hw_error_number; // test result of error monitor
	volatile long *hw_awgn_valid; // addr for getting AWGN valid signal
	volatile long *hw_dout;  // start of output bits
	volatile long *hw_snr; // snr setting
	volatile unsigned long *hw_finj;
	volatile unsigned long *hw_fail;
	volatile unsigned long *sm_be;
	volatile unsigned long *sm_fe;
	volatile unsigned long *sm_blocks;
	volatile int *dec_start_fin;
} tb_mem;


typedef struct {
	long *x;
	long *y1;
	long *y2;
	long *llr;
	long *x1_tail;
	long *y1_tail;
	long *x2_tail;
	long *y2_tail;
	turbo_memory *tmem;
} turbo_tbmem;

turbo_tbmem* alloc_turbo_tbmem (int tasks)
{
	turbo_tbmem *t = (turbo_tbmem*)malloc(sizeof(turbo_tbmem));
	t->x = (long*)mallocp(BLOCKLENGTH * sizeof(long), 4);
	t->y1 = (long*)mallocp(BLOCKLENGTH * sizeof(long), 4);
	t->y2 = (long*)mallocp(BLOCKLENGTH * sizeof(long), 4);
	t->llr = (long*)mallocp(BLOCKLENGTH * sizeof(long), 8);
	t->x1_tail = (long*)mallocp(DEPTH * sizeof(long), 4);
	t->y1_tail = (long*)mallocp(DEPTH * sizeof(long), 4);
	t->x2_tail = (long*)mallocp(DEPTH * sizeof(long), 4);
	t->y2_tail = (long*)mallocp(DEPTH * sizeof(long), 4);
	t->tmem = alloc_turbo_memory(BLOCKLENGTH, tasks);
	return t;
}

// number of tasks used for decoding a block (== number of map decoders)
#define tasks 2

#ifndef DEMOBOARD
    long d[BLOCKLENGTH];
    long encout[CWLENGTH];
    unsigned long rand1, rand2;
    double ri[CWLENGTH];
    double s2n_lin;
    double r1, r2;
    double ni1, ni2;
    long rk[CWLENGTH];
#endif


// variable declarations!
turbo_tbmem *u;
long *il;
volatile long *hw_enc_din;
tb_mem *tb;

#ifndef DEMOBOARD
void generate_input_data()
{
    int i;
    // generate input data!

    s2n_lin = pow(10.0, -(tb->snr/20.0)) * sqrt(0.5);  //*sqrt(0.5): komplex channel

    // generate bits
    for (i = 0; i<BLOCKLENGTH; i++) d[i] = 1 & rand();

#ifdef CONSOLE_DEBUG
    printf("Encoding block %d \n\r",tb->block);
#endif
    // encode
    encode(d, encout, il, BLOCKLENGTH);

#ifdef CONSOLE_DEBUG
    printf("Done Encoding \n\r");
#endif
    // awgn channel
    for(i = 0; i < CWLENGTH; i += 2)
    {
        do
        {
            rand1 = rand();
        } while (rand1==0);
        do
        {
            rand2 = rand();
        } while (rand2==0);

        r1 = sqrt( -2.0 * log( rand1 / (double)RAND_MAX) );
        r2 = 2*M_PI * (  rand2 / (double)RAND_MAX);

        ni1 = r1*cos(r2);
        ni2 = r1*sin(r2);

        ni1 *= s2n_lin;
        ni2 *= s2n_lin;
        ri[i]   = ni1 + encout[i];
        ri[i+1] = ni2 + encout[i+1];
    }

#ifdef CONSOLE_DEBUG
    printf("Done simulating the AWGN Channel \n\r");
#endif
    // quantization, saturation
    for(i = 0; i < CWLENGTH ; i++)
    {
        rk[i] = floor(ri[i] * 4);
        if(rk[i] > 31) rk[i] = 31;
        if(rk[i] < -32) rk[i] = -32;
    }

    // prepare input data for decoder
    hw_enc_din = rk;
    for (i = 0; i<BLOCKLENGTH; i++)
    {
        u->x [i] = *(hw_enc_din++);
        u->y1[i] = *(hw_enc_din++);
        u->y2[i] = *(hw_enc_din++);
    }
    for (i = 0; i<DEPTH; i++)
    {
        u->x1_tail[i] = *(hw_enc_din++);
        u->y1_tail[i] = *(hw_enc_din++);
    }
    for (i = 0; i<DEPTH; i++)
    {
        u->x2_tail[i] = *(hw_enc_din++);
        u->y2_tail[i] = *(hw_enc_din++);
    }
}
#endif

int main()
{
 	tb = (tb_mem*)mallocp(sizeof(tb_mem), 15);

	int i;
	il = (long*)mallocp(BLOCKLENGTH * sizeof(long), 3);

	u = alloc_turbo_tbmem(tasks);
	u->tmem = alloc_turbo_memory(BLOCKLENGTH, tasks);

#ifdef DEMOBOARD
	volatile char *hw_prot = (char*)PROT_ADDR;

	tb->hw_error_number = (long*)ERROR_NUMBER;
	tb->hw_awgn_valid = (long*)AWGN_VALID_ADDR;
	tb->hw_snr = (long*)SNR_ADDR;
	tb->hw_finj = (unsigned long*)FINJ_ADDR;
	tb->hw_fail = (unsigned long*)FAIL_ADDR;
	volatile unsigned long *hw_cpc = (unsigned long*)CPC_ADDR;
#endif

	printf("setting hardware protection...\n\r");

	// set protection codes
#ifdef DEMOBOARD
	/*
	// simulation 1 : no error protection
	for (i = 0; i<=0xf; i++) *(hw_prot+0x00+i) = 15; // prom
	for (i = 0; i<=0xf; i++) *(hw_prot+0x10+i) = 15; // prom
	for (i = 0; i<=0xf; i++) *(hw_prot+0x20+i) = 14;
	for (i = 0; i<=0xf; i++) *(hw_prot+0x30+i) = 14;
	// 0x4------- SDRAM
	// *(hw_prot+0x4f) = 15; // debug support unit stack
	for (i = 0; i<=0xf; i++) *(hw_prot+0x50+i) = 14;
	for (i = 0; i<=0xf; i++) *(hw_prot+0x60+i) = 14;
	for (i = 0; i<=0xf; i++) *(hw_prot+0x70+i) = 14;
	for (i = 0; i<=0xf; i++) *(hw_prot+0x80+i) = 15; // apb
	for (i = 0; i<=0xf; i++) *(hw_prot+0x90+i) = 15; // debug support unit
	for (i = 0; i<=0xf; i++) *(hw_prot+0xA0+i) = 14;
	for (i = 0; i<=0xf; i++) *(hw_prot+0xB0+i) = 14;
	for (i = 0; i<=0x7; i++) *(hw_prot+0xC0+i) = 1; // turbo input
	for (i = 8; i<=0xa; i++) *(hw_prot+0xC0+i) = 1; // turbo output
	for (i =0xb;i<=0xf; i++) *(hw_prot+0xC0+i) = 15; // testbench
	for (i = 0; i<=0xf; i++) *(hw_prot+0xD0+i) = 14;
	for (i = 0; i<=0xf; i++) *(hw_prot+0xE0+i) = 14;
	for (i = 0; i<=0xf; i++) *(hw_prot+0xF0+i) = 14;
	*/

	// simulation 2 : error protection
	for (i = 0; i<=0xf; i++) *(hw_prot+0x00+i) = 15; // prom
	for (i = 0; i<=0xf; i++) *(hw_prot+0x10+i) = 15; // prom
	// 0x4------- SDRAM
	// *(hw_prot+0x4f) = 15; // debug support unit stack
	for (i = 0; i<=0xf; i++) *(hw_prot+0x80+i) = 15; // apb
	for (i = 0; i<=0xf; i++) *(hw_prot+0x90+i) = 15; // debug support unit
	for (i = 0; i<=0x7; i++) *(hw_prot+0xC0+i) = 2; // turbo input
	for (i = 8; i<=0xa; i++) *(hw_prot+0xC0+i) = 2; // turbo output

//	for (i = 0; i<=0x7; i++) *(hw_prot+0xC0+i) = 4; // turbo input
//	for (i = 8; i<=0xa; i++) *(hw_prot+0xC0+i) = 8; // turbo output

	for (i =0xb;i<=0xf; i++) *(hw_prot+0xC0+i) = 15; // testbench
#endif

	*hw_cpc = 0x00000006; // error led reset
	*hw_cpc = 0x00000011; // cpc finj on

	// loop over injected failure rate
	for (tb->ifr = IFR_START; tb->ifr >= IFR_STOP; tb->ifr--)
	{
#ifdef DEMOBOARD
		// set injected failure rate on hw
		if      (tb->ifr == 3) { *(tb->hw_finj) = 0x24DD2F1A; *(tb->hw_finj+1) = 0x106; printf("IFR: 1e-%d\n",tb->ifr);}
		else if (tb->ifr == 4) { *(tb->hw_finj) = 0x36E2EB1C; *(tb->hw_finj+1) = 0x1A; printf("IFR: 1e-%d\n", tb->ifr);}
		else if (tb->ifr == 5) { *(tb->hw_finj) = 0x9F16B11C; *(tb->hw_finj+1) = 0x2; printf("IFR: 1e-%d\n", tb->ifr);}
		else if (tb->ifr == 6) { *(tb->hw_finj) = 0x431BDE82; *(tb->hw_finj+1) = 0; printf("IFR: 1e-%d\n", tb->ifr);}
		else if (tb->ifr == 7) { *(tb->hw_finj) = 0x6B5FCA6; *(tb->hw_finj+1) = 0; printf("IFR: 1e-%d\n", tb->ifr);}
		else if (tb->ifr == 8) { *(tb->hw_finj) = 0xABCC77; *(tb->hw_finj+1) = 0; printf("IFR: 1e-%d\n", tb->ifr);}
		else                   { *(tb->hw_finj) = 0; *(tb->hw_finj+1) = 0; printf("IFR: 0\n");}
#endif

		// generate interleaver
#ifdef CONSOLE_DEBUG
		printf("generate interleaver...\n");
#endif
		generate_interleaver( BLOCKLENGTH, il );

		// loop over snr
		printf("Decoding... SNR: %d .. %d (%d)   Min.Blocks: %d   Min.Errors: %d\n\r", (int) SNR_FROM, (int) SNR_TO, (int) SNR_STEP, BLOCKS_MIN, MIN_FE);
		for (tb->snr = SNR_FROM; tb->snr < SNR_TO+ (SNR_STEP/2) ; tb->snr+=SNR_STEP)
		{
			// set snr
			tb->snr_calc = tb->snr + 10*log10(4);  //noise*2 on hw
			tb->snr_xilinx = ((long)tb->snr_calc)*16 + (long)(floor(tb->snr_calc*10+0.5))%10;
			tb->actual_snr_xilinx = (double)(tb->snr_xilinx/16) + (tb->snr_xilinx%16)/10.0;
			tb->actual_snr_xilinx -= 10*log10(4);  //noise*2 on hw
#ifdef DEMOBOARD
			*(tb->hw_snr) = tb->snr_xilinx;
			while (*tb->hw_awgn_valid == 0) for(tb->i = 0; tb->i<10; tb->i++) {} // wait for valid
#endif

			// loop over blocks
			tb->hisum = 0;
			tb->bit_errors = 0;
			tb->block_errors = 0;
			tb->message = BLOCKS_MIN/10;
			tb->blocks = BLOCKS_MIN;
			if (tb->message < 1) tb->message = 1;
			for (tb->block = 1; tb->block<=tb->blocks; tb->block++)
			{

#ifdef DEMOBOARD
				// read input data
				hw_enc_din = (long*)DIN_START_ADDR;
				for (i = 0; i<BLOCKLENGTH; i++) u->x      [i] = *(hw_enc_din++);
				for (i = 0; i<BLOCKLENGTH; i++) u->y1     [i] = *(hw_enc_din++);
				for (i = 0; i<DEPTH; i++)       u->y1_tail[i] = *(hw_enc_din++);
				for (i = 0; i<DEPTH; i++)		u->x1_tail[i] = *(hw_enc_din++);
				for (i = 0; i<BLOCKLENGTH; i++) u->y2     [i] = *(hw_enc_din++);
				for (i = 0; i<DEPTH; i++)		u->y2_tail[i] = *(hw_enc_din++);
				for (i = 0; i<DEPTH; i++)		u->x2_tail[i] = *(hw_enc_din++);
#else
				generate_input_data();
#endif

#ifdef CONSOLE_DEBUG
				printf("Starting to decode \n\r");
#endif
				// decode

				// we will never change the amount of tasks so this can be ommitted!
				/*if (u->tmem->tasks != tasks)
				{
					free_turbo_memory(u->tmem);
					u->tmem = alloc_turbo_memory(BLOCKLENGTH, tasks);
				}
				*/

				tb->hisum += turbo_decoder(u->tmem, u->x, u->y1, u->y2, u->x1_tail, u->x2_tail, u->y1_tail, u->y2_tail, il, u->llr);

#ifdef DEMOBOARD
				// write output data
				tb->hw_dout = (long*)DOUT_START_ADDR;
				for (tb->i = 0; tb->i<BLOCKLENGTH; tb->i++)
				{
					//u->llr[i] = u->x[i]; //test
					*(tb->hw_dout++) = u->llr[tb->i];
				}
				tb->errors = *(tb->hw_error_number);

				// bugfix (dirty hack): zweiter vergleich wenn 1 error
				if (tb->errors==2)
				{
					tb->hw_dout = (long*)DOUT_START_ADDR;
					for (tb->i = 0; tb->i<BLOCKLENGTH; tb->i++)
					{
						*(tb->hw_dout++) = u->llr[tb->i];
					}
					tb->errors = *(tb->hw_error_number);
				}
#else
				// compare
				tb->errors = 0;
				for (i = 0; i<BLOCKLENGTH; i++)
				{
					if ((u->llr[i]<0) != d[i]) tb->errors++;
				}
#endif

				if (tb->errors) tb->block_errors++;
				tb->bit_errors += tb->errors;

				if (tb->block%tb->message==0)
				{
					printf("SNR: %d (#%x)   Blocks: %d   BitE: %d (%d)  FrameE: %d (%d)   Avg.HI: %d\n\r", (int) tb->actual_snr_xilinx, (unsigned int)tb->snr_xilinx, tb->block, tb->bit_errors,(int) ( tb->bit_errors/(double)tb->block/BLOCKLENGTH), tb->block_errors, (int) (tb->block_errors/(double)tb->block), (int) (tb->hisum/tb->block));
					if (tb->block==tb->blocks && tb->block_errors < MIN_FE)
					{
						tb->est_fac = 2*MIN_FE/(tb->block_errors+1);
						if (tb->est_fac > 10) tb->blocks *= 10;
						else if (tb->est_fac < 2) tb->blocks *= 2;
						else tb->blocks *= tb->est_fac;
						tb->message = tb->block/10;
						if (tb->message < 1) tb->message = 1;
					}
				}
			} // loop over blocks
		} // loop over snr

#ifdef DEMOBOARD
		*(tb->hw_finj+1) = 0;
		*(tb->hw_finj) = 0;
#endif
	} // loop over ifr

	free_turbo_memory(u->tmem);
	return 0;
}

