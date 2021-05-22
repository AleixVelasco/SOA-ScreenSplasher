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

  int screens = current()->screens;
  for (pag=NUM_PAG_DATA; pag<NUM_PAG_DATA+screens; pag++)
  {
    new_ph_pag=alloc_frame();
    if (new_ph_pag!=-1) /* One page allocated */
    {
      set_ss_screen_pag(process_PT, PAG_LOG_INIT_DATA+pag, new_ph_pag);
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
  /* Copy parent's DATA to child. We will use TOTAL_PAGES-1 as a temp logical page to map to */
  for (pag=NUM_PAG_KERNEL+NUM_PAG_CODE; pag<NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA; pag++)
  {
    /* Map one child page to parent's address space. */
    set_ss_pag(parent_PT, pag+NUM_PAG_DATA, get_frame(process_PT, pag));
    copy_data((void*)(pag<<12), (void*)((pag+NUM_PAG_DATA)<<12), PAGE_SIZE);
    del_ss_pag(parent_PT, pag+NUM_PAG_DATA);
  }

	for (pag=NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA; pag<NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA+screens; pag++)
  {
    /* Map one child page to parent's address space. */
    set_ss_screen_pag(parent_PT, pag+NUM_PAG_DATA, get_frame(process_PT, pag));
    copy_data((void*)(pag<<12), (void*)((pag+NUM_PAG_DATA)<<12), PAGE_SIZE);
    del_ss_pag(parent_PT, pag+NUM_PAG_DATA);
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
int escapeCode = 0, changeColor = 0;
char code[2];
int pos = 0;
Byte color;

	if ((ret = check_fd(fd, ESCRIPTURA)))
		return ret;
	if (nbytes < 0)
		return -EINVAL;
	if (!access_ok(VERIFY_READ, buffer, nbytes))
		return -EFAULT;
	
	Word buff[nbytes];
  color = current()->channel_table[fd]->content.bits.color;
int x = 0, y= 0;

		int j = 0;

  for(int i = 0; i<nbytes; ++i){ 
		if(escapeCode == 0) {
				// Condición para cambiar de color si previamente ha habido un escapeCode
					if(changeColor == 1) {
						// Foreground coloring
						if(code[0] == '3') {
							color = (Byte) ((color & 0xF0) | (Byte) (0x0F & code[1]));
							
						} else if(code[0] == '4') {// Background coloring
							color = (Byte) (color & 0x0F) | (code[1]<<4);
						}
						current()->channel_table[fd]->content.bits.color = color;
						changeColor = 0;
						
					}


				// Condición para detectar el carcter especial para indicar el inicio del escapeCode
				if(buffer[i] == '[') {
					escapeCode = 1;//printk(" Entra ");
					pos = 0;
				} else {
				 Word color_w = 0xFF00 & (color<<8);
				 Word w = (Word) (buffer[i] & 0x00FF) | color_w;
					char a = (char)buffer[i];
				 //printc_xy((Byte)y,(Byte)x,a,color);
				 buff[j] = w;
					j++;
				}
		} else {
			// Condición para detectar el carcter especial para indicar el final de escapeCode
			if(buffer[i] == 'm') {
				escapeCode = 0;
				changeColor = 1;//printk(" Sale ");
			} else {
				// Condición para que no desborde  el buffer del code
				if(pos <2){
					code[pos] = buffer[i];
					pos++;
				}
			}
		}

y = j % 80;
    if (j != 0 && j % 80 == 0) {
        x++;
    }
		
  }
//MIRAR ESTO
        current()->channel_table[fd]->content.bits.rwpointer = ((y & 0x7F)<<5) | (x & 0x1F);
	//(DWord) current()->channel_table[fd]->content.bits.rwpointer
	copy_data((void*)&buff, (void*)(current()->channel_table[fd]->logicpage<<12), nbytes*2);
	/*printc('\n',0x0F);
	printc('A',0x01);	printc('A',0x02);	printc('A',0x03);	printc('A',0x04);	printc('A',0x05);	printc('A',0x06);	printc('A',0x07);	printc('A',0x08);	printc('A',0x09);	printc('A',0x0A);	printc('A',0x0B);
	printc('A',0x0C);	printc('A',0x0D);	printc('A',0x0E);	printc('A',0x0F);
	printc('\n',0x0F);
	printc('A',0x10);	printc('A',0x20);	printc('A',0x30);	printc('A',0x40);	printc('A',0x50);	printc('A',0x60);	printc('A',0x70);	

	char a = '\033';printc(a,0x0F);*/

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
    int c = p->screens;//1
    if(c <= 10 && files_opened < 30) {
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
        //page_table_entry *process_PT = get_PT(current());
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



                   //free_frame(get_frame(process_PT, (unsigned int)(p->channel_table[p->foco]->logicpage)));
                   //p->channel_table[p->foco]->logicpage->entry = 0;

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
