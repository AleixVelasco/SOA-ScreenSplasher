/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <p_stats.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1

void * get_ebp();

int check_fd(int fd, int permissions)
{
  //if (fd!=1) return -EBADF; 
	if (fd<0 || fd>=current()->screens) return -EBADF;
  if (permissions!=ESCRIPTURA) return -EACCES; 
  return 0;
}

void user_to_system(void)
{
  update_stats(&(current()->p_stats.user_ticks), &(current()->p_stats.elapsed_total_ticks));
}

void system_to_user(void)
{
  update_stats(&(current()->p_stats.system_ticks), &(current()->p_stats.elapsed_total_ticks));
}

int sys_ni_syscall()
{
	return -ENOSYS; 
}

int sys_getpid()
{
	return current()->PID;
}

int global_PID=1000;

int ret_from_fork()
{
  return 0;
}

int sys_fork(void)
{
  struct list_head *lhcurrent = NULL;
  union task_union *uchild;
  
  /* Any free task_struct? */
  if (list_empty(&freequeue)) return -ENOMEM;

  lhcurrent=list_first(&freequeue);
  
  list_del(lhcurrent);
  
  uchild=(union task_union*)list_head_to_task_struct(lhcurrent);
  
  /* Copy the parent's task struct to child's */
  copy_data(current(), uchild, sizeof(union task_union));

	/* Inc the ref of the opened screens */
	for (int i=0; i<10; i++)
  {
    if(current()->channel_table[i] != NULL) {
			current()->channel_table[i]->content.bits.refs++;
		}
  }

  /* new pages dir */
  allocate_DIR((struct task_struct*)uchild);
  
  /* Allocate pages for DATA+STACK */
  int new_ph_pag, pag, i;
  page_table_entry *process_PT = get_PT(&uchild->task);
	
  for (pag=0; pag<NUM_PAG_DATA; pag++)
  {
    new_ph_pag=alloc_frame();
    if (new_ph_pag!=-1) /* One page allocated */
    {
      set_ss_pag(process_PT, PAG_LOG_INIT_DATA+pag, new_ph_pag);
    }
    else /* No more free pages left. Deallocate everything */
    {
      /* Deallocate allocated pages. Up to pag. */
      for (i=0; i<pag; i++)
      {
        free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
        del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
      }
      /* Deallocate task_struct */
      list_add_tail(lhcurrent, &freequeue);
      
      /* Return error */
      return -EAGAIN; 
    }
  }

  /* Copy parent's SYSTEM and CODE to child. */
  page_table_entry *parent_PT = get_PT(current());
  for (pag=0; pag<NUM_PAG_KERNEL; pag++)
  {
    set_ss_pag(process_PT, pag, get_frame(parent_PT, pag));
  }
  for (pag=0; pag<NUM_PAG_CODE; pag++)
  {
    set_ss_pag(process_PT, PAG_LOG_INIT_CODE+pag, get_frame(parent_PT, PAG_LOG_INIT_CODE+pag));
  }

  int screens = current()->screens+1;
  int PAG_LOG_INIT_SCREEN = PAG_LOG_INIT_DATA + NUM_PAG_DATA;

  /* Copy parent's DATA to child. We will use TOTAL_PAGES-1 as a temp logical page to map to */
  for (pag=NUM_PAG_KERNEL+NUM_PAG_CODE; pag<NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA; pag++)
  {
    /* Map one child page to parent's address space. */
    set_ss_pag(parent_PT, pag+NUM_PAG_DATA+screens, get_frame(process_PT, pag));
    copy_data((void*)(pag<<12), (void*)((pag+NUM_PAG_DATA+screens)<<12), PAGE_SIZE);
    del_ss_pag(parent_PT, pag+NUM_PAG_DATA+screens);
  }

 
  for (pag=0; pag<PAG_LOG_INIT_SCREEN+screens-1; pag++)
  {
    set_ss_screen_pag(process_PT, PAG_LOG_INIT_SCREEN + pag, get_frame(parent_PT, PAG_LOG_INIT_SCREEN + pag));
  }


  /* Deny access to the child's memory space */
  set_cr3(get_DIR(current()));

  uchild->task.PID=++global_PID;
  uchild->task.state=ST_READY;

  int register_ebp;		/* frame pointer */
  /* Map Parent's ebp to child's stack */
  register_ebp = (int) get_ebp();
  register_ebp=(register_ebp - (int)current()) + (int)(uchild);

  uchild->task.register_esp=register_ebp + sizeof(DWord);

  DWord temp_ebp=*(DWord*)register_ebp;
  /* Prepare child stack for context switch */
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=(DWord)&ret_from_fork;
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=temp_ebp;

  /* Set stats to 0 */
  init_stats(&(uchild->task.p_stats));

  /* Queue child process into readyqueue */
  uchild->task.state=ST_READY;
  list_add_tail(&(uchild->task.list), &readyqueue);
  return uchild->task.PID;
}


#define TAM_BUFFER 512

