///////////////////////////////////////////////////////
// Multicore Systems.
// Project 2 : 3D sugar scape.
// GNU General Public License
// Author:  Youngdae Kwon
// kydchonje@gmail.com
///////////////////////////////////////////////////////

#include "sugar.h"

struct setup s;
int total_loop;
int simulationCount;
/////////////////////////////////////////////
Map *MapArray;
Stats DeadSugarCount;
Stats* DeadSugarCount2d;
int prev_deadCount;
int prev_sugarCount;
int deadCountPeopleStartIdx;
int thread_deadCountPeopleStartIdx;
int prev_deadCountPeopleStartIdx;
int deadCountPeopleArrSize;
Element* deadCountPeopleArr;
int sugarCountStartIdx;
int prev_sugarCountStartIdx;
int thread_sugarCountStartIdx;
int sugarCountSize;
int* sugarCountArr_x;
int* sugarCountArr_y;
int* sugarCountArr_z;

int* peoplePointArr;
int numPeoplePointArr;
Element* peopleArr2d;
int* numPeopleArr2d;
int peopleArr2dSize;
Element* peopleArr;
int numPeopleArr;
int map_size_1;
int map_size_2;
int map_size_3;

/* global variables for pthread */
int num_thread;
int num_thread_action;
int num_thread_create_people_sugar;
// thread mutex lock variables
	// handler ID=0  createPeople
	// handler ID=1  createSugar_idx_x
	// handler ID=2  createSugar_idx_y
	// handler ID=3  createSugar_idx_z
	// handler ID>=4 other actions
pthread_t* thread_handlers;
pthread_mutex_t* pThread_lock_map;

// static function declartion
static void end();
static void initialize(void);
static void parameter_setup(struct setup *s);
static void people_generator(struct setup *s);
static void map_generator(struct setup *s);
static double mysecond(void);
////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	/* variables for main function */
	double t_total;
	// pthread_setup
	if( argc > 1){
		num_thread = atoi(argv[1]);
		if( (num_thread >= MIN_NUM_THREAD) && (num_thread <= MAX_NUM_THREAD) ){
			printf(" (Excluding Main Thread) NUM_THREAD = %d \n", num_thread);
		}else{
			printf(" (Excluding Main Thread) MAX_NUM_THREAD = %d \n", MAX_NUM_THREAD);
			printf(" (Excluding Main Thread) MIN_NUM_THREAD = %d \n", MIN_NUM_THREAD);
			return 0;
		}
	}else{
		num_thread = DEFAULT_NUM_THREAD;
		printf(" (Excluding Main Thread) DEFAULT_NUM_THREAD = %d \n", DEFAULT_NUM_THREAD);
	}
	num_thread_action = num_thread - MIN_NUM_THREAD;
	/*system setup_start*/
	initialize();
	// load map and people_gen data
	loadMapAndPeople();
	/*system setup_end*/
	// thread_setup_initialize
	thread_init();
	t_total = mysecond();
	while(simulationCount++< total_loop)
	{
		peopleMove();

		peopleMergeAndEat();

		createPeople();

		createSugar();
	}
	t_total = mysecond() - t_total;
	end();
	thread_free();
	fprintf(stderr, "\n\n=============== OUTPUT DISPLAY ===============\n");
	fprintf(stderr, "number of threads    %d\n", num_thread);
	fprintf(stderr, "t_total              %lf\n", t_total);
	return 0;
}

/////////////////////////////////////////////////////////////////
/////////////////////// I N I T /////////////////////////////////
void loadMapAndPeople(){
	printf("\n START: Initialize \n");
	register int i;
	int j,k, num;
	char trash[100];
	int x, y, z, eat, consume, sugar;
	map_size_1 = s.map_size;
	map_size_2 = map_size_1*map_size_1;
	map_size_3 = map_size_2*map_size_1;
	Element people;
	// initialize DeadSugarCount
	DeadSugarCount.deadCount = 0;
	DeadSugarCount.sugarCount = 0;
	// initialize DeadSugarCount
	// initialize long long long 1D Global Array : MapArray
	MapArray = malloc(sizeof(Map)*map_size_3);
	peoplePointArr = malloc(sizeof(int)*s.total_people);
	numPeoplePointArr = 0;

	peopleArr = malloc(sizeof(Element)*s.total_people);

	deadCountPeopleStartIdx = 0;
	deadCountPeopleArrSize = s.total_people*THREAD_CREATE_PEOPLE_SIZE_FACTOR;
	deadCountPeopleArr = malloc(sizeof(Element)*s.total_people*THREAD_CREATE_PEOPLE_SIZE_FACTOR);
	sugarCountStartIdx = 0;
	sugarCountSize = s.total_people*THREAD_CREATE_SUGAR_SIZE_FACTOR;
	sugarCountArr_x = malloc(sizeof(int)*s.total_people*THREAD_CREATE_SUGAR_SIZE_FACTOR);
	sugarCountArr_y = malloc(sizeof(int)*s.total_people*THREAD_CREATE_SUGAR_SIZE_FACTOR);
	sugarCountArr_z = malloc(sizeof(int)*s.total_people*THREAD_CREATE_SUGAR_SIZE_FACTOR);

	// load Map data
	FILE* rfp = fopen("./sugar_map.txt", "r");
	fscanf(rfp, "%s %s %d", trash, trash, &num);
	for(i = 0; i<map_size_1; i++)
	{
		for(j = 0; j<map_size_1; j++)
		{
			for(k = 0; k<map_size_1; k++)
			{
				fscanf(rfp, "%d", &num);
				MapArray[i*map_size_2 + j*map_size_1 + k].sugar = num;
				MapArray[i*map_size_2 + j*map_size_1 + k].front = NULL;
				MapArray[i*map_size_2 + j*map_size_1 + k].rear = NULL;
			}
		}
	}
	fclose(rfp);

	// load People gen data
	rfp = fopen("./gen_people_list.txt", "r");
	fscanf(rfp, "%s %s %d", trash, trash, &num);
	fscanf(rfp, "%s %s %s %s %s %s %s", trash, trash, trash, trash, trash, trash, trash);

	for(i=0; i<s.total_people; i++)
	{
		fscanf(rfp, "%d %d %d %d %d %d %d", &num, &x, &y, &z, &eat, &consume, &sugar);
		people.x = x;
		people.y = y;
		people.z = z;
		people.eat = eat;
		people.consume = consume;
		people.sugar = sugar;
		peopleArr[i] = people;
		numPeopleArr += 1;
	}
	simulationCount = 0;
}

