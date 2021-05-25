/*
 * interrupt.c -
 */
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>
#include <utils.h>

#include <sched.h>

#include <zeos_interrupt.h>

Gate idt[IDT_ENTRIES];
Register    idtR;
unsigned int shift_pulsat = 0;


char char_map[] =
{
  '\0','\0','1','2','3','4','5','6',
  '7','8','9','0','\'','¡','\0','\0',
  'q','w','e','r','t','y','u','i',
  'o','p','`','+','\0','\0','a','s',
  'd','f','g','h','j','k','l','ñ',
  '\0','º','\0','ç','z','x','c','v',
  'b','n','m',',','.','-','\0','*',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','7',
  '8','9','-','4','5','6','+','1',
  '2','3','0','\0','\0','\0','<','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0'
};

int zeos_ticks = 0;
int new_fps = 0;
int old_fps = 0;

void fps_routine(){

}

void clock_routine()
{
  //zeos_show_clock();
  zeos_ticks ++;

  if(zeos_ticks > 3){
  /* Prueba  */
  /*
   //De buffer->pagina de pantalla (color+caracter)
   char buffp[40] = "Hello I am a multicolor operating system";
   Word buf[40];
  
  for(int i = 0; i<40; ++i){ 
   Byte col = i%4+1; //Colores random excepto el negro
   char caracter = buffp[i]; 
    
   Word color_w = 0xFF00 & (col<<8);
   Word total = (Word) (caracter & 0x00FF) | color_w;
   
   buf[i] = total;
  }

   copy_data((void*)&buf, (void*)(current()->channel_table[current()->foco]->logicpage<<12), 80); //Copy data va por bytes al ser un word se ha de pasar el doble

  //De pagina de pantalla -> buffer
  Word bufft[40]; 
  copy_data((void*)(current()->channel_table[current()->foco]->logicpage<<12), (void*)&bufft, 80);

  int pos = 0;
  for(int i = 0; i < 40; i++){

     Byte col = bufft[i]>>8;
     char caracter = bufft[i];

     printc_xy(i, 0, caracter, col);
     ++pos;
  }
   */
  /* Prueba */

  /* Pintamos identificador de la pantalla y el proceso actual */
  
  char idProces[1];
  char idPantalla[1];

  itoa(current()->PID, idProces);
  itoa(current()->foco, idPantalla);
 
  int pos = 0;
  char buffer[8] = "Screen:";
  for(int i = 0; buffer[i]; ++i){
    printc_xy(pos,0, buffer[i],0x70);
    pos++;
  }
  printc_xy(pos,0, idPantalla[0],0x70);
  pos++;

  char buffer2[11] = " | Proces:";
  for(int i = 0; buffer2[i]; ++i){
    printc_xy(pos,0, buffer2[i],0x70);
    pos++;
  }
  printc_xy(pos,0, idProces[0],0x70);
  pos++;
 
  //Mostramos buffers 
  char buff_fps[8] = " | FPS:";
  char fps_c[2];
  itoa(old_fps,fps_c);
  for(int i = 0; buff_fps[i]; ++i){
    printc_xy(pos,0, buff_fps[i],0x70);
    pos++;
  }
  for(int i = 0; fps_c[i]; ++i){
    printc_xy(pos,0, fps_c[i],0x70);
    pos++;
  }
  if(old_fps > 9) pos++;
  
  //Obtenemos con zeos_ticks si ha llegado a 18 ticks = 1 segundo
  if(zeos_ticks%18 == 0 && zeos_ticks!=0){
   old_fps = new_fps;
   new_fps = 0;
  }

  for(int i = pos; i < 80-pos; i++){
    printc_xy(i,0,' ',0x02);
  }

   /* Prueba */
   /*
   //De buffer->pagina de pantalla (color+caracter)

   char buffp[500] = "Hello I am a multicolor operating system Hello I am a multicolor operating system Hello I am a multicolor operating system Hello I am a multicolor operating system Hello I am a multicolor operating system";
   Word buf[500];
  
  for(int i = 0; i<500; ++i){ 
   Byte col = i%4+1; //Colores random excepto el negro
   char caracter = buffp[i]; 
    
   Word color_w = 0xFF00 & (col<<8);
   Word total = (Word) (caracter & 0x00FF) | color_w;
   
   buf[i] = total;
  }
  copy_data((void*)&buf, (void*)(current()->channel_table[current()->foco]->logicpage<<12), 1000); //Copy data va por bytes al ser un word se ha de pasar el doble
  current()->channel_table[current()->foco]->content.bits.rwpointer = 0x005;
  
   */
   /* Prueba */

   /* Copiamos contenido extraido de la tabla de pantallas */
  //Coger contenido del foco actual y pasarlo a un buffer
  //Caracteres totales = filas_rwpointer * columnas_totales + columnas_rwpointer
  
  int columnas_rwpointer = (int)((current()->channel_table[current()->foco]->content.bits.rwpointer)>>5);
  int filas_rwpointer = (int)((current()->channel_table[current()->foco]->content.bits.rwpointer)&0x1F);
  
  int caracteres_totales = filas_rwpointer * 80 + columnas_rwpointer;
  Word total_c[caracteres_totales];

 copy_data((void*)((int)(current()->channel_table[current()->foco]->logicpage)<<12), total_c, caracteres_totales*2);

  //Pintamos contenido
  int y = 1;
  int salt = 0;
  for(int i = 0; i < 80*24; i++){
     Byte col = total_c[i]>>8;
     char caracter = total_c[i];
     if(i<caracteres_totales){
       col = total_c[i]>>8;
       caracter = total_c[i];
     }
     else{
       col = 0x02;
       caracter = ' ';
     }
     if(salt == 80){
        y++;
        salt = 0;
     }
     printc_xy(i%80, y%25, caracter, col);
     salt++;
  }
  new_fps++;
  
  }
  
  schedule();
  
}

