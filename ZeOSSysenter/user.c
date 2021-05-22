#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

   // TEST 1 
	/*char *buff = "Muestra [47m[31mesto por pan[30mtalla virtual"; 
  write(0,buff,strlen(buff)); */
 //TEST 2 
	/*int fd = createScreen(); 
	char m[1];
	itoa(fd,m);
	write(fd,m,strlen(m));*/
 /*char *buff = "Muestra esto por la segunda pantalla virtual"; 
 write(fd,buff,strlen(buff)); */

 //TEST 3 //Pulsar Shift+Tab para cambiar de pantalla virtual 
 //TEST 4 
 //int f = setFocus(3); 
 // Error f = -1 */
	char *buff;
int fd;
int ret = fork();
	/*switch(fork()) 
	{
		case 0:
			fd = createScreen();
			buff = "Muestra hijo"; 
 			write(fd,buff,strlen(buff)); 
			break;
		default:
			buff = "Muestra padre-hijo"; 
		  write(0,buff,strlen(buff)); 
	}*/

  while(1) { }
}
