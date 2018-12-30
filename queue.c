///////////////////////////////////////////////////////
// Multicore Systems.
// Project 2 : 3D sugar scape.
// GNU General Public License
// Author:  Youngdae Kwon
// kydchonje@gmail.com
///////////////////////////////////////////////////////
/*Queue - Linked List implementation*/
#include "./queue.h"

// Two glboal variables to store address of front and rear nodes.
struct Node* front = NULL;
struct Node* rear = NULL;

// To Enqueue an integer
void Enqueue(Element *pElement) {
	struct Node* pTemp =
		(struct Node*)malloc(sizeof(struct Node));
	pTemp->people.x = pElement->x;
	pTemp->people.y = pElement->y;
	pTemp->people.z = pElement->z;
	pTemp->people.eat = pElement->eat;
	pTemp->people.consume = pElement->consume;
	pTemp->people.sugar = pElement->sugar;
	// pTemp->people.isMoved = pElement->isMoved;
	pTemp->next = NULL;
	if(front == NULL && rear == NULL){
		front = rear = pTemp;
		return;
	}
	rear->next = pTemp;
	rear = pTemp;
}

// To Dequeue an integer.
void Dequeue() {
	struct Node* pTemp = front;
	if(front == NULL) {
		printf("Queue is Empty\n");
		return;
	}
	if(front == rear) {
		front = rear = NULL;
	}
	else {
		front = front->next;
	}
	free(pTemp);
}

Element Front() {
	if(front == NULL) {
		printf("Queue is empty\n");
		return;
	}
	return front->people;
}

void Print() {
	struct Node* pTemp = front;
	while(pTemp != NULL) {
		printf("x: %d, y: %d, z: %d \n",pTemp->people.x, pTemp->people.y, pTemp->people.z);
		printf("eat: %d, consume: %d, sugar: %d \n",pTemp->people.eat, pTemp->people.consume, pTemp->people.sugar);
		pTemp = pTemp->next;
	}
	printf("\n");
}

void mapEnqueue(Map *pMap, Element* pElement){
	struct Node* pTemp =
		(struct Node*)malloc(sizeof(struct Node));
	pTemp->people.x = pElement->x;
	pTemp->people.y = pElement->y;
	pTemp->people.z = pElement->z;
	pTemp->people.eat = pElement->eat;
	pTemp->people.consume = pElement->consume;
	pTemp->people.sugar = pElement->sugar;
	// pTemp->people.isMoved = pElement->isMoved;
	pTemp->next = NULL;
	if(pMap->front == NULL && pMap->rear == NULL){
		pMap->front = pMap->rear = pTemp;
		return;
	}
	pMap->rear->next = pTemp;
	pMap->rear = pTemp;
}

void mapDequeue(Map *pMap){
	struct Node* pTemp = pMap->front;
	if(pMap->front == NULL) {
		printf("Queue is Empty\n");
		return;
	}
	if(pMap->front == pMap->rear) {
		pMap->front = pMap->rear = NULL;
	}
	else {
		pMap->front = pMap->front->next;
	}
	free(pTemp);
}

Element mapFront(Map *pMap){
	if(pMap->front == NULL) {
		printf("Queue is empty\n");
		return;
	}
	return pMap->front->people;
}

void mapPrint(Map *pMap){
	struct Node* pTemp = pMap->front;
	while(pTemp != NULL) {
		printf("\n x: %d, y: %d, z: %d ",pTemp->people.x, pTemp->people.y, pTemp->people.z);
		// printf("\n eat: %d, consume: %d, sugar: %d isMoved: %d",pTemp->people.eat, pTemp->people.consume, pTemp->people.sugar, pTemp->people.isMoved);
		printf("\n eat: %d, consume: %d, sugar: %d ",pTemp->people.eat, pTemp->people.consume, pTemp->people.sugar);

		pTemp = pTemp->next;
	}
	printf("\n");
}