void thread_init(){
	// initialize thread handlers
	register int i;
	long thread_total_people, thread_total_sugar;
	long total_people_long = (long) s.total_people;
	thread_handlers = malloc(sizeof(pthread_t)*num_thread);
	thread_deadCountPeopleStartIdx = 0;
	thread_sugarCountStartIdx = 0;
	num_thread_create_people_sugar = 0;

	// init data structures for thread
	if(num_thread_action > 0){
		peopleArr2dSize = s.total_people*2/num_thread_action;
		peopleArr2d = malloc(sizeof(Element)*s.total_people*2);
		numPeopleArr2d = malloc(sizeof(int)*num_thread_action);
		DeadSugarCount2d = malloc(sizeof(Stats)*(num_thread_action));
		for(i=0; i<num_thread_action; i++){
			DeadSugarCount2d[i].deadCount = 0;
			DeadSugarCount2d[i].sugarCount = 0;
		}
	}
	// init locks
	pThread_lock_map = malloc(sizeof(pthread_mutex_t)*map_size_3);
	for(i=0; i<map_size_3; i++){
		pthread_mutex_init( (pThread_lock_map+i), NULL);
	}
	thread_total_people = (long)s.total_people*THREAD_CREATE_PEOPLE_SIZE_FACTOR;
	thread_total_sugar = (long)s.total_people*THREAD_CREATE_SUGAR_SIZE_FACTOR;
	pthread_create(&thread_handlers[0], NULL, thread_create_people, (void*) thread_total_people);
	pthread_create(&thread_handlers[1], NULL, thread_create_sugar_x, (void*) thread_total_sugar);
	pthread_create(&thread_handlers[2], NULL, thread_create_sugar_y, (void*) thread_total_sugar);
	pthread_create(&thread_handlers[3], NULL, thread_create_sugar_z, (void*) thread_total_sugar);
	usleep(SLEEP_ONE_SECOND*total_people_long*5/666664);
	printf("\n FISISH : Initialize \n");
}

void thread_free(){
	int i;
	free(thread_handlers);
	for(i=0; i<map_size_3; i++){
		pthread_mutex_destroy( (pThread_lock_map+i) );
	}
	free(pThread_lock_map);

	if(num_thread_action > 0){
		int i;
		free(peopleArr2d);
		free(numPeopleArr2d);
		free(DeadSugarCount2d);
	}
}
/////////////////////// I N I T /////////////////////////////////
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/////////////////////// M O V E /////////////////////////////////
void peopleMove(){
	// Load Balancing Scheme.
	if(num_thread_action > 0){
		long thread_iter;
		for( thread_iter=0; thread_iter<num_thread_action; thread_iter++){
			pthread_create(&thread_handlers[MIN_NUM_THREAD+thread_iter], NULL, thread_people_move, (void*) thread_iter);
		}
		for( thread_iter=0; thread_iter<num_thread_action; thread_iter++){
			pthread_join(thread_handlers[MIN_NUM_THREAD+thread_iter], NULL);
		}
	}else{
		int directionFlag, max_sugar, x, y, z;
		register int i;
		register int tempMapSugar;
		for(i=0; i<s.total_people; i++){
			directionFlag = 0;
			max_sugar = 0;
			x = peopleArr[i].x;
			y = peopleArr[i].y;
			z = peopleArr[i].z;
			// check +x, +y, +z direction
			if( x+1 < map_size_1){
				tempMapSugar = MapArray[(x+1)*map_size_2 + y*map_size_1 + z].sugar;
				if (tempMapSugar > max_sugar){
					max_sugar = tempMapSugar;
					directionFlag = 1;
				}
			}
			if( y+1 < map_size_1){
				tempMapSugar = MapArray[x*map_size_2 + (y+1)*map_size_1 + z].sugar;
				if (tempMapSugar > max_sugar){
					max_sugar = tempMapSugar;
					directionFlag = 2;
				}
			}
			if( z+1 < map_size_1){
				tempMapSugar = MapArray[x*map_size_2 + y*map_size_1 + (z+1)].sugar;
				if (tempMapSugar > max_sugar){
					max_sugar = tempMapSugar;
					directionFlag = 3;
				}
			}
			// check -x, -y, -z direction
			if( x-1 >= 0){
				tempMapSugar = MapArray[(x-1)*map_size_2 + y*map_size_1 + z].sugar;
				if (tempMapSugar > max_sugar){
					max_sugar = tempMapSugar;
					directionFlag = 4;
				}
			}
			if( y-1 >= 0){
				tempMapSugar = MapArray[x*map_size_2 + (y-1)*map_size_1 + z].sugar;
				if (tempMapSugar > max_sugar){
					max_sugar = tempMapSugar;
					directionFlag = 5;
				}
			}
			if( z-1 >= 0){
				tempMapSugar = MapArray[x*map_size_2 + y*map_size_1 + (z-1)].sugar;
				if (tempMapSugar > max_sugar){
					max_sugar = tempMapSugar;
					directionFlag = 6;
				}
			}
			// 1 : x+  2: y+  3: z+  4:x-  5:y-  6:z-
			switch (directionFlag){
				case 1:
					peopleArr[i].x = x+1;
					break;
				case 2:
					peopleArr[i].y = y+1;
					break;
				case 3:
					peopleArr[i].z = z+1;
					break;
				case 4:
					peopleArr[i].x = x-1;
					break;
				case 5:
					peopleArr[i].y = y-1;
					break;
				case 6:
					peopleArr[i].z = z-1;
					break;
				default:
					break;
			}
		}
	}
}

