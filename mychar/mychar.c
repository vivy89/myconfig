#include <stdlib.h>
#include <stdio.h>

char getch()
{
	char c;
	system("stty -echo");
	system("stty -icanon");
	c=getchar();
	system("stty icanon");
	system("stty echo");
	return c;
}
