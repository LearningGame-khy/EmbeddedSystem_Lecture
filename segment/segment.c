#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <asm/fcntl.h>
#include <linux/ioport.h>
#include <linux/delay.h>

#include <asm/ioctl.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define DRIVER_AUTHOR		"hanback"		
#define DRIVER_DESC		"7-Segment program"	

#define	SEGMENT_MAJOR		242			
#define	SEGMENT_NAME		"SEGMENT"		
#define SEGMENT_MODULE_VERSION	"SEGMENT PORT V0.1"	

#define SEGMENT_ADDRESS_GRID	0x88000030	
#define SEGMENT_ADDRESS_DATA	0x88000032	
#define SEGMENT_ADDRESS_1	0x88000034	
#define SEGMENT_ADDRESS_RANGE	0x1000		

#define MODE_0_WAIT			0
#define MODE_1_CORRECT		1
#define MODE_2_NOT_CORRECT	5

static unsigned int segment_usage = 0;
static unsigned long *segment_data;
static unsigned long *segment_grid;
static int mode_select = 0;

int segment_open (struct inode *inode, struct file *filp) {
	if(segment_usage != 0) return -EBUSY;
	
	segment_grid =  ioremap(SEGMENT_ADDRESS_GRID, SEGMENT_ADDRESS_RANGE);
	segment_data =  ioremap(SEGMENT_ADDRESS_DATA, SEGMENT_ADDRESS_RANGE);
	
	if(!check_mem_region((unsigned long)segment_data,SEGMENT_ADDRESS_RANGE) && !check_mem_region((unsigned long)segment_grid, SEGMENT_ADDRESS_RANGE)) {
		request_region((unsigned long)segment_grid, SEGMENT_ADDRESS_RANGE, SEGMENT_NAME);
		request_region((unsigned long)segment_data, SEGMENT_ADDRESS_RANGE, SEGMENT_NAME);
	} else printk("driver : unable to register this!\n");

	segment_usage = 1;	
	return 0; 
}

int segment_release (struct inode *inode, struct file *filp) {
	iounmap(segment_grid);
	iounmap(segment_data);

	release_region((unsigned long)segment_data, SEGMENT_ADDRESS_RANGE);
	release_region((unsigned long)segment_grid, SEGMENT_ADDRESS_RANGE);

	segment_usage = 0;
	return 0;
}

unsigned char Getsegmentcode (unsigned int x) {
	unsigned char code = 0;
	
	switch (x) {
		case 0 : code = 0xfc; break;
		case 1 : code = 0x60; break;
		case 2 : code = 0xda; break;
		case 3 : code = 0xf2; break;
		case 4 : code = 0x66; break;
		case 5 : code = 0xb6; break;
		case 6 : code = 0xbe; break;
		case 7 : code = 0xe4; break;
		case 8 : code = 0xfe; break;
		case 9 : code = 0xf6; break;
		
		case 45	: code = 0x02; break;	//45 = '-'

		default : code = 0; break;
	}
	return code;
}

void Wait_Mode(unsigned char *data, unsigned int mod) {
	unsigned int i =0;
	
	for (i=0; i<6; i++) data[i] = 0;
	
	switch (mod) {
		case 0: 
			data[0] = 0xf0;
			data[1] = 0x90;
			data[2] = 0x90;
			data[3] = 0x90;
			data[4] = 0x90;
			data[5] = 0x9c;
			break;
			
		case 1: data[0] = 0x80; break;
		case 2: data[1] = 0x80; break;
		case 3: data[2] = 0x80; break;
		case 4: data[3] = 0x80; break;
		case 5: data[4] = 0x80; break;
		case 6: data[5] = 0x80; break;
		case 7: data[5] = 0x0c; break;
		case 8: data[5] = 0x10; break;
		case 9: data[4] = 0x10; break;
		case 10: data[3] = 0x10; break;
		case 11: data[2] = 0x10; break;
		case 12: data[1] = 0x10; break;
		case 13: data[0] = 0x10; break;
		case 14: data[0] = 0x60; break;
		
		default: break;
	}
	return;
}

void SetArray_Value(unsigned char *data, int iValue){
	data[5]=Getsegmentcode((iValue/100000)	%	10);
	data[4]=Getsegmentcode((iValue/10000)	%	10);
	data[3]=Getsegmentcode((iValue/1000)	%	10);
	data[2]=Getsegmentcode((iValue/100)		%	10);
	data[1]=Getsegmentcode((iValue/10)		%	10);
	data[0]=Getsegmentcode((iValue/1)		%	10);
	
	return;
}

