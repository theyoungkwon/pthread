///////////////////////////////////////////////////////
// Multicore Systems.
// Project 2 : 3D sugar scape.
// GNU General Public License
// Author:  Youngdae Kwon
// kydchonje@gmail.com
///////////////////////////////////////////////////////

#ifndef __LIB_QUEUE_H__
#define __LIB_QUEUE_H__

#include "setup.h"

void Enqueue(Element* pElement);
void Dequeue(void);
Element Front(void);
void Print(void);

void mapEnqueue(Map* pMap, Element* pElement);
void mapDequeue(Map* pMap);
Element mapFront(Map* pMap);
void mapPrint(Map* pMap);

#endif
