/*
Matt Hintzke
CS 460
KC Wang
Assignment #1
*/

#include "ext2.h"

char* cp;
INODE* ip;
DIR *dp;
GD *gb;

/* Define variables outside of main to save space in a.out */ 
char buff[1024]; 
char buff2[1024]; 
int inodeNumber; 
int Default = 0; 	
u32* ib; 
int i; 
char file[32]; 	// the name of the desired file
int iblk;

// Print string
void prints(char *s) 
{
	 
	while(*s != '\0')
	{
		putc(*s);  
		s++; 
	}
}

void getblk(int blk, char * buff)
{
	readfd(blk/18, (blk/9)%2, (blk*2)%18, buff);
}

unsigned long get_ino(char* name, char *buff)
{
	int inodeNumber; 
	
	

	inodeNumber = 0; 

	cp = buff; 
	dp = (DIR *)cp; 

	while(cp < &buff[1024])
	{ 
		
		if(!strncmp(dp->name, name, dp->name_len))
		{
		 
			return dp->inode; 
		}

		cp += dp->rec_len; 			// Go to next directory 
		dp = (DIR *)cp;
	}
	//prints("\n\rCould not find "); prints(name); 
	return inodeNumber; 	// Returns 0 
	
}


void gets( char* s)
{
	 
	*s = getc(); 
	while(*s != '\r')	// while the Enter key is not pressed 
	{
		
		putc(*s); 	// echo the character 
		s++; 		// move to the next spot in the char array 
		*s = getc();  	// read another character 
	}
	*s = '\0';
}

int main()
{
	
	

	inodeNumber = 0; 
	while(inodeNumber == 0)
	{
		prints("boot image (enter for MTX): ");  
		gets(file);		// Read user input of desired boot image

		if(*file == '\0')
		{ 
			Default = 1;
		}

		
		getblk(2, buff);

		gb = (GD *)buff;

		iblk = (u16)gb->bg_inode_table;

		getblk((u16)iblk, buff); 	// Load block 5 which is where Inodes table begins
		
		ip = (INODE *)buff + 1; 	// Read in root INODE

		getblk((int)ip->i_block[0], buff);	// Read root->i_block[0] into buff 
		// I've looked through and discovered that all of the i_blocks in root 
			// are empty except i_block[0]

		inodeNumber = get_ino("boot", buff); 	// Get inode # of boot directory 
							// inodeNumber = 13 
	
		getblk(((inodeNumber - 1)/8) + 5, buff); 	// Read boot's data block into buff 

		ip = (INODE *)buff + (inodeNumber-1)%8; 

		getblk((int)ip->i_block[0], buff); 

		 
		if(Default)
		{
			//prints("Searching for MTX..."); 
			file[0] = 'm'; 
			file[1] = 't'; 
			file[2] = 'x'; 	
			file[3] = 0; 	// most space efficient way to assign array :) 

			inodeNumber = get_ino(file, buff); 
		}
		else
		{
			//prints("Searching for "); prints(s); prints("..."); 
			inodeNumber = get_ino(file, buff);
		}
	
		 
	}


	getblk(((inodeNumber - 1)/8) + 5, buff); // Read mtx' data block into buff
	ip = (INODE *)buff + (inodeNumber-1)%8; 
	//ip += (inodeNumber-1)%8;	// ip now points to inode of mtx 

	 
	getblk((int)ip->i_block[12],buff2);      // Load indirect blocks into buff2


	// Since ES points to 0x1000, getblk at 0 offset will put this all in memory 
	setes(0x1000);		// Set ES to 0x1000
	for(i = 0; i<12; i++)
    { 
    	if(ip->i_block[i] == 0)
			return 0; 
    	

		getblk((int)ip->i_block[i],0);
		//putc('.');
		//getc();
      	inces();   
   	}
	
	ib = buff2; 
	
	 //Read in each block by block in indirect blocks 
	for(i=0; i<256; i++)
	{
		getblk(*ib, 0); 
		inces(); 	
		ib++; 
	}

	return 1; 


}
