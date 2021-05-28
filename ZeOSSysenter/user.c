#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
 
   //TEST 0 Número de PID del proceso, ID de pantalla 0 y FPS deben de mostrarse

   // TEST 1 Multiples pantallas en un proceso + máximo de pantallas por proceso + funcionamiento de write + creación de pantallas + cambio de pantallas
   /* Para cambiar de pantalla pulsar Shift+Tab (Pulsar por separado u otra combinación no debe de funcionar)*/
   /*
   //Pantalla 0
   char buff[12] = "Canal num: ";
   char val_num[3];
   write(0,buff,strlen(buff));
   int num = 0;
   itoa(num, val_num);
   write(0,val_num,strlen(val_num));
   //Pantalla 1
   int fd = createScreen();
   write(fd,buff,strlen(buff));
   num = fd;
   itoa(num, val_num);
   write(fd,val_num,strlen(val_num));
   //Pantalla 2
   fd = createScreen();
   write(fd,buff,strlen(buff));
   num = fd;
   itoa(num, val_num);
   write(fd,val_num,strlen(val_num));
   //Pantalla 3
   fd = createScreen();
   write(fd,buff,strlen(buff));
   num = fd;
   itoa(num, val_num);
   write(fd,val_num,strlen(val_num));
   //Pantalla 4
   fd = createScreen();
   write(fd,buff,strlen(buff));
   num = fd;
   itoa(num, val_num);
   write(fd,val_num,strlen(val_num));
   //Pantalla 5
   fd = createScreen();
   write(fd,buff,strlen(buff));
   num = fd;
   itoa(num, val_num);
   write(fd,val_num,strlen(val_num));
   //Pantalla 6
   fd = createScreen();
   write(fd,buff,strlen(buff));
   num = fd;
   itoa(num, val_num);
   write(fd,val_num,strlen(val_num));
   //Pantalla 7
   fd = createScreen();
   write(fd,buff,strlen(buff));
   num = fd;
   itoa(num, val_num);
   write(fd,val_num,strlen(val_num));
   //Pantalla 8
   fd = createScreen();
   write(fd,buff,strlen(buff));
   num = fd;
   itoa(num, val_num);
   write(fd,val_num,strlen(val_num));
   //Pantalla 9
   fd = createScreen();
   write(fd,buff,strlen(buff));
   num = fd;
   itoa(num, val_num);
   write(fd,val_num,strlen(val_num));
   //Pantalla 10 (No debe de funcionar)
   fd = createScreen();
   write(fd,buff,strlen(buff));
   num = fd;
   itoa(num, val_num);
   write(fd,val_num,strlen(val_num));
   //Pantalla 11 (No debe de funcionar)
   fd = createScreen();
   write(fd,buff,strlen(buff));
   num = fd;
   itoa(num, val_num);
   write(fd,val_num,strlen(val_num));
   */
   
   //TEST 2 Comprobación de códigos de escape: color, puntero, borrar carácter
   /*
   [x;yf cursor
   x -> num de un digito o 2
   y -> num de un digito o 2
   [3xm Foreground
   x-> color 0..9
   [4xm Background
   x-> color 0..9
   DEL : \177
   */
   /*
	char *buff = "Muestra [47m[31mesto por\177 pan[30mtalla virtual"; 
        write(0,buff,strlen(buff)); 
	buff = "[1;0fEl cursor se ha [44mmovido\177\177 Y se h[32man b[2;0forrado caracteres";
	write(0,buff,strlen(buff));
   */
   

   //TEST 3 Cerrar pantalla
   /*
	//Pantalla 0
        char buff[12] = "Canal num: ";
        char val_num[3];
	write(0,buff,strlen(buff));
	int num = 0;
	itoa(num, val_num);
	write(0,val_num,strlen(val_num));
	//Pantalla 1
	int fd = createScreen();
	write(fd,buff,strlen(buff));
	num = fd;
	itoa(num, val_num);
	write(fd,val_num,strlen(val_num));
	//Pantalla 2
	fd = createScreen();
	write(fd,buff,strlen(buff));
	num = fd;
	itoa(num, val_num);
	write(fd,val_num,strlen(val_num));
	//Pantalla 3
	fd = createScreen();
	write(fd,buff,strlen(buff));
	num = fd;
	itoa(num, val_num);
	write(fd,val_num,strlen(val_num));
 
        close();
        close();
     */

   //TEST 4 Comprobación setFocus
   /*
	//Pantalla 0
        char buff[12] = "Canal num: ";
        char val_num[3];
	write(0,buff,strlen(buff));
	int num = 0;
	itoa(num, val_num);
	write(0,val_num,strlen(val_num));
	//Pantalla 1
	int fd = createScreen();
	write(fd,buff,strlen(buff));
	num = fd;
	itoa(num, val_num);
	write(fd,val_num,strlen(val_num));
	//Pantalla 2
	int foco = createScreen();
	write(foco,buff,strlen(buff));
	num = foco;
	itoa(num, val_num);
	write(foco,val_num,strlen(val_num));
	//Pantalla 3
	fd = createScreen();
	write(fd,buff,strlen(buff));
	num = fd;
	itoa(num, val_num);
	write(fd,val_num,strlen(val_num));
        
        setFocus(foco);
    */    
     

  //TEST 5 Comprobación fork() + Cambio de pantalla + Crear nueva pantalla
  /* Para cambiar de pantalla pulsar Shift+Tab (Pulsar por separado u otra combinación no debe de funcionar)*/
  /*
   char *buff;
   int fd = 0;
	switch(fork()) 
	{
		case 0:
			fd = createScreen();
			buff = "Muestra hijo"; 
 			write(fd,buff,strlen(buff)); 
			break;
		default:
			buff = "Muestra padre"; 
		        write(0,buff,strlen(buff)); 
	}
  */
  //TEST 7 Comprobación fork() + Compartición de terminal + Codigo de escape: color + puntero
  /*
   char *buff;
   int fd = 0;
	switch(fork()) 
	{
		case 0:
			buff = "[1;0fSoy el hijo y escribo"; 
 			write(fd,buff,strlen(buff)); 
			break;
		default:
			buff = "[32mSoy el padre y escribo"; 
		        write(0,buff,strlen(buff)); 
	}
  */

  //TEST 8 Comprobación funcionamiento múltiples procesos con fork()
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

  //TEST 9 Comprobación pantalla llena + máximos FPS 
  /* Para cambiar de pantalla pulsar Shift+Tab (Pulsar por separado u otra combinación no debe de funcionar), Comprobar en otra pantalla cuantos caracteres no se han mostrado por falta de espacio*/

  char buffer[80*24]= "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Integer tempus viverra leo ut mattis. Donec dictum mi enim. Nam sit amet bibendum tortor, vitae commodo ipsum. Nunc augue orci, vulputate pharetra ante sit amet, sodales scelerisque lacus. Praesent semper enim dui, non malesuada dui vulputate at. Etiam feugiat ligula vel felis euismod vehicula ut sed nisi. Maecenas tincidunt, massa eget aliquam dignissim, leo quam feugiat felis, quis ultrices justo tortor ac justo. Nulla dui nulla, posuere non faucibus non, convallis sit amet purus. Maecenas dapibus nisi vitae elit venenatis, vel sodales erat congue. Phasellus sollicitudin tortor sed efficitur lobortis. Fusce accumsan mattis dui. Aliquam eget pulvinar elit. Morbi non rutrum metus. Sed sollicitudin nunc non ullamcorper semper. Suspendisse sollicitudin ac odio sit amet tincidunt. Suspendisse justo eros, interdum id tellus vel, lacinia consequat nulla. Suspendisse congue tortor velit, ac elementum lectus iaculis ac. In hac habitasse platea dictumst. Nunc tempor est lacus, ut commodo risus mattis in. Fusce pretium purus ut odio sodales, ut pretium urna feugiat. Sed cursus mauris vel nisl viverra viverra. Ut a lacus semper, ullamcorper risus vitae, consequat ante. Pellentesque eget erat magna. Donec lacinia erat eros, ut mattis tortor dapibus ut. Donec tempor id magna finibus aliquam In magna felis, commodo sed lacinia nec, tincidunt ac augue. In eu erat imperdiet, iaculis urna commodo, fringilla elit. Donec auctor eget diam vitae molestie. Suspendisse luctus gravida mi et ullamcorper. Pellentesque vitae diam risus. Sed nec gravida mi. Fusce turpis libero, egestas nec enim sodales, molestie molestie nisl. Etiam tellus turpis, tincidunt nec eleifend maximus, maximus ut enim. Nullam auctor eget dui eu euismod. Fusce iaculis leo sed scelerisque sodales. Aenean quis lorem sed purus vulputate placerat sit amet sed felis. Sed in tellus eget urna accumsan hendrerit. Nullam porta tristique purus. Fusce et purus arcu. Pellentesque at neque ac ipsum pharetra tincidunt. Maecenas dapibus nisi vitae elit venenatis, vel sodales erat congue. Phasellus sollicitudin tortor sed efficitur lobortis. Fusce accumsan mattis dui. Aliquam eget pulvinar elit. Morbi non rutrum metus. Sed sollicitudin nunc non ullamcorper semper. Suspendisse sollicitudin ac odio sit amet tincidunt. Suspendisse justo eros, interdum id tellus vel, lacinia consequat nulla. Suspendisse congue tortor velit, ac elementum lectus iaculis ac. In hac habitasse platea dictumst. Nunc tempor est lacus, ut commodo risus mattis in. Fusce pretium purus ut odio sodales, ut pretium urna feugiat. Sed cursus mauris vel nisl viverra viverra. Ut a lacus semper, ullamcorper risus vitae, consequat ante. Pellentesque eget erat magna. Donec lacinia erat eros, ut mattis tortor dapibus ut. Donec tempor id magna finibus aliquam In magna felis, commodo sed lacinia nec, tincidunt ac augue. In eu erat imperdiet, iaculis urna commodo, fringilla elit. Donec auctor eget diam vitae molestie. Suspendisse luctus gravida mi et ullamcorper. Pellentesque vitae diam risus. Sed nec gravida mi. Fusce turpis libero, egestas nec enim sodales, molestie molestie nisl. Etiam tellus turpis, tincidunt nec eleifend maximus, maximus ut enim. Nullam auctor eget dui eu euismod. Fusce iaculis leo sed scelerisque sodales. Aenean quis lorem sed purus vulputate placerat sit amet sed felis. Sed in tellus eget urna accumsan hendrerit. Nullam porta tristique purus. Fusce et purus arcu. Pellentesque at neque ac ipsum pharetra tincidunt."; 
write(0,buffer,strlen(buffer));

  while(1) { }
}
