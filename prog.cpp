#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <stdio.h>

int	NowYear;		// 2020 - 2025
int	NowMonth;		// 0 - 11

float	NowPrecip;		// inches of rain per month
float	NowTemp;		// temperature this month
float	NowHeight;		// grain height in inches
int	NowNumDeer;		// number of deer in the current population

int NowLostDeer;

float ang;
float temp;
float precip;

float tempFactor;
float precipFactor;

int tornadoFactor;

unsigned int seed = 0;

const float GRAIN_GROWS_PER_MONTH =		9.0;
const float ONE_DEER_EATS_PER_MONTH =		1.0;

const float AVG_PRECIP_PER_MONTH =		7.0;	// average
const float AMP_PRECIP_PER_MONTH =		6.0;	// plus or minus
const float RANDOM_PRECIP =			2.0;	// plus or minus noise

const float AVG_TEMP =				60.0;	// average
const float AMP_TEMP =				20.0;	// plus or minus
const float RANDOM_TEMP =			10.0;	// plus or minus noise

const float MIDTEMP =				40.0;
const float MIDPRECIP =				10.0;


float SQR( float x );
float Ranf( unsigned int *seedp,  float low, float high );
int Ranf( unsigned int *seedp, int ilow, int ihigh );

void GrainDeer();
void Grain();
void Watcher();
void Tornado();

int
main()
{
	#ifndef _OPENMP
		fprintf( stderr, "No OpenMP support!\n" );
		return 1;
	#endif

		// starting date and time:
	NowMonth =    0;
	NowYear  = 2020;

	NowLostDeer = 0;

	// starting state (feel free to change this if you want):
	NowNumDeer = 10;
	NowHeight =  30.;

	ang = (  30.*(float)NowMonth + 15.  ) * ( M_PI / 180. );

	temp = AVG_TEMP - AMP_TEMP * cos( ang );
	NowTemp = temp + Ranf( &seed, -RANDOM_TEMP, RANDOM_TEMP );
	seed++;

	precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );
	NowPrecip = precip + Ranf( &seed,  -RANDOM_PRECIP, RANDOM_PRECIP );
	seed++;
	if( NowPrecip < 0. )
		NowPrecip = 0.;

	tempFactor = exp(-SQR(( NowTemp - MIDTEMP ) / 10.));
	precipFactor = exp(-SQR(( NowPrecip - MIDPRECIP ) / 10.));

	tornadoFactor = Ranf (&seed, 0, 9);
	seed++;

	omp_set_num_threads( 4 );	// same as # of sections

	#pragma omp parallel sections
	{
		#pragma omp section
		{
			GrainDeer( );
		}

		#pragma omp section
		{
			Grain( );
		}

		#pragma omp section
		{
			Watcher( );
		}

		#pragma omp section
		{
			Tornado( );	// your own
		}
	}       // implied barrier -- all functions must return in order
		// to allow any of them to get past here

	return 0;
}

float
SQR( float x )
{
	return x*x;
}

float
Ranf( unsigned int *seedp,  float low, float high )
{
        float r = (float) rand_r( seedp );              // 0 - RAND_MAX

        return(   low  +  r * ( high - low ) / (float)RAND_MAX   );
}


int
Ranf( unsigned int *seedp, int ilow, int ihigh )
{
        float low = (float)ilow;
        float high = (float)ihigh + 0.9999f;

        return (int)(  Ranf(seedp, low,high) );
}

void
GrainDeer()
{
	while (NowYear < 2026)
	{
		int NextNumDeer = NowNumDeer;

		if (NowNumDeer > NowHeight)
			NextNumDeer--;

		if (NowNumDeer < NowHeight)
			NextNumDeer++;

		if (NextNumDeer < 0)
			NextNumDeer = 0;

		if (tornadoFactor < 2)
			NextNumDeer /= 2;

		if (NowLostDeer > 1)
		{
			NextNumDeer += 2;
		}
		else if (NowLostDeer == 1)
		{
			NextNumDeer += 1;
		}

		#pragma omp barrier

		NowNumDeer = NextNumDeer;

		#pragma omp barrier

		#pragma omp barrier
	}
	return;
}

void
Grain()
{
	while (NowYear < 2026)
	{
		float NextHeight = NowHeight + tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH;
		NextHeight -= (float)NowNumDeer * ONE_DEER_EATS_PER_MONTH;

		if (NextHeight < 0.)
			NextHeight = 0.;

		#pragma omp barrier

		NowHeight = NextHeight;

		#pragma omp barrier

		#pragma omp barrier
	}
	return;
}

void
Watcher()
{
	while (NowYear < 2026)
	{
		#pragma omp barrier

		#pragma omp barrier

		float NowHeight_cm = NowHeight * 2.54;
		float NowTemp_C = (5./9.) * (NowTemp - 32);
		printf("%d \t %d \t %.2f \t %.2f \t %.2f \t %d \n",
					NowMonth, NowYear, NowTemp_C, NowPrecip, NowHeight_cm, NowNumDeer);

		if (NowMonth < 11)
			NowMonth++;
		else
		{
			NowYear++;
			NowMonth = 0;
		}

		ang = (  30.*(float)NowMonth + 15.  ) * ( M_PI / 180. );

		temp = AVG_TEMP - AMP_TEMP * cos( ang );
		NowTemp = temp + Ranf( &seed, -RANDOM_TEMP, RANDOM_TEMP );
		seed++;

		precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );
		NowPrecip = precip + Ranf( &seed,  -RANDOM_PRECIP, RANDOM_PRECIP );
		seed++;
		if( NowPrecip < 0. )
			NowPrecip = 0.;

		tempFactor = exp(-SQR(( NowTemp - MIDTEMP ) / 10.));
		precipFactor = exp(-SQR(( NowPrecip - MIDPRECIP ) / 10.));

		tornadoFactor = Ranf (&seed, 0, 9);
		seed++;

		#pragma omp barrier
	}
	return;
}

void
Tornado()
{
	while (NowYear < 2026)
	{

		int NextLostDeer = NowLostDeer;

		if (tornadoFactor < 2)            //tornado coming
		 	NextLostDeer = NowLostDeer + NowNumDeer / 2;

		if (NextLostDeer > 1)
		{
			NextLostDeer -= 2;
		}
		else if (NextLostDeer == 1)
		{
			NextLostDeer = 0;
		}

		#pragma omp barrier

		NowLostDeer = NextLostDeer;

		#pragma omp barrier

		#pragma omp barrier
	}
	return;
}