void* thread_people_move (void* rank){
	long my_rank = (long) rank;
	int my_n = s.total_people/num_thread_action;
	int start_from = my_n*my_rank;
	int end_to;
	// last action_thread is doing remaining jobs
	if( my_rank == (num_thread_action-1) ) {
		end_to = s.total_people;
	}else{
		end_to = start_from + my_n;
	}

	int directionFlag, max_sugar, x, y, z;
	register int i;
	register int tempMapSugar;
	for(i=start_from; i<end_to; i++){
		directionFlag = 0;
		max_sugar = 0;
		x = peopleArr[i].x;
		y = peopleArr[i].y;
		z = peopleArr[i].z;
		// check +x, +y, +z direction
		if( x+1 < map_size_1){
			tempMapSugar = MapArray[(x+1)*map_size_2 + y*map_size_1 + z].sugar;
			if (tempMapSugar > max_sugar){
				max_sugar = tempMapSugar;
				directionFlag = 1;
			}
		}
		if( y+1 < map_size_1){
			tempMapSugar = MapArray[x*map_size_2 + (y+1)*map_size_1 + z].sugar;
			if (tempMapSugar > max_sugar){
				max_sugar = tempMapSugar;
				directionFlag = 2;
			}
		}
		if( z+1 < map_size_1){
			tempMapSugar = MapArray[x*map_size_2 + y*map_size_1 + (z+1)].sugar;
			if (tempMapSugar > max_sugar){
				max_sugar = tempMapSugar;
				directionFlag = 3;
			}
		}
		// check -x, -y, -z direction
		if( x-1 >= 0){
			tempMapSugar = MapArray[(x-1)*map_size_2 + y*map_size_1 + z].sugar;
			if (tempMapSugar > max_sugar){
				max_sugar = tempMapSugar;
				directionFlag = 4;
			}
		}
		if( y-1 >= 0){
			tempMapSugar = MapArray[x*map_size_2 + (y-1)*map_size_1 + z].sugar;
			if (tempMapSugar > max_sugar){
				max_sugar = tempMapSugar;
				directionFlag = 5;
			}
		}
		if( z-1 >= 0){
			tempMapSugar = MapArray[x*map_size_2 + y*map_size_1 + (z-1)].sugar;
			if (tempMapSugar > max_sugar){
				max_sugar = tempMapSugar;
				directionFlag = 6;
			}
		}
		// 1 : x+  2: y+  3: z+  4:x-  5:y-  6:z-
		switch (directionFlag){
			case 1:
				peopleArr[i].x = x+1;
				break;
			case 2:
				peopleArr[i].y = y+1;
				break;
			case 3:
				peopleArr[i].z = z+1;
				break;
			case 4:
				peopleArr[i].x = x-1;
				break;
			case 5:
				peopleArr[i].y = y-1;
				break;
			case 6:
				peopleArr[i].z = z-1;
				break;
			default:
				break;
		}
	}
	pthread_exit(0);
}
/////////////////////////// MOVE  /////////////////////////////////
///////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
//////////////////// MERGE AND EAT /////////////////////////////////
void peopleMergeAndEat(){
	// initialize bplus tree before merging starts
	int x, y, z;
	register int i;
	int temp_key = 0;
	numPeoplePointArr = 0;
	// insert people from peopleArr to MapArray
	for(i=0; i<s.total_people; i++){
		x = peopleArr[i].x;
		y = peopleArr[i].y;
		z = peopleArr[i].z;
		temp_key = x*map_size_2 + y*map_size_1 + z;
		if( MapArray[temp_key].front == NULL ){ // If this access is first to Map
			peoplePointArr[numPeoplePointArr] = temp_key;
			numPeoplePointArr += 1;
		}
		// always insert people to Map no matter what this access is first or not
		mapEnqueue(&MapArray[temp_key], &peopleArr[i]);
	}
	// Load Balancing Scheme.
	if(num_thread_action > 0){
		long thread_iter;
		for( thread_iter=0; thread_iter<num_thread_action; thread_iter++){
			pthread_create(&thread_handlers[MIN_NUM_THREAD+thread_iter], NULL, thread_people_merge_eat, (void*) thread_iter);
		}
		for( thread_iter=0; thread_iter<num_thread_action; thread_iter++){
			pthread_join(thread_handlers[MIN_NUM_THREAD+thread_iter], NULL);
		}
		// merge information from threads
		int j=0, idx=0;
		i = 0;
		for(idx=0; idx<num_thread_action; idx++){
			// merge peopleArr2d to peopleArr
			for(j=0; j<numPeopleArr2d[idx]; j++){
				peopleArr[i] = peopleArr2d[idx*peopleArr2dSize + j];
				i += 1;
			}
			// calculate DeadSugarCount from action threads
			DeadSugarCount.deadCount += DeadSugarCount2d[idx].deadCount;
			DeadSugarCount.sugarCount += DeadSugarCount2d[idx].sugarCount;
		}
		numPeopleArr = i;
	}else{
		// perform merge and eat by searching peoplePointArr
		int idx_node,total_eat,total_consume,total_sugar, iter, amountOfEatingSugar;
		struct Node* pTemp;
		Element people;
		numPeopleArr = 0;
		for(i=0; i<numPeoplePointArr; i++){
			temp_key = peoplePointArr[i];
			pTemp = MapArray[temp_key].front;
			idx_node = 0;total_eat=0;total_consume=0;total_sugar=0;
			people.x = pTemp->people.x;
			people.y = pTemp->people.y;
			people.z = pTemp->people.z;
			// check if queue is empty
			while(pTemp != NULL) {
				total_eat += pTemp->people.eat;
				total_consume += pTemp->people.consume;
				total_sugar += pTemp->people.sugar;
				idx_node += 1;
				pTemp = pTemp->next;
			}
			// erase old people in Queue
			for (iter = 0; iter < idx_node; ++iter)
			{
				mapDequeue(&MapArray[temp_key]);
			}
			// initialize new people
			people.eat = total_eat/idx_node;
			people.consume = total_consume/idx_node;
			people.sugar = total_sugar/idx_node;
			// update sugar count (amount of sugar deleted by merging)
			DeadSugarCount.sugarCount += (total_sugar - people.sugar);
			// update dead count & insert new people in Queue
			DeadSugarCount.deadCount += (idx_node-1);
			/////////////// MERGE //////////////////////////

			/////////////// EAT //////////////////////////
			if(MapArray[temp_key].sugar > people.eat){ amountOfEatingSugar = people.eat; }
			else{ amountOfEatingSugar = MapArray[temp_key].sugar; }
			people.sugar += amountOfEatingSugar;
			MapArray[temp_key].sugar -= amountOfEatingSugar;
			// update dead count & sugar count
			if( (people.sugar-people.consume) > 0){
				people.sugar -= people.consume;
				DeadSugarCount.sugarCount += people.consume;
				peopleArr[numPeopleArr] = people;
				numPeopleArr += 1;
			}
			else {
				DeadSugarCount.deadCount += 1;
				DeadSugarCount.sugarCount += people.sugar;
			}
			/////////////// EAT //////////////////////////
		}
	}
}

