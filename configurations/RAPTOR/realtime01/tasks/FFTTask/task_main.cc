/*
	ORCOS - an Organic Reconfigurable Operating System
	Copyright (C) 2008 University of Paderborn

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <orcos.hh>


    /* fft */
#define fftsize          256 / 4
#define fftsize2         129 / 4

    /* FFT */
struct    complex { float rp, ip; } ;

    /* FFT */
struct complex z[fftsize+1], w[fftsize+1], e[fftsize2+1];

float   zr, zi;
int 	seed;

float Cos (float x)
/* computes cos of x (x in radians) by an expansion */
{
int i, factor;
float    result,power;

   result = 1.0; factor = 1;  power = x;
   for ( i = 2; i <= 10; i++ ) {
      factor = factor * i;  power = power*x;
      if ( (i & 1) == 0 )  {
        if ( (i & 3) == 0 ) result = result + power/factor;
        else result = result - power/factor;
      }
   }
   return (result);
}

int Min0(int arg1,int arg2)
    {
    if ( arg1 < arg2 )
        return (arg1);
    else
        return (arg2);
    }


void Uniform11(int iy, float yfl)
{
    iy = (4855*iy + 1731) & 8191;
    yfl = iy/8192.0;
} /* uniform */

void Exptab(int n,complex e[])
{ /* exptab */
    float theta, divisor, h[26];
    int i, j, k, l, m;

    theta = 3.1415926536;
    divisor = 4.0;
    for ( i=1; i <= 25; i++ )
        {
        h[i] = 1/(2*Cos( theta/divisor ));
        divisor = divisor + divisor;
        }

    m = n / 2 ;
    l = m / 2 ;
    j = 1 ;
    e[1].rp = 1.0 ;
    e[1].ip = 0.0;
    e[l+1].rp = 0.0;
    e[l+1].ip = 1.0 ;
    e[m+1].rp = -1.0 ;
    e[m+1].ip = 0.0 ;

    do {
        i = l / 2 ;
        k = i ;

        do {
            e[k+1].rp = h[j]*(e[k+i+1].rp+e[k-i+1].rp) ;
            e[k+1].ip = h[j]*(e[k+i+1].ip+e[k-i+1].ip) ;
            k = k+l ;
        } while ( k <= m );

        j = Min0( j+1, 25);
        l = i ;
    } while ( l > 1 );

} /* exptab */

void Fft(int n,complex z[],complex w[],complex e[],float sqrinv)
{
    int i, j, k, l, m, index;
    m = n / 2 ;
    l = 1 ;

    do {
        k = 0 ;
        j = l ;
        i = 1 ;

        do {

            do {
                w[i+k].rp = z[i].rp+z[m+i].rp ;
                w[i+k].ip = z[i].ip+z[m+i].ip ;
                w[i+j].rp = e[k+1].rp*(z[i].rp-z[i+m].rp)
                -e[k+1].ip*(z[i].ip-z[i+m].ip) ;
                w[i+j].ip = e[k+1].rp*(z[i].ip-z[i+m].ip)
                +e[k+1].ip*(z[i].rp-z[i+m].rp) ;
                i = i+1 ;
            } while ( i <= j );

            k = j ;
            j = k+l ;
        } while ( j <= m );

        /*z = w ;*/ index = 1;
        do {
            z[index] = w[index];
            index = index+1;
        } while ( index <= n );
        l = l+l ;
    } while ( l <= m );

    for ( i = 1; i <= n; i++ )
        {
        z[i].rp = sqrinv*z[i].rp ;
        z[i].ip = -sqrinv*z[i].ip;
        }

}


void final()
{
    final();
}

void* fftmain(int instance)
{


 //   if (instance > 10) final();

    int i;
    int cd_id = 0;

/*    Exptab(fftsize,e) ;
    seed = 5767 ;
    for ( i = 1; i <= fftsize; i++ )
        {
        Uniform11( seed, zr );
        Uniform11( seed, zi );
        z[i].rp = 20.0*zr - 10.0;
        z[i].ip = 20.0*zi - 10.0;
        }


    for ( i = 1; i <= 20; i++ ) {
       Fft(fftsize,z,w,e,0.0625) ;
    }
*/
    for (i =1; i < 0x1FFFFF; i++)
    {
        cd_id += 1;
    }

    return (void*) cd_id;
}

/*extern "C" int task_main(int instance)
{
  return (int) fftmain(0);
}*/



extern "C" int task_main(int instance)
{
unsigned int i;
static int cd_id = 0;

if (instance > 200) final();

fftmain(instance);

//if (instance == 1){
//	cd_id = fopen("dev/led0");
//}

//fputc(instance, cd_id);

/*int exectime = 166444;

thread_attr_t attr;
attr.deadline = 400000;
attr.period = 400000 ;
attr.executionTime = 166444;
attr.phase = 0;
attr.stack_size = 2048;

int id;

thread_create(&id,&attr,&fftmain,0);

attr.deadline = 200000;
attr.period = 400000;
attr.executionTime = 166444;
attr.phase = 166444 / 2;
attr.stack_size = 2048;

thread_create(&id,&attr,&fftmain,0);

thread_run(0);*/

//for (i =1; i < 0xFFFFF; i++)
//{
    //cd_id += 1;
//}

//return cd_id;*/
}


