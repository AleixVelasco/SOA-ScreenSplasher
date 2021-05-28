#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

   // TEST 1 
   
	/*char *buff = "Muestra [47m[31mesto por\177 pan[30mtalla virtual"; 
  write(0,buff,strlen(buff)); 
	buff = "[1;0fHola k tal";
	write(0,buff,strlen(buff));*/
  
 //TEST 2 
 /*
	int fd = createScreen(); 
	char m[1];
	itoa(fd,m);
	write(fd,m,strlen(m));
	close();
 */
 /*
 char *buff = "Muestra esto por la segunda pantalla virtual"; 
int fd = createScreen(); 
 write(fd,buff,strlen(buff)); 
*/
 //TEST 3 //Pulsar Shift+Tab para cambiar de pantalla virtual 
 //TEST 4 
 //int f = setFocus(3); 
 // Error f = -1 */
	//char *buff;
//int fd;
//int ret = fork();
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

//TEST 6 Debugueando fork
/*
int fd = 0;
char *bufw = "Muestra hijo"; 
char *bufd = "Muestra padre"; 
int pid = fork();
if(pid == 0){
write(fd,bufw,strlen(bufw)); 
fork();
fork();
}
else{
int fd = createScreen(); 
write(fd,bufd,strlen(bufd));
fork();
fork();
}
*/

//TEST 7 Debugueando fork pa que se vea el mismo mensaje
/*
int fd = 0;
char *bufd = "Muestra mensaje"; 
write(fd,bufd,strlen(bufd));
fd = createScreen(); 
char *bufd2 = "Hola!";
write(fd,bufd2,strlen(bufd2));
fork();
*/

  //TEST 5 COmprobar que no crea más pantllas al superar el limite de pantallas en un proceso
  /*
  int fd = createScreen(); 
  fd = createScreen(); 
  fd = createScreen(); 
  fd = createScreen();
  fd = createScreen(); 
  fd = createScreen();
  fd = createScreen(); 
  fd = createScreen();
  fd = createScreen(); 
  fd = createScreen();
  fd = createScreen(); 
  fd = createScreen();
  fd = createScreen();
  */

 //TEST 6 Comprobar que la pantalla virtual se muestra entera
 /*char buff[80*24] = "Esto es una frase de prueba larga1 Esto es una frase de prueba larga2 Esto es una frase de prueba3 Esto es una frase de perueba larga4 ESto es una frase de peruba5 ESto es una frase de prueba6 Esto es una frase de prueba larga1 Esto es una frase de prueba larga2 Esto es una frase de prueba3 Esto es una frase de perueba larga4 ESto es una frase de peruba5 ESto es una frase de prueba6 Esto es una frase de prueba larga1 Esto es una frase de prueba larga2 Esto es una frase de prueba3 Esto es una frase de perueba larga4 ESto es una frase de peruba5 ESto es una frase de prueba6 Esto es una frase de prueba larga1 Esto es una frase de prueba larga2 Esto es una frase de prueba3 Esto es una frase de perueba larga4 ESto es una frase de peruba5 ESto es una frase de prueba6 Esto es una frase de prueba larga1 Esto es una frase de prueba larga2 Esto es una frase de prueba3 Esto es una frase de perueba larga4 ESto es una frase de peruba5 ESto es una frase de prueba6 Esto es una frase de prueba larga1 Esto es una frase de prueba larga2 Esto es una frase de prueba3 Esto es una frase de perueba larga4 ESto es una frase de peruba5 ESto es una frase de prueba6 fin una frase de prueba larga1 Esto es una frase de prueba larga2 Esto es una frase de prueba3 Esto es una frase de perueba larga4 ESto es una frase de peruba5 ESto es una frase de prueba6 Esto es una frase de prueba larga1 Esto es una frase de prueba larga2 Esto es una frase de prueba3 Esto es una frase de perueba larga4 ESto es una frase de peruba5 ESto es una frasedfghjkkkkkkkkk una frase de prueba larga2 Esto es una frase de prueba3 Esto es una frase de perueba larga4 ESto es una frase de peruba5 ESto es una frasedfghjkkkkkkkkk frase de prueba larga2 Esto es una frase de prueba3 Esto es una frase de perueba larga4 ESto es una frase de peruba5 ESto es una frasedfghjkkkkkkkkkESto es una frase de peruba5 ESto es una frasedfghjkkkkkkkkkfrasedfghjkkkkkkkkkftyguhy76tt  una frase de prueba3 Esto es una frase de perueba larga4 ESto es una frase de peruba5 ESto es una frase de prueba6 Esto es una frase de prueba larga1 Esto es una frase de prueba larga2 Esto es una frase de prueba3 Esto es una frase de perueba larga4 ESto es una frase de peruba5 ESto es una frase de prueba6 Esto es una frase de prueba larga1 Esto es una frase de prueba larga2 Esto es una frase de prueba3 Esto es una frase de perueba larga4 ESto es una frase de peruba5 ESto es una frase de prueba6 Esto es una frase de prueba larga1 Esto es una frase de prueba larga2 Esto es una frase de prueba3 Esto es una frase de perueba larga4 ESto es una frase de peruba5 ESto es una frase de prueba6 Esto es una frase de prueba larga1 Esto es una frase de prueba larga2 Esto es una frase de prueba3 Esto es una frase de perueba larga4 ESto es una frase de peruba5 ESto es una frase de prueba6 fin una frase de prueba larga1 Esto es una frase de prueba larga2 Esto es una frase de prueba3 Esto es una frase de perueba larga4 ESto es una frase de peruba5 ESto es una frase de prueba6 Esto es una frase de prueba larga1 Esto es una frase de prueba larga2 Esto es una frase de prueba3 Esto es una frase de perueba larga4 ESto es una frase de peruba5 ESto es una frasedfghjkkkkkkkkk una frase de prueba larga2 Esto es una frase de prueba3 Esto es una frase de perueba larga4 ESto es una frase de peruba5 ESto es una frasedfghjkkkkkkkkk frase de prueba larga2 Esto es una frase de prueba3 Esto es una frase de perueba larga4 ESto es una frase de peruba5 ESto es una frasedfghjkkkkkkkkkESto es una frase de peruba5 ESto es una frasedfghjkkkkkkkkkfrasedfghjkkkkkkkkkftyguhy76tt"; 
  write(0,buff,strlen(buff));
*/


  while(1) { }
}