void* thread_people_merge_eat (void* rank){
	long my_rank = (long) rank;
	int my_n = numPeoplePointArr/num_thread_action;
	int start_from = my_n*my_rank;
	int end_to;
	// last action_thread is doing remaining jobs
	if( my_rank == (num_thread_action-1) ) {
		end_to = numPeoplePointArr;
	}else{
		end_to = start_from + my_n;
	}

	// perform merge and eat by searching peoplePointArr
	int idx_node,total_eat,total_consume,total_sugar, iter, amountOfEatingSugar;
	struct Node* pTemp;
	Element people;
	numPeopleArr = 0;
	int temp_key;
	int temp_unit = my_rank*peopleArr2dSize;
	int temp_adder=0;
	register int i;
	numPeopleArr2d[my_rank] = 0;
	DeadSugarCount2d[my_rank].sugarCount = 0;
	DeadSugarCount2d[my_rank].deadCount = 0;
	for(i=start_from; i<end_to; i++){
		temp_key = peoplePointArr[i];
		pTemp = MapArray[temp_key].front;
		idx_node = 0;total_eat=0;total_consume=0;total_sugar=0;
		people.x = pTemp->people.x;
		people.y = pTemp->people.y;
		people.z = pTemp->people.z;
		// check if queue is empty
		while(pTemp != NULL) {
			total_eat += pTemp->people.eat;
			total_consume += pTemp->people.consume;
			total_sugar += pTemp->people.sugar;
			idx_node += 1;
			pTemp = pTemp->next;
		}
		// erase old people in Queue
		for (iter = 0; iter < idx_node; ++iter)
		{
			mapDequeue(&MapArray[temp_key]);
		}
		// initialize new people
		people.eat = total_eat/idx_node;
		people.consume = total_consume/idx_node;
		people.sugar = total_sugar/idx_node;
		// update sugar count (amount of sugar deleted by merging)
		DeadSugarCount2d[my_rank].sugarCount += (total_sugar - people.sugar);
		// update dead count & insert new people in Queue
		DeadSugarCount2d[my_rank].deadCount += (idx_node-1);
		/////////////// MERGE //////////////////////////

		/////////////// EAT //////////////////////////
		if(MapArray[temp_key].sugar > people.eat){ amountOfEatingSugar = people.eat; }
		else{ amountOfEatingSugar = MapArray[temp_key].sugar; }
		people.sugar += amountOfEatingSugar;
		MapArray[temp_key].sugar -= amountOfEatingSugar;
		// update dead count & sugar count
		if( (people.sugar-people.consume) > 0){
			people.sugar -= people.consume;
			DeadSugarCount2d[my_rank].sugarCount += people.consume;
			peopleArr2d[temp_unit + temp_adder] = people;
			temp_adder += 1;
			numPeopleArr2d[my_rank] += 1;
		}
		else {
			DeadSugarCount2d[my_rank].deadCount += 1;
			DeadSugarCount2d[my_rank].sugarCount += people.sugar;
		}
		/////////////// EAT //////////////////////////
	}
	pthread_exit(0);
}
/////////////////////// MERGE AND EAT /////////////////////////////////
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
//////////////////////// CREATE PEOPLE AND SUGAR /////////////////////
void createPeople(){
	long thread_argu_people;
	switch(simulationCount){
		case 1:
			pthread_join(thread_handlers[THREAD_CREATE_PEOPLE_ID], NULL);
			prev_deadCountPeopleStartIdx = deadCountPeopleStartIdx;
			prev_deadCount = DeadSugarCount.deadCount;
		break;
		case 2:
			thread_argu_people = (long) prev_deadCount;
			prev_deadCount = DeadSugarCount.deadCount;
			thread_deadCountPeopleStartIdx = prev_deadCountPeopleStartIdx;
			prev_deadCountPeopleStartIdx = deadCountPeopleStartIdx;
			pthread_create(&thread_handlers[THREAD_CREATE_PEOPLE_ID], NULL, thread_create_people, (void*)thread_argu_people);
		break;
		default:
			thread_argu_people = (long) prev_deadCount;
			prev_deadCount = DeadSugarCount.deadCount;
			pthread_join(thread_handlers[THREAD_CREATE_PEOPLE_ID], NULL);
			thread_deadCountPeopleStartIdx = prev_deadCountPeopleStartIdx;
			prev_deadCountPeopleStartIdx = deadCountPeopleStartIdx;
			pthread_create(&thread_handlers[THREAD_CREATE_PEOPLE_ID], NULL, thread_create_people, (void*)thread_argu_people);
		break;
	}
	register int i;
	if( (num_thread_action>0)&&(DeadSugarCount.deadCount > num_thread_action) ){
		long thread_iter;
		num_thread_create_people_sugar = DeadSugarCount.deadCount;
		for(thread_iter=MIN_NUM_THREAD; thread_iter<num_thread; thread_iter++){
			pthread_create(&thread_handlers[thread_iter], NULL, thread_insert_people, (void*)thread_iter);
		}
		for(i=MIN_NUM_THREAD; i<num_thread; i++){
			pthread_join(thread_handlers[i], NULL);
		}
	}else{
		for(i=0; i<DeadSugarCount.deadCount ; i++)
		{
			if( (i+deadCountPeopleStartIdx) >= deadCountPeopleArrSize){
				peopleArr[numPeopleArr] = deadCountPeopleArr[(i+deadCountPeopleStartIdx)-deadCountPeopleArrSize];
			}else{
				peopleArr[numPeopleArr] = deadCountPeopleArr[(i+deadCountPeopleStartIdx)];
			}
			numPeopleArr += 1;
		}
	}
	if( (DeadSugarCount.deadCount+deadCountPeopleStartIdx) >= deadCountPeopleArrSize){
		deadCountPeopleStartIdx = (DeadSugarCount.deadCount+deadCountPeopleStartIdx)-deadCountPeopleArrSize;
	}else{
		deadCountPeopleStartIdx = (DeadSugarCount.deadCount+deadCountPeopleStartIdx);
	}
}

