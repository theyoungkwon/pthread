#ifndef __LIB_LCGRAND_H
#define __LIB_LCGRAND_H

/* The following 3 declarations are for use of the random-number generator
   lcgrand and the associated functions lcgrandst and lcgrandgt for seed
   management.  This file (named lcgrand.h) should be included in any program
   using these functions by executing
       #include "lcgrand.h"
   before referencing the functions. */

#include "setup.h"

float lcgrand64bit(int stream);
float lcgrand(int stream);
void  lcgrandst(long zset, int stream);
long  lcgrandgt(int stream);

int   uniform(int lower, int upper, int seed);
int   uniform64bit(int lower, int upper, int seed);
int   uniform32bit(int lower, int upper, int seed);
Element uniform_people(
	int lower, int upper,
	int min_eating, int max_eating,
	int min_consume, int max_consume,
	int min_sugar, int max_sugar,
	int seed_peo_x, int seed_peo_y, int seed_peo_z,
	int seed_peo_eat, int seed_peo_consume, int seed_peo_sugar
	);
PeopleDirection uniform_map(
	int lower, int upper,
	int seed_map_x, int seed_map_y,int seed_map_z
	);

#endif