void SetArray_voidFill(unsigned char *data, int iValue){
	int i;
	int range = 1;
	
	for(i = 0; i < 6 ; i++){
		iValue = iValue/10;
		if(iValue > 0){
			range++;
		}
	}
	
	for(i = 6 ; i > range ; i--){
		if(i == 1){
			//
		}
		else{
			data[i - 1] = 0;
		}
	}
	return;
}

void SetArray_Shift(unsigned char *data, int levelNum){
	unsigned char temp[6];
	unsigned int i;
	for(i = 0 ; i < 6 ; i++) {
		temp[i] = data[i];
	}
	for(i=0;i<6;i++) {
		data[i] = temp[(i + levelNum) % 6 ];
	}
	return;
}

void SetArray_Sub(unsigned char *data, unsigned int iValue, unsigned int rValue){
	data[5] = 0x00;
	data[4] = Getsegmentcode((iValue/10)	%10);
	data[3] = Getsegmentcode((iValue/1)		%10);
	data[2] = Getsegmentcode(45);
	data[1] = Getsegmentcode((rValue/10)	%10);
	data[0] = Getsegmentcode((rValue/1)		%10);
	return;
}

ssize_t segment_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) {
    unsigned char data[6];
    unsigned char digit[6]={0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
    unsigned int i,num,ret;
	unsigned int input = 0;

    ret=copy_from_user(&num, gdata, 4);	
	input = num;
	
	for (i=0; i<6; i++) data[i] = 0;
	
	if (mode_select == MODE_0_WAIT) {
		Wait_Mode(data, num);
	}
	else if (mode_select == MODE_1_CORRECT) {
		SetArray_Value(data, input);
		SetArray_voidFill(data, input);
	}
	else if (mode_select == MODE_2_NOT_CORRECT) {
		SetArray_Sub(data, input/100, input%100);
		//SetArray_Value(data, input);
	}
	else if(mode_select >= 100 && mode_select <= 105){
		SetArray_Value(data, input);
		SetArray_voidFill(data, input);
		SetArray_Shift(data, mode_select%100);
	}
	else if(mode_select >=200 && mode_select <= 205) {
		SetArray_Sub(data, input/100, input%100);
		SetArray_Shift(data, mode_select%100);
	}
	else{
		data[0] = 0xf0;
		data[1] = 0x90;
		data[2] = 0x90;
		data[3] = 0x90;
		data[4] = 0x90;
		data[5] = 0x9c;
	}
	
	for(i=0;i<6;i++) {
		*segment_grid = digit[i];
		*segment_data = data[i];
		mdelay(3);	
	}
    
    *segment_grid = ~digit[0];
    *segment_data = 0;

    return length;
}

static int segment_ioctl(struct file *flip, unsigned int cmd, unsigned long arg) {
    switch(cmd) {
		case MODE_0_WAIT: 			mode_select = 0; break;
		case MODE_1_CORRECT:		mode_select = 1; break;
		case MODE_2_NOT_CORRECT:	mode_select = 5; break;
		
		case 100: mode_select = 100; break;
		case 101: mode_select = 101; break;
		case 102: mode_select = 102; break;
		case 103: mode_select = 103; break;
		case 104: mode_select = 104; break;
		case 105: mode_select = 105; break;
		
		case 200: mode_select = 200; break;
		case 201: mode_select = 201; break;
		case 202: mode_select = 202; break;
		case 203: mode_select = 203; break;
		case 204: mode_select = 204; break;
		case 205: mode_select = 205; break;

		default: return -EINVAL;
    }
    return 0;
}
							    
struct file_operations segment_fops = {
	.owner		= THIS_MODULE,
	.open		= segment_open,
	.write		= segment_write,
	.release	= segment_release,
	.unlocked_ioctl		= segment_ioctl,
};

int segment_init(void) {
	int result;

	result = register_chrdev(SEGMENT_MAJOR, SEGMENT_NAME, &segment_fops);
	if (result < 0) {
		printk(KERN_WARNING"Can't get any major\n");
		return result;
	}

	printk(KERN_INFO"Init Module, 7-Segment Major Number : %d\n", SEGMENT_MAJOR);
	return 0;
}


void segment_exit(void) {
	unregister_chrdev(SEGMENT_MAJOR,SEGMENT_NAME);

	printk("driver: %s DRIVER EXIT\n", SEGMENT_NAME);
}

module_init(segment_init);	
module_exit(segment_exit);	

MODULE_AUTHOR(DRIVER_AUTHOR);	
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("Dual BSD/GPL");	