void createSugar(){
	// long thread_argu_people;
	long thread_argu_sugar, thread_iter;
	switch(simulationCount){
		case 1:
			pthread_join(thread_handlers[THREAD_CREATE_SUGAR_X_ID], NULL);
			pthread_join(thread_handlers[THREAD_CREATE_SUGAR_Y_ID], NULL);
			pthread_join(thread_handlers[THREAD_CREATE_SUGAR_Z_ID], NULL);
			prev_sugarCount = DeadSugarCount.sugarCount;
			prev_sugarCountStartIdx = sugarCountStartIdx;
		break;
		case 2:
			thread_argu_sugar = (long) prev_sugarCount;
			prev_sugarCount = DeadSugarCount.sugarCount;
			thread_sugarCountStartIdx = prev_sugarCountStartIdx;
			prev_sugarCountStartIdx = sugarCountStartIdx;
			pthread_create(&thread_handlers[THREAD_CREATE_SUGAR_X_ID], NULL, thread_create_sugar_x, (void*)thread_argu_sugar);
			pthread_create(&thread_handlers[THREAD_CREATE_SUGAR_Y_ID], NULL, thread_create_sugar_y, (void*)thread_argu_sugar);
			pthread_create(&thread_handlers[THREAD_CREATE_SUGAR_Z_ID], NULL, thread_create_sugar_z, (void*)thread_argu_sugar);
		break;
		default:
			pthread_join(thread_handlers[THREAD_CREATE_SUGAR_X_ID], NULL);
			pthread_join(thread_handlers[THREAD_CREATE_SUGAR_Y_ID], NULL);
			pthread_join(thread_handlers[THREAD_CREATE_SUGAR_Z_ID], NULL);
			thread_argu_sugar = (long) prev_sugarCount;
			prev_sugarCount = DeadSugarCount.sugarCount;
			thread_sugarCountStartIdx = prev_sugarCountStartIdx;
			prev_sugarCountStartIdx = sugarCountStartIdx;
			pthread_create(&thread_handlers[THREAD_CREATE_SUGAR_X_ID], NULL, thread_create_sugar_x, (void*)thread_argu_sugar);
			pthread_create(&thread_handlers[THREAD_CREATE_SUGAR_Y_ID], NULL, thread_create_sugar_y, (void*)thread_argu_sugar);
			pthread_create(&thread_handlers[THREAD_CREATE_SUGAR_Z_ID], NULL, thread_create_sugar_z, (void*)thread_argu_sugar);
		break;
	}

	register int i;
	int x, y, z;
	// when sugarCount is over limit
	if( DeadSugarCount.sugarCount > sugarCountSize){
		int temp;
		if( (num_thread_action>0)&&(DeadSugarCount.sugarCount > num_thread_action) ){
			num_thread_create_people_sugar = sugarCountSize;
			for(thread_iter=MIN_NUM_THREAD; thread_iter<num_thread; thread_iter++){
				pthread_create(&thread_handlers[thread_iter], NULL, thread_insert_sugar, (void*)thread_iter);
			}
			for(i=MIN_NUM_THREAD; i<num_thread; i++){
				pthread_join(thread_handlers[i], NULL);
			}
		}else{
			for(i=0; i<sugarCountSize ; i++)
			{
				if( (i+sugarCountStartIdx) >= sugarCountSize){
					x = sugarCountArr_x[(i+sugarCountStartIdx)-sugarCountSize];
					y = sugarCountArr_y[(i+sugarCountStartIdx)-sugarCountSize];
					z = sugarCountArr_z[(i+sugarCountStartIdx)-sugarCountSize];
					MapArray[x*map_size_2 + y*map_size_1 + z].sugar += 1;
				}else{
					x = sugarCountArr_x[(i+sugarCountStartIdx)];
					y = sugarCountArr_y[(i+sugarCountStartIdx)];
					z = sugarCountArr_z[(i+sugarCountStartIdx)];
					MapArray[x*map_size_2 + y*map_size_1 + z].sugar += 1;
				}
			}
		}
		if( (sugarCountSize+sugarCountStartIdx) >= sugarCountSize){
			sugarCountStartIdx = (sugarCountSize+sugarCountStartIdx)-sugarCountSize;
		}else{
			sugarCountStartIdx = (sugarCountSize+sugarCountStartIdx);
		}
		temp = DeadSugarCount.sugarCount - sugarCountSize;
		for(i=0; i<temp; i++)
		{
			x = uniform(0, map_size_1-1, s.SEED_MAP_X);
			y = uniform(0, map_size_1-1, s.SEED_MAP_Y);
			z = uniform(0, map_size_1-1, s.SEED_MAP_Z);
			MapArray[x*map_size_2 + y*map_size_1 + z].sugar += 1;
		}
		DeadSugarCount.sugarCount = sugarCountSize;
	}
	// when sugarCount is within capacity
	else{
		if( ( num_thread_action >0) &&(DeadSugarCount.sugarCount > num_thread_action) ){
			num_thread_create_people_sugar = DeadSugarCount.sugarCount;
			for(thread_iter=MIN_NUM_THREAD; thread_iter<num_thread; thread_iter++){
				pthread_create(&thread_handlers[thread_iter], NULL, thread_insert_sugar, (void*)thread_iter);
			}
			for(i=MIN_NUM_THREAD; i<num_thread; i++){
				pthread_join(thread_handlers[i], NULL);
			}
		}
		else{
			for(i=0; i<DeadSugarCount.sugarCount ; i++)
			{
				if( (i+sugarCountStartIdx) >= sugarCountSize){
					x = sugarCountArr_x[(i+sugarCountStartIdx)-sugarCountSize];
					y = sugarCountArr_y[(i+sugarCountStartIdx)-sugarCountSize];
					z = sugarCountArr_z[(i+sugarCountStartIdx)-sugarCountSize];
					MapArray[x*map_size_2 + y*map_size_1 + z].sugar += 1;
				}else{
					x = sugarCountArr_x[(i+sugarCountStartIdx)];
					y = sugarCountArr_y[(i+sugarCountStartIdx)];
					z = sugarCountArr_z[(i+sugarCountStartIdx)];
					MapArray[x*map_size_2 + y*map_size_1 + z].sugar += 1;
				}
			}
		}
		if( (DeadSugarCount.sugarCount+sugarCountStartIdx) >= sugarCountSize){
			sugarCountStartIdx = (DeadSugarCount.sugarCount+sugarCountStartIdx)-sugarCountSize;
		}else{
			sugarCountStartIdx = (DeadSugarCount.sugarCount+sugarCountStartIdx);
		}
	}
	if( simulationCount != total_loop){
		// make dead count zero
		DeadSugarCount.deadCount = 0;
		DeadSugarCount.sugarCount = 0;
	}
}