int sys_write(int fd, char *buffer, int nbytes) {
	char localbuffer [TAM_BUFFER];
	int bytes_left;
	int ret;
	int escapeCode = 0, changeColor = 0, changeCursor = 0, delete = 0;
	char code[5];
	int pos = 0;

 	int changeForegroundColor = -1, changeBackgroundColor = -1, changeCursorPos = -1;

	if ((ret = check_fd(fd, ESCRIPTURA)))
		return ret;
	if (nbytes < 0)
		return -EINVAL;
	if (!access_ok(VERIFY_READ, buffer, nbytes))
		return -EFAULT;
	

	Byte color = (Byte) current()->channel_table[fd]->content.bits.color;
	int positionCursor = (int) current()->channel_table[fd]->content.bits.rwpointer;

	DWord *screen = (DWord)(current()->channel_table[fd]->logicpage<<12);

	/*if((screen + (DWord) (positionCursor/2)) > (screen + PAGE_SIZE)){
		bytes_left = ((screen + (DWord) (positionCursor/2)) - (screen + PAGE_SIZE))*2;
	}*/
	//int currPos = (int) current()->channel_table[fd]->content.bits.rwpointer;

	DWord checkPos = (DWord) current()->channel_table[fd]->content.bits.rwpointer;
	for(int i = 0; i<nbytes; ++i){

		// Change Color
		if(changeColor == 1) {
			// Foreground coloring
			if(changeForegroundColor > -1) {
				color = (Byte) ((color & 0xF0) | (Byte) (0x0F & ((char)changeForegroundColor)));
				changeForegroundColor = -1;
			} else if(changeBackgroundColor > -1) {// Background coloring
				color = (Byte) (color & 0x0F) | (((char)changeBackgroundColor)<<4);
				changeBackgroundColor = -1;
			}
			current()->channel_table[fd]->content.bits.color = color;
			changeColor = 0;	
		}

		int currPos = (int) current()->channel_table[fd]->content.bits.rwpointer;
		if(buffer[i] == '['){//Escape Code
			if((i+1) < nbytes && buffer[i+1] == '3' && (i+3) < nbytes && buffer[i+3] == 'm') {// Change Foreground Color
				changeForegroundColor = (int) buffer[i+2];
				changeColor = 1;
				i += 3;
			}else if((i+1) < nbytes && buffer[i+1] == '4' && (i+3) < nbytes && buffer[i+3] == 'm') {// Change Background Color
				changeBackgroundColor = (int) buffer[i+2];
				changeColor = 1;
				i += 3;
			}
	
			if((i+2) < nbytes && buffer[i+2] == ';' && (i+4) < nbytes && buffer[i+4] == 'f'){
				int posx = (int) (buffer[i+1]-'0');
				int posy = (int) (buffer[i+3]-'0');
				if(posx >= 0 && posx < 25 && posy >=0 && posy < 80){
					int changeCursorPos = posx * 80 + posy;
					current()->channel_table[fd]->content.bits.rwpointer = changeCursorPos;
				}
				i += 4;
			}else if((i+2) < nbytes && buffer[i+2] == ';' && (i+5) < nbytes && buffer[i+5] == 'f') {
				int posx = (int) (buffer[i+1]-'0');
				int posy = ((int) (buffer[i+3]-'0') * 10) + ((int) buffer[i+4]);
				if(posx >= 0 && posx < 25 && posy >=0 && posy < 80){
					int changeCursorPos = posx * 80 + posy;
					current()->channel_table[fd]->content.bits.rwpointer = changeCursorPos;
				}
				i += 5;
			}else if((i+3) < nbytes && buffer[i+3] == ';' && (i+5) < nbytes && buffer[i+5] == 'f') {
				int posx = ((int) (buffer[i+1]-'0') * 10) + ((int) buffer[i+2]);
				int posy = (int) (buffer[i+4]-'0');
				if(posx >= 0 && posx < 25 && posy >=0 && posy < 80){
					int changeCursorPos = posx * 80 + posy;
					current()->channel_table[fd]->content.bits.rwpointer = changeCursorPos;
				}
				i += 5;
			}else if((i+3) < nbytes && buffer[i+3] == ';' && (i+6) < nbytes && buffer[i+6] == 'f') {
				int posx = ((int) (buffer[i+1]-'0') * 10) + ((int) buffer[i+2]);
				int posy = ((int) (buffer[i+4]-'0') * 10) + ((int) buffer[i+5]);
				if(posx >= 0 && posx < 25 && posy >=0 && posy < 80){
					int changeCursorPos = posx * 80 + posy;
					current()->channel_table[fd]->content.bits.rwpointer = changeCursorPos;
				}
				i += 6;
			}

		} else if(buffer[i] == '\177'){// Delete char

			Word w = ((Word) (color << 8)) | ((Word) ' ');
			if(currPos != 0)
				currPos =	currPos-1;
			if((int)currPos % 2 == 0) {
				currPos = (currPos/2);
				screen[currPos] = (DWord) (screen[currPos] & 0xFFFF0000) | (DWord) ((DWord) w  & 0x0000FFFF);
			}else if((int)currPos % 2 == 1){
				currPos = ((currPos-1)/2);
				screen[currPos] = (DWord) (screen[currPos] & 0x0000FFFF) | (DWord) ((DWord) (w<<16) & 0xFFFF0000);
			}
			current()->channel_table[fd]->content.bits.rwpointer -= 1;

		} else{
			
			Word w = ((Word) (color << 8)) | ((Word) buffer[i]);
			if((int)currPos % 2 == 0) {
				currPos = (currPos/2);
				screen[currPos] = (DWord) (screen[currPos] & 0xFFFF0000) | (DWord) ((DWord) w  & 0x0000FFFF);
			}else if((int)currPos % 2 == 1){
				currPos = ((currPos-1)/2);
				screen[currPos] = (DWord) (screen[currPos] & 0x0000FFFF) | (DWord) ((DWord) (w<<16) & 0xFFFF0000);
			}
			current()->channel_table[fd]->content.bits.rwpointer += 1;

		}
	
	}
	
	//return bytes_left;




	/*printc('\n',0x0F);
	printc('A',0x01);	printc('A',0x02);	printc('A',0x03);	printc('A',0x04);	printc('A',0x05);	printc('A',0x06);	printc('A',0x07);	printc('A',0x08);	printc('A',0x09);	printc('A',0x0A);	printc('A',0x0B);
	printc('A',0x0C);	printc('A',0x0D);	printc('A',0x0E);	printc('A',0x0F);
	printc('\n',0x0F);
	printc('A',0x10);	printc('A',0x20);	printc('A',0x30);	printc('A',0x40);	printc('A',0x50);	printc('A',0x60);	printc('A',0x70);	*/

	/*bytes_left = nbytes;
	while (bytes_left > TAM_BUFFER) {
		copy_from_user(buffer, localbuffer, TAM_BUFFER);
		ret = sys_write_console(localbuffer, TAM_BUFFER);
		bytes_left-=ret;
		buffer+=ret;
	}
	if (bytes_left > 0) {
		copy_from_user(buffer, localbuffer,bytes_left);
		ret = sys_write_console(localbuffer, bytes_left);
		bytes_left-=ret;
	}
	return (nbytes-bytes_left);*/
	return 0;
}