void keyboard_routine()
{
  //0x0f <- tab
  //0x2a <- L_shift

  unsigned char c = inb(0x60);

  //Si se despulsa shift desmarcalo
  if(((c>>7) == 1) && (c&0x7f) == 0x2a){
  shift_pulsat = 0;
  }
  //Si la tecla pulsada es tab y shift ya ha sido pulsado, cambia foco
  else if ((c&0x80) && ((c&0x7f) == 0x0f) && shift_pulsat == 1){
  current()->foco = (((current()->foco)+1)%current()->screens);
  }
  //Si la tecla pulsada no es tab, comprueba que es shift
  else if (((c>>7) == 0) && (c&0x7f) == 0x2a){
  shift_pulsat = 1;
  }
  
}

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE INTERRUPTION GATE FLAGS:                          R1: pg. 5-11  */
  /* ***************************                                         */
  /* flags = x xx 0x110 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE TRAP GATE FLAGS:                                  R1: pg. 5-11  */
  /* ********************                                                */
  /* flags = x xx 0x111 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);

  //flags |= 0x8F00;    /* P = 1, D = 1, Type = 1111 (Trap Gate) */
  /* Changed to 0x8e00 to convert it to an 'interrupt gate' and so
     the system calls will be thread-safe. */
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void clock_handler();
void keyboard_handler();
void system_call_handler();

void setMSR(unsigned long msr_number, unsigned long high, unsigned long low);

void setSysenter()
{
  setMSR(0x174, 0, __KERNEL_CS);
  setMSR(0x175, 0, INITIAL_ESP);
  setMSR(0x176, 0, (unsigned long)system_call_handler);
}

void setIdt()
{
  /* Program interrups/exception service routines */
  idtR.base  = (DWord)idt;
  idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;
  
  set_handlers();

  /* ADD INITIALIZATION CODE FOR INTERRUPT VECTOR */
  setInterruptHandler(32, clock_handler, 0);
  setInterruptHandler(33, keyboard_handler, 0);

  setSysenter();

  set_idt_reg(&idtR);
}