void* thread_create_people (void* rank){
	long numPeopleCreating = (long) rank;
	register Element people;
	register int i;
	for(i=0; i<numPeopleCreating ; i++)
	{
		people = uniform_people(
			0, map_size_1-1,
			s.min_eating, s.max_eating,
			s.min_consume, s.max_consume,
			s.min_sugar, s.max_sugar,
			s.SEED_PEOPLE_X, s.SEED_PEOPLE_Y, s.SEED_PEOPLE_Z,
			s.SEED_PEOPLE_EAT, s.SEED_PEOPLE_CONSUME, s.SEED_PEOPLE_SUGAR
			);
		if( (thread_deadCountPeopleStartIdx+i) >= deadCountPeopleArrSize ){
			deadCountPeopleArr[thread_deadCountPeopleStartIdx+i-deadCountPeopleArrSize] = people;
		}else{
			deadCountPeopleArr[thread_deadCountPeopleStartIdx+i] = people;
		}
	}
	pthread_exit(0);
}

void* thread_insert_people (void* rank){
	long my_rank = (long) rank;
	my_rank -= MIN_NUM_THREAD;
	int my_n = num_thread_create_people_sugar/(num_thread_action);
	int start_from = my_n*my_rank;
	int end_to;
	if( my_rank == (num_thread_action - 1) ) {
		end_to = num_thread_create_people_sugar;
	}else{
		end_to = start_from + my_n;
	}
	register int i;

	for(i=start_from; i<end_to ; i++)
	{
		if( (i+deadCountPeopleStartIdx) >= deadCountPeopleArrSize){
			peopleArr[numPeopleArr+i] = deadCountPeopleArr[(i+deadCountPeopleStartIdx)-deadCountPeopleArrSize];
		}else{
			peopleArr[numPeopleArr+i] = deadCountPeopleArr[(i+deadCountPeopleStartIdx)];
		}
	}
	pthread_exit(0);
}

void* thread_create_sugar_x (void* rank){
	long numSugarCreating = (long) rank;
	register int i;
	register int x;
	for(i=0; i<numSugarCreating ; i++)
	{
		x = uniform(0, map_size_1-1, s.SEED_MAP_X);
		if( (thread_sugarCountStartIdx+i) >= sugarCountSize ){
			sugarCountArr_x[(i+thread_sugarCountStartIdx)-sugarCountSize] = x;
		}else{
			sugarCountArr_x[(i+thread_sugarCountStartIdx)] = x;
		}
	}
	pthread_exit(0);
}