extern int zeos_ticks;

int sys_gettime()
{
  return zeos_ticks;
}

void sys_exit()
{  
  int i;

  page_table_entry *process_PT = get_PT(current());

  // Deallocate all the propietary physical pages
  for (i=0; i<NUM_PAG_DATA; i++)
  {
    free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
    del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
  }
  
  /* Free task_struct */
  list_add_tail(&(current()->list), &freequeue);
  
  current()->PID=-1;
  
  /* Restarts execution of the next process */
  sched_next_rr();
}

/* System call to force a task switch */
int sys_yield()
{
  force_task_switch();
  return 0;
}

extern int remaining_quantum;

int sys_get_stats(int pid, struct stats *st)
{
  int i;
  
  if (!access_ok(VERIFY_WRITE, st, sizeof(struct stats))) return -EFAULT; 
  
  if (pid<0) return -EINVAL;
  for (i=0; i<NR_TASKS; i++)
  {
    if (task[i].task.PID==pid)
    {
      task[i].task.p_stats.remaining_ticks=remaining_quantum;
      copy_to_user(&(task[i].task.p_stats), st, sizeof(struct stats));
      return 0;
    }
  }
  return -ESRCH; /*ESRCH */
}

extern int files_opened;

int sys_createScreen()
{
    struct task_struct *p = current();
    int c = p->screens;
    if(c < 10 && files_opened < 30) {
        p->screens++;
        p->channel_table[c] = open_screen_page( p );//2
        p->foco = c;

        return c;
    }
    return -1;
}

int sys_close()
{
    struct task_struct *p = current();
        page_table_entry *process_PT = get_PT(current());
    int c = p->screens;
    if(c > 1) {
        p->screens--;

        //del_user_page_screen(p,p->screens); usar?

        //Eliminar de la tabla pantallas y si se queda sin referencias quitarlo todo
                p->channel_table[p->foco]->content.bits.refs = (p->channel_table[p->foco]->content.bits.refs) - 1;

                if (p->channel_table[p->foco]->content.bits.refs == 0){
                   p->channel_table[p->foco]->content.entry = 0;
           p->channel_table[p->foco]->content.bits.refs = 0;
           p->channel_table[p->foco]->content.bits.rwpointer = 0;
           p->channel_table[p->foco]->content.bits.color = 0;

           /* Deallocate allocated pages. Delete reserved pages. */
                   free_frame(get_frame(process_PT, p->channel_table[p->foco]->logicpage));
                   p->channel_table[p->foco]->content.entry = 0;
           p->channel_table[p->foco]->logicpage = NULL;

               files_opened--;

                }

        //Eliminar de la tabla de canales y cambiar foco
                p->channel_table[p->foco] = NULL;
        p->foco = (p->screens)-1;
        return 0;
    }
    return -1;
}

int sys_getfocus(int focus)
{
	return current()->foco;
}

int sys_setFocus(int canal)
{
	struct task_struct *p = current();
	if(canal >= 0 && canal <10 && canal < p->screens) {
		p->foco = canal;
		return 0;
	}
	return -1;
}
