///////////////////////////////////////////////////////
// Multicore Systems.
// Project 2 : 3D sugar scape.
// GNU General Public License
// Author:  Youngdae Kwon
// kydchonje@gmail.com
///////////////////////////////////////////////////////

#ifndef __LIB_SETUP_H__
#define __LIB_SETUP_H__

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>

struct setup
{
	int map_size;
	int total_people;
	int min_map_sugar;
	int max_map_sugar;
	int min_eating;
	int max_eating;
	int min_consume;
	int max_consume;
	int min_sugar;
	int max_sugar;
	int SEED_MAP_X;
	int SEED_MAP_Y;
	int SEED_MAP_Z;
	int SEED_MAP_SUGAR;
	int SEED_PEOPLE_X;
	int SEED_PEOPLE_Y;
	int SEED_PEOPLE_Z;
	int SEED_PEOPLE_EAT;
	int SEED_PEOPLE_CONSUME;
	int SEED_PEOPLE_SUGAR;
};

//////////////////////////////////////////////////

typedef struct Element {
	int x;
	int y;
	int z;
	int eat;
	int consume;
	int sugar;
} Element;

struct Node {
	struct Element people;
	struct Node* next;
};

typedef struct Map{
	int sugar;
	struct Node* front;
	struct Node* rear;
	struct Node node;
} Map;

typedef struct PeopleDirection{
	int idx_x;
	int idx_y;
	int idx_z;
} PeopleDirection;

typedef struct Stats
{
	int deadCount;
	int sugarCount;
} Stats;


#endif