void* thread_create_sugar_y (void* rank){
	long numSugarCreating = (long) rank;
	register int i;
	register int y;
	for(i=0; i<numSugarCreating ; i++)
	{
		y = uniform(0, map_size_1-1, s.SEED_MAP_Y);
		if( (thread_sugarCountStartIdx+i) >= sugarCountSize ){
			sugarCountArr_y[(i+thread_sugarCountStartIdx)-sugarCountSize] = y;
		}else{
			sugarCountArr_y[(i+thread_sugarCountStartIdx)] = y;
		}
	}
	pthread_exit(0);
}

void* thread_create_sugar_z (void* rank){
	long numSugarCreating = (long) rank;
	register int i;
	register int z;
	for(i=0; i<numSugarCreating ; i++)
	{
		z = uniform(0, map_size_1-1, s.SEED_MAP_Z);
		if( (thread_sugarCountStartIdx+i) >= sugarCountSize ){
			sugarCountArr_z[(i+thread_sugarCountStartIdx)-sugarCountSize] = z;
		}else{
			sugarCountArr_z[(i+thread_sugarCountStartIdx)] = z;
		}
	}
	pthread_exit(0);
}

void* thread_insert_sugar (void* rank){
	long my_rank = (long) rank;
	my_rank -= MIN_NUM_THREAD;
	int my_n = num_thread_create_people_sugar/(num_thread - MIN_NUM_THREAD);
	int start_from = my_n*my_rank;
	int end_to, temp_key;
	int x, y, z;
	if( my_rank == (num_thread-1 - MIN_NUM_THREAD) ) {
		end_to = num_thread_create_people_sugar;
	}else{
		end_to = start_from + my_n;
	}
	register int i;

	for(i=start_from; i<end_to ; i++)
	{
		if( (i+sugarCountStartIdx) >= sugarCountSize){
			x = sugarCountArr_x[(i+sugarCountStartIdx)-sugarCountSize];
			y = sugarCountArr_y[(i+sugarCountStartIdx)-sugarCountSize];
			z = sugarCountArr_z[(i+sugarCountStartIdx)-sugarCountSize];
			temp_key = x*map_size_2 + y*map_size_1 + z;
			pthread_mutex_lock( (pThread_lock_map+temp_key) );
			MapArray[temp_key].sugar += 1;
			pthread_mutex_unlock( (pThread_lock_map+temp_key) );
		}else{
			x = sugarCountArr_x[(i+sugarCountStartIdx)];
			y = sugarCountArr_y[(i+sugarCountStartIdx)];
			z = sugarCountArr_z[(i+sugarCountStartIdx)];
			temp_key = x*map_size_2 + y*map_size_1 + z;
			pthread_mutex_lock( (pThread_lock_map+temp_key) );
			MapArray[temp_key].sugar += 1;
			pthread_mutex_unlock( (pThread_lock_map+temp_key) );
		}
	}
	pthread_exit(0);
}
/////////////////////// CREATE PEOPLE AND SUGAR /////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void printMapSugar(){
	int idx_x, idx_y, idx_z;
	printf("\n\nMap size: %d\n", s.map_size);
	for(idx_x = 0; idx_x<s.map_size; idx_x++)
	{
		for(idx_y = 0; idx_y<s.map_size; idx_y++)
		{
			for(idx_z = 0; idx_z<s.map_size; idx_z++)
			{
				printf("%d ", MapArray[idx_x*map_size_2 + idx_y*map_size_1 + idx_z].sugar);
			}
			printf("\n");
		}
		printf("\n");
	}
}

int cmpfunc_total(const void *a, const void *b)
{
	Element *pa = (Element *)a;
	Element *pb = (Element *)b;

	if (pa->x > pb->x)
		return 1;
	else if (pa->x < pb->x)
		return -1;
	else
	{
		if (pa->y > pb->y)
			return 1;
		else if (pa->y < pb->y)
			return -1;
		else
		{
			if (pa->z > pb->z)
				return 1;
			else if (pa->z < pb->z)
				return -1;
			else
			{
				if (pa->eat > pb->eat)
					return 1;
				else if (pa->eat < pb->eat)
					return -1;
				else
				{
					if (pa->consume > pb->consume)
						return 1;
					else if (pa->consume < pb->consume)
						return -1;
					else
					{
						if (pa->sugar > pb->sugar)
							return 1;
						else if (pa->sugar < pb->sugar)
							return -1;
						else
							return 0;
					}
				}
			}
		}
	}
}

void end()
{
	// free allocated variables
	pthread_join(thread_handlers[THREAD_CREATE_PEOPLE_ID], NULL);
	pthread_join(thread_handlers[THREAD_CREATE_SUGAR_X_ID], NULL);
	pthread_join(thread_handlers[THREAD_CREATE_SUGAR_Y_ID], NULL);
	pthread_join(thread_handlers[THREAD_CREATE_SUGAR_Z_ID], NULL);
	int idx_x, idx_y, idx_z;
	int i=0;
	printf("\n END : Write output_map_table \n");
	FILE* wfp = fopen("./output_map_table.txt", "w");
	fprintf(wfp, "Map size: %d\n", s.map_size);
	for(idx_x = 0; idx_x<s.map_size; idx_x++)
	{
		for(idx_y = 0; idx_y<s.map_size; idx_y++)
		{
			for(idx_z = 0; idx_z<s.map_size; idx_z++)
			{
				fprintf(wfp, "%d ", MapArray[idx_x*map_size_2 + idx_y*map_size_1 + idx_z].sugar);
			}
			fprintf(wfp, "\n");
		}
		fprintf(wfp, "\n");
	}
	fclose(wfp);

	wfp = fopen("./output_people_list.txt", "w");
	fprintf(wfp, "Total people: %d\n", s.total_people);
	fprintf(wfp, "Index\tx_axis\ty_axis\tz_axis\teating\tconsume\tsugar\n");

	// sort peopleArr
	printf("\n END : Write output_people_list \n");
	qsort(peopleArr, s.total_people, sizeof(Element), cmpfunc_total);

	// print sorted outputs
	for(i=0; i<s.total_people; i++){
		fprintf(wfp, "%d\t%d\t%d\t%d\t%d\t%d\t%d\n", i+1, peopleArr[i].x, peopleArr[i].y, peopleArr[i].z, peopleArr[i].eat, peopleArr[i].consume, peopleArr[i].sugar);
	}
	fclose(wfp);
	// free allocated variables
	free(peoplePointArr);
	free(deadCountPeopleArr);
	free(sugarCountArr_x);
	free(sugarCountArr_y);
	free(sugarCountArr_z);
	free(MapArray);
	free(peopleArr);
}

