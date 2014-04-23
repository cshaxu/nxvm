#include <stdio.h>
#include <unistd.h>
#include <termios.h>

int mygetch(void)
{
	struct termios old,new;
	char ch;

	tcgetattr(STDIN_FILENO,&old);

	new=old;
	new.c_lflag &=~(ICANON | ECHO);

	tcsetattr(STDIN_FILENO,TCSANOW,&new);
	ch=getchar();
	tcsetattr(STDIN_FILENO,TCSANOW,&old);

	return (ch);
}

int main()
{
	char a[20];
	int x=2;
	int i=0;
	do {
		a[i]=mygetch();
		if(127==a[i]) x=1;
		if(10==a[i]) break;
		switch(x) {
		case 1:
			printf("\b \b");
			i--;
			x=2;
			break;
		case 2:
			printf("*");
			i++;
			break;
		}
	} while(20!=i);
	a[i]='\0';
	printf("%s",a);
}