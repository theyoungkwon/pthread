///////////////////////////////////////////////////////
// Multicore Systems.
// Project 2 : 3D sugar scape.
// GNU General Public License
// Author:  Youngdae Kwon
// kydchonje@gmail.com
///////////////////////////////////////////////////////

#ifndef __LIB_SUGAR_H__
#define __LIB_SUGAR_H__

#define SLEEP_ONE_SECOND   1000000
#define MIN_NUM_THREAD	   4
#define DEFAULT_NUM_THREAD 8
#define MAX_NUM_THREAD 	   96
#define THREAD_CREATE_SUGAR_X_ID 0
#define THREAD_CREATE_SUGAR_Y_ID 1
#define THREAD_CREATE_SUGAR_Z_ID 2
#define THREAD_CREATE_PEOPLE_ID  3
#define THREAD_CREATE_PEOPLE_SIZE_FACTOR 3
#define THREAD_CREATE_SUGAR_SIZE_FACTOR 150

#include "./setup.h"
#include "./lcgrand.h"
#include "./queue.h"

////////////////////////////////////////////////////

/* sequential functions */
void loadMapAndPeople();
void peopleMove();
void peopleMergeAndEat();
void createPeople();
void createSugar();
void printMapSugar();

/* thread functions */
void thread_init();
void thread_free();
void* thread_people_move (void* rank);
void thread_main_people_move (int  rank);
void* thread_people_merge_eat (void* rank);
void thread_main_people_merge_eat (int rank);
void* thread_create_people (void* num_create);
void* thread_insert_people (void* rank);
void* thread_create_sugar_x (void* num_create);
void* thread_create_sugar_y (void* num_create);
void* thread_create_sugar_z (void* num_create);
void* thread_insert_sugar (void* rank);
// quick sort compare functions
int cmpfunc_total (const void * a, const void * b);

#endif