void initialize(void)
{
	parameter_setup(&s);	//parameter setting
	map_generator(&s);
	people_generator(&s);
}

void parameter_setup(struct setup *s)
{
	FILE* rfp = fopen("./test_setup.txt", "r");
	char trash[100];
	fscanf(rfp, "%s %d", trash, &total_loop);
	fscanf(rfp, "%s %d", trash, &s->map_size);
	fscanf(rfp, "%s %d", trash, &s->total_people);
	fscanf(rfp, "%s %d", trash, &s->min_map_sugar);
	fscanf(rfp, "%s %d", trash, &s->max_map_sugar);
	fscanf(rfp, "%s %d", trash, &s->min_eating);
	fscanf(rfp, "%s %d", trash, &s->max_eating);
	fscanf(rfp, "%s %d", trash, &s->min_consume);
	fscanf(rfp, "%s %d", trash, &s->max_consume);
	fscanf(rfp, "%s %d", trash, &s->min_sugar);
	fscanf(rfp, "%s %d", trash, &s->max_sugar);
	fscanf(rfp, "%s %d", trash, &s->SEED_MAP_X);
	fscanf(rfp, "%s %d", trash, &s->SEED_MAP_Y);
	fscanf(rfp, "%s %d", trash, &s->SEED_MAP_Z);
	fscanf(rfp, "%s %d", trash, &s->SEED_MAP_SUGAR);
	fscanf(rfp, "%s %d", trash, &s->SEED_PEOPLE_X);
	fscanf(rfp, "%s %d", trash, &s->SEED_PEOPLE_Y);
	fscanf(rfp, "%s %d", trash, &s->SEED_PEOPLE_Z);
	fscanf(rfp, "%s %d", trash, &s->SEED_PEOPLE_EAT);
	fscanf(rfp, "%s %d", trash, &s->SEED_PEOPLE_CONSUME);
	fscanf(rfp, "%s %d", trash, &s->SEED_PEOPLE_SUGAR);
	fclose(rfp);
}

static void people_generator(struct setup *s)
{
	int map_size = s->map_size;
	int total_people = s->total_people;
	int min_eating = s->min_eating;
	int max_eating = s->max_eating;
	int min_consume = s->min_consume;
	int max_consume = s->max_consume;
	int min_sugar = s->min_sugar;
	int max_sugar = s->max_sugar;
	int SEED_PEOPLE_X = s->SEED_PEOPLE_X;
	int SEED_PEOPLE_Y = s->SEED_PEOPLE_Y;
	int SEED_PEOPLE_Z = s->SEED_PEOPLE_Z;
	int SEED_PEOPLE_EAT = s->SEED_PEOPLE_EAT;
	int SEED_PEOPLE_CONSUME = s->SEED_PEOPLE_CONSUME;
	int SEED_PEOPLE_SUGAR = s->SEED_PEOPLE_SUGAR;
	int i;
	FILE* wfp = fopen("./gen_people_list.txt", "w");
	fprintf(wfp, "Total people: %d\n", total_people);
	fprintf(wfp, "Index\tx_axis\ty_axis\tz_axis\teating\tconsume\tsugar\n");
	for(i=1; i<=total_people; i++)
	{
		int x = uniform(0, map_size-1, SEED_PEOPLE_X);
		int y = uniform(0, map_size-1, SEED_PEOPLE_Y);
		int z = uniform(0, map_size-1, SEED_PEOPLE_Z);
		int eat = uniform(min_eating, max_eating, SEED_PEOPLE_EAT);
		int consume = uniform(min_consume, max_consume, SEED_PEOPLE_CONSUME);
		int sugar = uniform(min_sugar, max_sugar, SEED_PEOPLE_SUGAR);
		fprintf(wfp, "%d\t%d\t%d\t%d\t%d\t%d\t%d\n", i, x, y, z, eat, consume, sugar);
	}
	fclose(wfp);
}

static void map_generator(struct setup *s)
{
	int map_size = s->map_size;
	int min_map_sugar = s->min_map_sugar;
	int max_map_sugar = s->max_map_sugar;
	int SEED_MAP_SUGAR = s->SEED_MAP_SUGAR;
	int i, j, k;
	FILE* wfp = fopen("./sugar_map.txt", "w");
	fprintf(wfp, "Map size: %d\n", map_size);
	for(i = 0; i<map_size; i++)
	{
		for(j = 0; j<map_size; j++)
		{
			for(k = 0; k<map_size; k++)
			{
				fprintf(wfp, "%d ", (int)uniform(min_map_sugar, max_map_sugar, SEED_MAP_SUGAR));
			}
			fprintf(wfp, "\n");
		}
		fprintf(wfp, "\n");
	}
	fclose(wfp);
}

static double mysecond(void)
{
        struct timeval tp;
        struct timezone tzp;
        int i;

        i = gettimeofday(&tp,&tzp);
        return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}