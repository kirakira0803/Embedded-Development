#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <linux/input.h>
#include <sys/mman.h>
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
int fd=-1;
int *pLcd=MAP_FAILED;
void Point(int x,int y,int color)
{
	*(pLcd+(800*y+x))=color;
}
int LcdInit()
{
	fd=open("/dev/fb0",O_RDWR);
	if(fd==-1)
	{
		printf("open LCD error\n");
		return -1;
	}
	pLcd = mmap(NULL,800*480*4,PROT_READ|PROT_WRITE,MAP_SHARED,
					fd,0);
	if(pLcd==MAP_FAILED)
	{
		printf("mmap error\n");
		return -1;
	}
	return 0;
}
void LcdClose()
{
	if(pLcd != MAP_FAILED)
	{	
		munmap(pLcd,800*480*4);
	}
	if(fd!=-1)
	{
		close(fd);
	}
}
int BmpDisplay(char *BmpPath)
{
	int fd=open(BmpPath,O_RDWR);
	if(fd==-1)
	{
		printf("open bmp error\n");
		return -1;
	}
	unsigned int width=0,high=0,depth=0;
	lseek(fd,0x12,SEEK_SET);
	read(fd,&width,4);
	lseek(fd,0x16,SEEK_SET);
	read(fd,&high,4);
	lseek(fd,0x1C,SEEK_SET);
	read(fd,&depth,2);

	int hunzi=0;
	int line_Value_byte=0;
	int line_byte=0;
	int totle_byte=0;
	line_Value_byte = abs(width) *(depth/8);
	if(line_Value_byte%4)
	{
		hunzi=4-(line_Value_byte%4);
	}
	
	line_byte = line_Value_byte+hunzi;
	totle_byte = line_byte*abs(high);
	unsigned char *pBmp=malloc(totle_byte);
	read(fd,pBmp,totle_byte);
	unsigned char bit_a=0,bit_r=0,bit_g=0,bit_b=0;
	unsigned int color[480][800]={0};
	unsigned int x,y;
	int id=0;printf("7\n");
	for(y=0;y<480;y++)
	{
		for(x=0;x<800;x++)
		{
			bit_b=pBmp[id++];
			bit_g=pBmp[id++];
			bit_r=pBmp[id++];
			if(depth==32)
			{
				bit_a=pBmp[id++];
			}
			color[y][x]=bit_a<<24|bit_r<<16|bit_g<<8|bit_b;
		}
		id=id+hunzi;
	}printf("6\n");
	for(y=0;y<480;y++)
	{
		for(x=0;x<800;x++)
		{
			Point(x,y,color[y][x]);
		}
	}printf("5\n");
	
	free(pBmp);
	close(fd);
}
int GetDirection()
{
	int fd=open("/dev/input/event0",O_RDWR);
	if(fd==-1)
	{
		printf("open event0 error\n");
		return -1;
	}
	struct input_event event0;
	int res =0;
	int x_start=-1;
	int y_start=-1;
	int x_end=-1;
	int y_end=-1;
	while(1)
	{
		res=read(fd,&event0,sizeof(event0));
		if(res != sizeof(event0))
		{
			continue;
		}
		if(event0.type==EV_KEY&&event0.code==BTN_TOUCH
			&& event0.value==0)
		{
			break;
		}
		if(event0.type==EV_ABS)
		{
			if(event0.code==ABS_PRESSURE && event0.value==0)
			{
				break;
			}
			if(event0.code == ABS_X)
			{
				if(x_start==-1)
				{
					x_start=event0.value;
				}
				x_end=event0.value;
			}
			if(event0.code==ABS_Y)
			{
				if(y_start==-1)
				{
					y_start=event0.value;
				}
				y_end=event0.value;
			}
		}
	}
	if(abs(x_end-x_start)>abs(y_end-y_start))
	{
		if(x_end-x_start>0)
		{
			return RIGHT;
		}
		else
		{
			return LEFT;
		}
	}
	if(abs(x_end-x_start)<abs(y_end-y_start))
	{
		if(y_end-y_start>0)
		{
			return DOWN;
		}
		else
		{
			return UP;
		}
	}
}
int main()
{
	/*for(int y=0;y<480;y++)
	{
		for(int x=0;x<800;x++)
		{
			color[y][x]=0xFFFF00;
		}
	}*/
	/*
	struct BITMAPFILEHEADER			   //文件头
	{
		short bfType;                  //"BM"代表bmp文件
		unsigned long bfSize;		   //BMP文件的大小
		short bfReserved1;			   //保留，必须为0
		short bfReserved2;			   //保留，必须为0
		unsigned long bfOffBits;	   //从文件开头到位图数据开始的字节数
	}；
	struct BITMAPINFOHEADER            //信息头
	{
		unsigned long biSize;          //本信息头占用的字节数
		unsigned long biWidth;		   //图像的宽度（以像素为单位)
		unsigned long biHight;		   //图像的高度（以像素为单位）
		short biPlanes;				   //调色盘（值应为1）
		short biBitCount;			   //一个像素点的位数（24位则24bit rgb）
		unsigned long biCompression;   //压缩方式（不压缩即为0）
		unsigned long biSizeImage;	   //0
		unsigned long biXPelsPerMeter; //水平分辨率
		unsigned long biYPelsPerMeter; //垂直分辨率
		unsigned long biClrUsed;       //0
		unsigned long biClrImportant;  //0
	};
	struct RGB          //像素的单个点
	{
		unsigned char B;			//蓝色bule
		unsigned char G;			//绿色green
		unsighed char R;			//红色red
	};
	*/
	
	LcdInit();
	char *bmpPath[2]={"./0.bmp","./1.bmp"};
	int i=0;
	int diretion;
	while(1)
	{
		printf("1\n");
		diretion=GetDirection();printf("3\n");
		if(diretion==UP||diretion==RIGHT)
		{
			i--;
			if(i<0)
			{
				i=2-1;
			}
		}
		if(diretion==DOWN||diretion==LEFT)
		{
			i++;
			if(i>2-1)
			{
				i=0;
			}
		}
		BmpDisplay(bmpPath[i]);printf("2\n");
	}
	LcdClose();
	return 0;
}
