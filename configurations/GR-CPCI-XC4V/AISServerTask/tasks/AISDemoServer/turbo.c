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
//
//              blocklength (info bits)   512
//              bitwidth input              6
//
// ===================================================================

#include "orcos.hh"
#include "turbo.h"
#include "map.h"
#include <math.h>
#include <stdlib.h>

extern int mysock;
//#define CONSOLE_DEBUG

// LTE Interleaver Generation

int get_index(int blockl)
{
	if (blockl <= 512)  return       (blockl-  40)/8;
	if (blockl <= 1024) return  59 + (blockl- 512)/16;
	if (blockl <= 2048) return  91 + (blockl-1024)/32;
	                    return 123 + (blockl-2048)/64;
}

int f1_list[] = {3,7,19,7,7,11,5,11,7,41,103,15,9,17,9,21,101,21,57,23,13,27,11,27,85,29,33,15,17,33,103,19,19,37,19,21,21,115,193,21,133,81,45,23,243,151,155,25,51,47,91,29,29,247,29,89,91,157,55,31,17,35,227,65,19,37,41,39,185,43,21,155,79,139,23,217,25,17,127,25,239,17,137,215,29,15,147,29,59,65,55,31,17,171,67,35,19,39,19,199,21,211,21,43,149,45,49,71,13,17,25,183,55,127,27,29,29,57,45,31,59,185,113,31,17,171,209,253,367,265,181,39,27,127,143,43,29,45,157,47,13,111,443,51,51,451,257,57,313,271,179,331,363,375,127,31,33,43,33,477,35,233,357,337,37,71,71,37,39,127,39,39,31,113,41,251,43,21,43,45,45,161,89,323,47,23,47,263};
int f2_list[] = {10,12,42,16,18,20,22,24,26,84,90,32,34,108,38,120,84,44,46,48,50,52,36,56,58,60,62,32,198,68,210,36,74,76,78,120,82,84,86,44,90,46,94,48,98,40,102,52,106,72,110,168,114,58,118,180,122,62,84,64,66,68,420,96,74,76,234,80,82,252,86,44,120,92,94,48,98,80,102,52,106,48,110,112,114,58,118,60,122,124,84,64,66,204,140,72,74,76,78,240,82,252,86,88,60,92,846,48,28,80,102,104,954,96,110,112,114,116,354,120,610,124,420,64,66,136,420,216,444,456,468,80,164,504,172,88,300,92,188,96,28,240,204,104,212,192,220,336,228,232,236,120,244,248,168,64,130,264,134,408,138,280,142,480,146,444,120,152,462,234,158,80,96,902,166,336,170,86,174,176,178,120,182,184,186,94,190,480};

void generate_interleaver(long blockl, long interleaver_pattern[])
{
	int i;

	int f1 = f1_list[get_index(blockl)];
	int f2 = f2_list[get_index(blockl)];

	for (i = 0; i < blockl; i++)
	{
		interleaver_pattern[i] = (f1*i + f2*i*i) % blockl;
	}
}


// Turbo Decoder

turbo_memory* alloc_turbo_memory(long blocklength, int tasks)
{
	int i;
	long ws;

	turbo_memory* mem = (turbo_memory*) malloc(sizeof(turbo_memory));
	mem->llr_il = (long*)mallocp(blocklength * sizeof(long), 8);
	mem->x_il = (long*)mallocp(blocklength * sizeof(long), 4);
	mem->extr_i = (long*)mallocp(blocklength * sizeof(long), 3);
	mem->extr_o = (long*)mallocp(blocklength * sizeof(long), 3);
	mem->blocklength = blocklength;
	mem->tasks = tasks;
	for (i = 0; i<tasks; i++)
	{
		ws = (blocklength+tasks-1)/tasks;
		if ((i+1)*ws > blocklength) ws = blocklength - i*ws;
		mem->map_mem[i] = alloc_map_memory(ws, tasks, i);
	}
	return mem;
}

void free_turbo_memory(turbo_memory* mem)
{
	int i;

	if (!mem)
		return;

	// free memory
	free(mem->llr_il);
	free(mem->x_il);
	free(mem->extr_i);
	free(mem->extr_o);

	for (i = 0; i<mem->tasks; i++) map_memory_free(mem->map_mem[i]);

	free(mem);
}

