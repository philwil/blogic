/* 
   main start of it all
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>

#define NUM_IO 32
#define NUM_INS 16
#define NUM_OUTS 16

enum {
  BLK_NULL,
  BLK_INV,
  BLK_AND,
  BLK_OR,
};

typedef struct io io_t;
typedef struct blk blk_t;
typedef struct typ typ_t;

struct io
{
  int idx;
  char *name;
  int value;
  int changed;
  io_t *next;
};

struct blk
{
  int idx;
  char *name;
  char *type_name;
  int type;
  io_t *ins[NUM_INS];
  io_t *outs[NUM_OUTS];
  int num_ins;
  int num_outs;
  blk_t *next;
};


struct typ 
{
  int id;
  char *name;
  int (*fcn) (blk_t *blk, int time);
};


int fcn_null(blk_t *t, int time);
int fcn_inv(blk_t *t, int time);
int fcn_and(blk_t *t, int time);
int fcn_or(blk_t *t, int time);


typ_t g_typs[] = {
  { BLK_NULL,   "NULL", fcn_null},
  { BLK_INV,    "INV",  fcn_inv},
  { BLK_AND,    "AND",  fcn_and},
  { BLK_OR,     "OR",   fcn_or},
  { -1, NULL},
};

io_t *g_io[2]={NULL,NULL};

blk_t *g_blks=NULL;
blk_t *last_blk=NULL;

io_t *find_io_name(io_t*io, char *name);
int set_io(char *name, int value);


blk_t *new_blkx(blk_t *blk_in, char *name, int type)
{
  blk_t *item;
  int i;
  item = malloc(sizeof(blk_t));
  last_blk = item;
  if(!g_blks)g_blks = item;
  if(blk_in) blk_in->next = item;
  if(blk_in)item->idx = blk_in->idx+1;
  item->next = NULL;
  item->name = NULL;
  item->type = type;
  if(name) item->name =strdup(name);
  for (i = 0 ; i < NUM_INS ; i++)
    item->ins[i] = NULL;
  item->num_ins = 0;
  for (i = 0 ; i < NUM_OUTS ; i++)
    item->outs[i] = NULL;
  item->num_outs = 0;
  return item;
}

int fetch_type(char *type)
{
  typ_t *item;
  int i=0;
  while(i>=0)
    {
      item=&g_typs[i];
      if((strcmp(item->name, type) == 0)
	 || (item->id < 0))
	return item->id;
      i++;
    }
  return -1;
}


char *get_type(int type)
{
  typ_t *item;
  int i=0;
  while(i>=0)
    {
      item=&g_typs[i];
      if((item->id == type)
	 || (item->id < 0))
	return item->name;
      i++;
    }
  return NULL;
}

int run_type(blk_t *blk, int type, int time)
{
  typ_t *item;
  int i=0;
  while(i>=0)
    {
      item=&g_typs[i];
      if(item->id == type)
	 return item->fcn(blk, time);
      i++;
    }
  return 0;
}

blk_t *new_blkc(char *name, char *type)
{

  int tint = fetch_type(type);
  return new_blkx(last_blk, name, tint);
}

void add_blk_in(blk_t *item, char *name)
{
  io_t *io;
  io = find_io_name(g_io[0], name);
  printf("add_blk_in name [%s] io %p\n", name, io);
  if(!io->name)set_io(name, 0);
  io = find_io_name(g_io[0], name);
  item->ins[item->num_ins++] = io;
}

void add_blk_out(blk_t *item, char *name)
{
  io_t *io;
  io = find_io_name(g_io[1], name);
  printf("add_blk_out name [%s] io %p\n", name, io);
  if(!io->name)set_io(name, 0);
  io = find_io_name(g_io[1], name);
  item->outs[item->num_outs++] = io;
}

//new_blk("BLK name type INV IN v1 v2 v3 OUT ov1 ov2 ov3");
blk_t *new_blk(char *def)
{
  char name[64];
  char type[64];
  int idx;
  char *spin;
  char *spout;
  blk_t *item=NULL;
  idx = sscanf(def, "BLK %s type %s", name, type);
  if(idx > 1)
    {
      item = new_blkc(name, type);

    }
  if(item)
    {      
      spin = strstr(def, "IN");
      spout = strstr(def, "OUT");
      if(spin)spin+=strlen("IN ");
      if(spout)
	{
	  spout+=strlen("OUT ");
	}
      
      while(spin)
	{
	  idx =  sscanf(spin, "%s", name);
	  
	  if(idx)
	    {
	      if(strcmp(name,"OUT") == 0)
		{
		  spin=NULL;
		  continue;
		}
	      if(strcmp(name,"IN") == 0)
		{
		  spin = strstr(spin, name);
		  spin+=strlen(name);
		  continue;
		}
	      add_blk_in(item, name);
	      spin = strstr(spin, name);
	      spin+=strlen(name);
	    }
	  else
	    {
	      spin = NULL;
	    }
	}
      while(spout && (strlen(spout) > 0))
	{
	  idx =  sscanf(spout, "%s", name);
	  if(idx)
	    {
	      add_blk_out(item, name);
	      spout = strstr(spout, name);
	      spout+=strlen(name);
	    }
	  else
	    {
	      spout = NULL;
	    }
	}
    }
  return item;
}

void show_blks(void)
{
  int i;
  blk_t *item=g_blks;
  io_t *io;
  while (item)
    {
      printf("BLK [%d] %s type %s IN ", item->idx, item->name, get_type(item->type)); 
      for(i = 0; i < item->num_ins; i++)
	{
	  io = item->ins[i];
	  printf("%s ", io->name);
	}
      printf("OUT ");
      for(i = 0; i < item->num_outs; i++)
	{
	  io = item->outs[i];
	  printf("%s ", io->name);
	}
      printf("\n");
      item=item->next;
    }

}

void run_blks(int time)
{
  int i;
  blk_t *item=g_blks;
  io_t *io;
  while (item)
    {
      run_type(item, item->type, time);
      item=item->next;
    }

}

io_t *new_io(io_t *io_in, int idx, char *name, int value)
{
  io_t *io;
  io = malloc(sizeof(io_t));
  if(io_in) io_in->next = io;
  io->idx = idx;
  io->next = NULL;
  io->name = NULL;
  if(name) io->name =strdup(name);
  io->value = value;
  io->changed = 0;
  return io;

}

void show_ios(io_t *io, char *head)
{
  if (head) printf("%s", head);
  printf(">>[");
  while(io)
    {
      printf("%d", io->value);
      io=io->next;
    }
  printf("]<<\n");
}


int setup_ios(int num)
{
  int i;
  io_t *io1=NULL;
  io_t *io2=NULL;

  for (i = 0 ; i < num; i++)
    {
      io1 = new_io(io1, i, NULL, 0);
      io2 = new_io(io2, i, NULL, 0);
      if(!g_io[0]) g_io[0]= io1;
      if(!g_io[1]) g_io[1]= io2;
    } 
}

io_t *find_io_name(io_t*io, char *name)
{
  while (io)
    {
      if(!io->name)
	return io;
      if(strcmp(io->name, name) == 0) 
	return io;
      io=io->next;
    }
  return io;
}

int set_io(char *name, int value)
{
  io_t *io = find_io_name(g_io[1], name);
  if(!io->name)
    {
      io->name=strdup(name);
      io = find_io_name(g_io[0], name);
      io->name=strdup(name);
    }
  io = find_io_name(g_io[0], name);
  io->value = value;
  io->changed;
  return io->changed;
}


int fcn_null(blk_t *it, int t)
{
  printf("fcn_null running on blk [%s]\n", it->name);
  return 0;
}

int fcn_inv(blk_t *it, int t)
{
  printf("fcn_inv running on blk [%s]\n", it->name);
  return 0;
}

int fcn_and(blk_t *it, int t)
{
  printf("fcn_and running on blk [%s]\n", it->name);
  return 0;
}


int fcn_or(blk_t *it, int t)
{
  printf("fcn_or running on blk [%s]\n", it->name);
  return 0;
}

int main (int argc , char *argv[])
{

  printf("We are running %s\n", argv[0]);
  setup_ios(NUM_IO);
  show_ios(g_io[0], "IO BANK 0");
  show_ios(g_io[1], "IO BANK 1");
  set_io("out1",0);
  set_io("out2",1);
  set_io("out3",2);
  show_ios(g_io[0], "IO BANK 0");
  new_blkx(last_blk, "NULL" , BLK_NULL);
  new_blkx(last_blk, "INV" , BLK_INV);
  new_blkx(last_blk, "AND" , BLK_AND);
  new_blkx(last_blk, "OR" , BLK_OR);
  new_blk("BLK name type INV IN v1 v2 v3 OUT ov1 ov2 ov3");
  show_blks();
  run_blks(100);
  return 0;

}
