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
// Author(s):   Worm, Kienle, Thul, May
//
// ===================================================================
//
// Project:     Turbo-Code Decoder for AIS
//
// ===================================================================



#include <stdlib.h>
#include <math.h>


void encode (long *input, long *output, long *ileaver, int blocklength)
{
  int k;
  /* These variables will function as the FF
     in the encoder's shift register.     */
  int enc1_FF_1, enc1_FF_2, enc1_FF_3,
      enc2_FF_1, enc2_FF_2, enc2_FF_3;

  /* these hold the values that will enter
     the shift registers at the next time step */
  int enc1_SR_in, enc2_SR_in;
  int address;

  int depth = 3;

  /* doin' the ordinary thing... */
  for (k = 0; k < blocklength; k++)
  {
    if (k == 0)
	{
	  enc1_FF_3 = enc1_FF_2 = enc1_FF_1 = 0;
	  enc2_FF_3 = enc2_FF_2 = enc2_FF_1 = 0;
	}

    /* modeling the feedback xor gates */
    enc1_SR_in = (input [k] + enc1_FF_2 + enc1_FF_3) % 2;
    enc2_SR_in = (input [ileaver[k]] + enc2_FF_2 + enc2_FF_3) % 2;

    /* modeling the output xor gates + BPSK */
    output [3*k] = 2*(input[k])-1;
	output [3*k+1] = 2*((enc1_SR_in + enc1_FF_1 + enc1_FF_3) % 2)-1;
	output [3*k+2] = 2*((enc2_SR_in + enc2_FF_1 + enc2_FF_3) % 2)-1;

    /* modeling the shift register */
    enc1_FF_3 = enc1_FF_2;
    enc1_FF_2 = enc1_FF_1;
    enc1_FF_1 = enc1_SR_in;

    enc2_FF_3 = enc2_FF_2;
    enc2_FF_2 = enc2_FF_1;
    enc2_FF_1 = enc2_SR_in;
  } /* for k */

  /* going for the termination */
  for (k = 0; k < depth; k++)
    {
      /* generate tail bits for encoder 1 and encoder 2

	 the feedback is now considered the systematic output
	 while the "real" feedback is set to 0 to drive the
	 encoders into the all-zero state
	 */

      address = 3*blocklength + 2*k;

      /* modeling the feedback xor gates */
      enc1_SR_in = enc2_SR_in = 0;

      /* modeling the output xor gates + BPSK */
      /* 1st Enc: */
      output[address]   = 2*((enc1_FF_3  + enc1_FF_2) % 2)-1;
      output[address+1] = 2*((enc1_SR_in + enc1_FF_1 + enc1_FF_3) % 2)-1;

      /* 2nd Enc: */
      output[address+6] = 2*((enc2_FF_3  + enc2_FF_2) % 2)-1;
      output[address+7] = 2*((enc2_SR_in + enc2_FF_1 + enc2_FF_3) % 2)-1;

      /* modeling the shift registers */
      enc1_FF_3 = enc1_FF_2;
      enc1_FF_2 = enc1_FF_1;
      enc1_FF_1 = enc1_SR_in;

      enc2_FF_3 = enc2_FF_2;
      enc2_FF_2 = enc2_FF_1;
      enc2_FF_1 = enc2_SR_in;
    }

  // make sure to have the right BPSK mapping
  for (k = 0; k < 3*blocklength+4*depth; k++) {
      output[k] = -output[k];
  }
}