int turbo_decoder(turbo_memory* mem, long *x, long *y1, long *y2, long *x1_tail, long *x2_tail, long *y1_tail, long *y2_tail,
		long *il, long *llr)
{
	long* extr_i = mem->extr_i;
	long* extr_o = mem->extr_o;
	long* llr_il = mem->llr_il;
	long* x_il = mem->x_il;
	long blocklength = mem->blocklength;
	int tasks = mem->tasks;
	long i, hiter;
	long stop = 0;
	int rc;
	void *status;
	//pthread_t threads[MAX_NUM_THREADS];
	//pthread_attr_t attr;
	int pos_from;

#ifdef ITCTRL
	unsigned long llrsum = 0, llrsum_old = 0;
	unsigned long llrsumthr = blocklength*LLRSUMTHRF;
#endif

	// initialize
	for( i=0; i < blocklength; i++ )
	{
		extr_i[i] = 0;
		extr_o[i] = 0;
	}
	for( i=0; i < blocklength; i++ ) x_il[ i ] = x[ il[i] ];

//	pthread_attr_init(&attr);
//	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // create destination socket address
    sockaddr addr;

    // send message to clients to decodce the appropriate block specified in mad_td
	// well be decoding the last block! so only send request if this is not the last block!
    memcpy(&addr.name_data,"TurboDeco\0",10);   //< use this service


	unint4 acks = 0;

	// iterate
	for( hiter=1; hiter <= MAX_HALFITERATIONS && !stop; hiter++ )
	{
		if( hiter % 2 == 1 )
		{
			acks = 0;
            memcpy(&addr.name_data,"TurboDeco\0",10);   //< use this service

            // calculate MAP1
			//map_decoder( mem->map_mem, x, y1, x1_tail, y1_tail, extr_i, extr_o, llr );
			for ( i=0; i<tasks; i++)
			{

#ifdef CONSOLE_DEBUG
			    printf("Starting to decode block %d \n\r",i);
#endif

				pos_from = i*((blocklength+tasks-1)/tasks);
				map_td[i].mem = mem->map_mem[i];
				map_td[i].x = x + pos_from;
				map_td[i].y = y1 + pos_from;
				map_td[i].x_tail = x1_tail;
				map_td[i].y_tail = y1_tail;
				map_td[i].z = extr_i + pos_from;
				map_td[i].extr = extr_o + pos_from;
				map_td[i].llr = llr + pos_from;
				map_td[i].block_id = i;

				//printf("map_td @%x, x @%x y @%x \n\r",&map_td[i],map_td[i].x,map_td[i].y );

				//rc = pthread_create(&threads[i], &attr, map_decoder_t, (void *)&map_td[i]);


                if (i < tasks-1)
                {
    				acks = acks | (1 << i );

                    // tell client to start decoding
#ifdef CONSOLE_DEBUG
                    printf("Sending decode request number %d \n\r",i);
#endif
                    int ret = -1;

                    do {
                    	ret = sendto(mysock,(void *)&map_td[i],sizeof(map_td),&addr);

                    } while (ret < 0);
                }
                else
                {
#ifdef CONSOLE_DEBUG
                    printf("Decoding block %d \n\r",i);
#endif
                    map_decoder_t((void *)&map_td[i]);
                }


			}

			// wait for all blocks to be decoded!
			//for ( i=0; i<tasks; i++) pthread_join(threads[i], &status);
#ifdef CONSOLE_DEBUG
			printf("Waiting for decoder to finish 1\n\r");
#endif

			unint4 waited = 0;

            while (acks != 0)
            {

                if (waited > 300)
                {
                	waited = 0;
                	// resend those requests that have not been answered yet
                	for (int j = 0; j < tasks; j++)
                	{
                		if ( (acks  & (1 << j)) != 0)
                		{
                			printf("Resending Job %d \n\r",j);

                			memcpy(&addr.name_data,"TurboDeco\0",10);   //< use this service

                			int ret = -1;
                			// resend job j
                			 do {
                			     ret = sendto(mysock,(void *)&map_td[j],sizeof(map_td),&addr);
                			    } while (ret < 0);
                		}
                	}

                }

                char* msgptr;

                // simply wait for all other client signals
                int msglen = recv(mysock,&msgptr,MSG_PEEK);
                while (msglen > 0)
                {
                	int block_id = *((int*) msgptr);
                	//printf("Decoding block %d finished \n\r",block_id);
                	acks = acks & (~(1 << block_id ));
                	//printf("Job %d  finished\n\r",block_id);
                	msglen = recv(mysock,&msgptr,MSG_PEEK);
                }

                if (acks != 0)
                {
                	//printf("Waiting 100 ms \n\r");
                	sleep(100); // peek every 10 ms
                	waited += 100;
                }
             }

#ifdef CONSOLE_DEBUG
            printf("Decoding finished \n\r");
#endif
			// interleave extrinsic information
#ifdef ITCTRL
			llrsum = 0;
#endif
			for( i=0; i < blocklength; i++)
			{
				extr_i[ i ] = extr_o[ il[i] ];
#ifdef ITCTRL
				llrsum += abs(llr[i]);
#endif
			}

			// iteration control
#ifdef ITCTRL
			if (llrsum <= llrsum_old || llrsum > llrsumthr) stop = 1;
			llrsum_old = llrsum;
#endif
		}
		else  // !( hiter % 2 == 1 )
		{

			acks = 0;
			memcpy(&addr.name_data,"TurboDeco\0",10);   //< use this service

			// calculate MAP2
			//map_decoder( mem->map_mem, x_il, y2, x2_tail, y2_tail, extr_i, extr_o, llr_il );
			for ( i=0; i<tasks; i++)
			{
#ifdef CONSOLE_DEBUG
			    printf("Starting to decode block %d \n\r",i);
#endif

				pos_from = i*((blocklength+tasks-1)/tasks);
				map_td[i].mem = mem->map_mem[i];
				map_td[i].x = x_il + pos_from;
				map_td[i].y = y2 + pos_from;
				map_td[i].x_tail = x2_tail;
				map_td[i].y_tail = y2_tail;
				map_td[i].z = extr_i + pos_from;
				map_td[i].extr = extr_o + pos_from;
				map_td[i].llr = llr_il + pos_from;
				map_td[i].block_id = i;

			    // send message to clients to decodce the appropriate block specified in mad_td
				//rc = pthread_create(&threads[i], &attr, map_decoder_t, (void *)&map_td[i]);
				//printf("map_td @%x, x @%x y @%x \n\r",&map_td[i],map_td[i].x,map_td[i].y );

				// well be decoding the last block! so only send request if this is not the last block!
				if (i < tasks-1)
				{
					acks = acks | (1 << i );

#ifdef CONSOLE_DEBUG
                    printf("Sending decode request number %d \n\r",i);
#endif

                    // tell client to start decoding
  				   int ret = -1;

					do {
						ret = sendto(mysock,(void *)&map_td[i],sizeof(map_td),&addr);
					} while (ret < 0);
				}
				else
				{
#ifdef CONSOLE_DEBUG
                    printf("Decoding block %d \n\r",i);
#endif
					map_decoder_t((void *)&map_td[i]);
				}

			}



			// wait for all blocks to be decoded!
			//for ( i=0; i<tasks; i++) pthread_join(threads[i], &status);
#ifdef CONSOLE_DEBUG
			printf("Waiting for decoder to finish 2 acks=%d\n\r",acks);
#endif
			unint4 waited = 0;
			 while (acks != 0)
			{

				if (waited > 300)
				{
					waited = 0;
					// resend those requests that have not been answered yet
					for (int j = 0; j < tasks; j++)
					{
						if ( (acks  & (1 << j)) != 0)
						{
							printf("Resending Job %d \n\r",j);

							memcpy(&addr.name_data,"TurboDeco\0",10);   //< use this service

							int ret = -1;
							// resend job j
							 do {
								 ret = sendto(mysock,(void *)&map_td[j],sizeof(map_td),&addr);
								} while (ret < 0);
						}
					}

				}

				char* msgptr;

				// simply wait for all other client signals
				int msglen = recv(mysock,&msgptr,MSG_PEEK);
				while (msglen > 0)
				{
					int block_id = *((int*) msgptr);
					//printf("Decoding block %d finished \n\r",block_id);
					acks = acks & (~(1 << block_id ));
					//printf("Job %d  finished\n\r",block_id);
					msglen = recv(mysock,&msgptr,MSG_PEEK);
				}

				if (acks != 0)
				{
					//printf("Waiting 100 ms \n\r");
					sleep(100); // peek every 10 ms
					waited += 100;
				}

			 }

#ifdef CONSOLE_DEBUG
            printf("Decoding finished \n\r");
#endif

			// de-interleave extrinsic information
#ifdef ITCTRL
			llrsum = 0;
#endif
			for( i=0; i < blocklength; i++)
			{
				extr_i[ il[i] ] = extr_o[i];
#ifdef ITCTRL
				llrsum += abs(llr_il[i]);
#endif
			}

			// iteration control
#ifdef ITCTRL
			if (llrsum <= llrsum_old || llrsum > llrsumthr) stop = 1;
			llrsum_old = llrsum;
#endif

			// de-interleave LLRs
			if( hiter == MAX_HALFITERATIONS || stop)
			{
				for( i=0; i < blocklength; i++)
				{
					llr[ il[i] ] = llr_il[i];
				}
			}
		} // hiter % 2 == 1
		//printf("%ld\n",llrsum);
	} // for hiter
	return hiter-1;
}


