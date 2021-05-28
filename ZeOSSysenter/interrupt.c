/*
 * interrupt.c -
 */

#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>
#include <sched.h>
#include <utils.h>
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

#define rdtsc(low,high) \
        __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))

unsigned long long get_val(void) {
        unsigned long eax;
        unsigned long edx;
        unsigned long long ticks;

        rdtsc(eax,edx);

        ticks=((unsigned long long) edx << 32) + eax;

        return ticks;
}

int zeos_ticks = 0;
int new_fps = 0;
int old_fps = 0;

unsigned long long old_cicles = 0;
unsigned long long new_cicles = 0;

void clock_routine()
{
  //zeos_show_clock();
  zeos_ticks ++;
  unsigned long long val = get_val();

  if(zeos_ticks > 3){
  /* Pintamos identificador de la pantalla y el proceso actual */
  
  char idProces[5];
  char idPantalla[1];

  itoak(current()->PID, idProces);
  itoak(current()->foco, idPantalla);
  
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
  int ln = 2;
  if(current()->PID < 10) ln = 1;
  if(current()->PID >= 1000) ln = 4;
  for(int i = 0; i<ln; ++i){
    printc_xy(pos,0, idProces[i],0x70);
    pos++;
  }

  //Mostramos buffers 
 
  char buff_fps[8] = " | FPS:";
  for(int i = 0; buff_fps[i]; ++i){
    printc_xy(pos,0, buff_fps[i],0x70);
    pos++;
  }
  char fps_c[2];
  itoak(old_fps,fps_c);
  for(int i = 0; fps_c[i]; ++i){
    printc_xy(pos,0, fps_c[i],0x70);
    pos++;
  }
  if(old_fps > 9) pos++;

  new_cicles = val;
  //cicles = 219700
  if((new_cicles - old_cicles) >= 219700*18){
   old_fps = new_fps;
   new_fps = 0;
   old_cicles = new_cicles;
  }
  
  for(int i = pos; i < 80-pos; i++){
    printc_xy(i,0,' ',0x02);
  }
  
  int x = 0, y = 1;
  DWord screen = (DWord)(current()->channel_table[current()->foco]->logicpage<<12);
  for(DWord *i = (DWord*)screen; (int)i < screen + PAGE_SIZE; i++) {
	DWord m = *i;
	Byte bcolor1 = (Byte) (m>>8);
	Byte bchar1 = (Byte) m;
	Byte bcolor2 = (Byte) (m>>24);
	Byte bchar2 = (Byte) (m>>16);
        
	//if((y * 80 + x) == (int) current()->channel_table[current()->foco]->content.bits.rwpointer) {
	//	bcolor1 = (bcolor1 & 0x7F) | 0x80;
	//}
	printc_xy(x, y, bchar1, bcolor1);

	x++;
	if(x == 80 && y < 24){
		x = 0; 
		y++;
	}
	//if((y * 80 + x) == (int) current()->channel_table[current()->foco]->content.bits.rwpointer) {
	//	bcolor2 = (bcolor2 & 0x7F) | 0x80;
	//}

	printc_xy(x, y, bchar2, bcolor2);

	x++;
	if(x == 80 && y < 24){
		x = 0; 
		y++;
	}
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

