/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.

* File Name : test.cpp

* Purpose :

* Creation Date : 06-05-2013

* Last Modified : Mon 06 May 2013 10:53:33 PM CST

* Created By : Philip Zhang 

_._._._._._._._._._._._._._._._._._._._._.*/

#include <stdio.h>
#include <stdlib.h>
#include "TreeSkeleton.h"

CTreeSkeleton gb_treeskl;
double gb_max_angle = 10;

void instruct()
{
	printf("%s", "Operations:\n\
				 i:insert\n\
				 d:delete(0)\n\
				 D:delete(1)\n\
				 h:prev\n\
				 l:next\n\
				 j:ascend\n\
				 k:descent\n\
				 p:display\n\
				 L:load\n\
				 w:save\n\
				 s:simplify\n\
				 q:quit\n"); 
}

void processInsert()
{
	printf("%s\n", "Input the x, y, z, and r(using space to seperate items)");
	float x, y, z, r;
	scanf("%f %f %f %f", &x, &y, &z, &r);
	gb_treeskl.Insert(x, y, z, r);
	printf("%s\n", "insert done");
}

void processDelete(unsigned mode)
{
	gb_treeskl.Delete(mode);
	printf("%s\n", "delete done");
}

void processSelect(char c)
{
	switch(c)
	{
	case 'h':
		gb_treeskl.Previous();
		break;
	case 'l':
		gb_treeskl.Next();
		break;
	case 'j':
		gb_treeskl.Descent();
		break;
	case 'k':
		gb_treeskl.Ascent();
		break;
	default:
		break;
	}
}

void processLoad()
{
	printf("%s\n", "input the file name");
	char name[50];
	scanf("%s", name);
	gb_treeskl.Load(name);
	printf("%s\n", "load done");
}

void processSave()
{
	printf("%s\n", "input the file name");
	char name[50];
	scanf("%s", name);
	gb_treeskl.Save(name, 0);
	printf("%s\n", "save done");
}

void processSimplify()
{
	gb_treeskl.Simplify(gb_max_angle * 3.1415926 / 180.0);
	gb_max_angle *= 2;
	printf("%s\n", "simplify done");
}

int main()
{
	instruct();
	while(1)
	{
		char c;
		scanf("%c",	&c);
		switch(c)
		{
			case 'i':
			processInsert();	
			break;
			case 'd':
			processDelete(0);
			break;
			case 'D':
			processDelete(1);
			break;
			case 'h':
			case 'l':
			case 'j':
			case 'k':
			processSelect(c);
			break;
			case 'p':
			gb_treeskl.Display();
			break;
			case 'L':
			processLoad();
			break;
			case 'w':
			processSave();
			break;
			case 's':
			processSimplify();
			break;
			case 'q':
			exit(0);
			break;
		}
	}
	return 0;
}
