#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <winsock2.h>
#include <pthread.h>
#define SENDNAME "pretty5.mp4"
#define SHORT_BUF (1100)

#define LOCAL_IP "192.168.1.7"
#define LOCAL_PORT ((uint16_t)12345)

#define debug(...)           \
    do                       \
    {                        \
        printf(__VA_ARGS__); \
        fflush(stdout);      \
    } while (0)




#pragma pack(2)
typedef struct tagBITMAPFILEHEADER3{
	WORD bfType;//位图文件的类型，在Windows中，此字段的值总为‘BM’(1-2字节）
	DWORD bfSize;//位图文件的大小，以字节为单位（3-6字节，低位在前）
	WORD bfReserved1;//位图文件保留字，必须为0(7-8字节）
	WORD bfReserved2;//位图文件保留字，必须为0(9-10字节）
	DWORD bfOffBits;//位图数据的起始位置，以相对于位图（11-14字节，低位在前）
	//文件头的偏移量表示，以字节为单位
} BitMapFileHeader;	//BITMAPFILEHEADER;
 #pragma pack()

typedef struct tagBITMAPINFOHEADER3{
	DWORD biSize;//本结构所占用字节数（15-18字节）
	LONG biWidth;//位图的宽度，以像素为单位（19-22字节）
	LONG biHeight;//位图的高度，以像素为单位（23-26字节）
	WORD biPlanes;//目标设备的级别，必须为1(27-28字节）
	WORD biBitCount;//每个像素所需的位数，必须是1（双色），（29-30字节）
	//4(16色），8(256色）16(高彩色)或24（真彩色）之一
	DWORD biCompression;//位图压缩类型，必须是0（不压缩），（31-34字节）
	//1(BI_RLE8压缩类型）或2(BI_RLE4压缩类型）之一
	DWORD biSizeImage;//位图的大小(其中包含了为了补齐行数是4的倍数而添加的空字节)，以字节为单位（35-38字节）
	LONG biXPelsPerMeter;//位图水平分辨率，像素数（39-42字节）
	LONG biYPelsPerMeter;//位图垂直分辨率，像素数（43-46字节)
	DWORD biClrUsed;//位图实际使用的颜色表中的颜色数（47-50字节）
	DWORD biClrImportant;//位图显示过程中重要的颜色数（51-54字节）
} BitMapInfoHeader;	//BITMAPINFOHEADER;
 
 
typedef struct tagRGBQUAD2{
	BYTE rgbBlue;//蓝色的亮度（值范围为0-255)
	BYTE rgbGreen;//绿色的亮度（值范围为0-255)
	BYTE rgbRed;//红色的亮度（值范围为0-255)
	BYTE rgbReserved;//保留，必须为0
} RgbQuad2;	//RGBQUAD;
  
int Rgb565ConvertBmp(char *buf,int width,int height, const char *filename)
{
	FILE* fp;
 
	BitMapFileHeader bmfHdr; //定义文件头
	BitMapInfoHeader bmiHdr; //定义信息头
	RgbQuad2 bmiClr[3]; //定义调色板
 
	bmiHdr.biSize = sizeof(BitMapInfoHeader);
	bmiHdr.biWidth = width;//指定图像的宽度，单位是像素
	bmiHdr.biHeight = height;//指定图像的高度，单位是像素
	bmiHdr.biPlanes = 1;//目标设备的级别，必须是1
	bmiHdr.biBitCount = 16;//表示用到颜色时用到的位数 16位表示高彩色图
	bmiHdr.biCompression = BI_BITFIELDS;//BI_RGB仅有RGB555格式
	bmiHdr.biSizeImage = (width * height * 2);//指定实际位图所占字节数
	bmiHdr.biXPelsPerMeter = 0;//水平分辨率，单位长度内的像素数
	bmiHdr.biYPelsPerMeter = 0;//垂直分辨率，单位长度内的像素数
	bmiHdr.biClrUsed = 0;//位图实际使用的彩色表中的颜色索引数（设为0的话，则说明使用所有调色板项）
	bmiHdr.biClrImportant = 0;//说明对图象显示有重要影响的颜色索引的数目，0表示所有颜色都重要
 
	//RGB565格式掩码
	bmiClr[0].rgbBlue = 0;
	bmiClr[0].rgbGreen = 0xF8;
	bmiClr[0].rgbRed = 0;
	bmiClr[0].rgbReserved = 0;
 
	bmiClr[1].rgbBlue = 0xE0;
	bmiClr[1].rgbGreen = 0x07;
	bmiClr[1].rgbRed = 0;
	bmiClr[1].rgbReserved = 0;
 
	bmiClr[2].rgbBlue = 0x1F;
	bmiClr[2].rgbGreen = 0;
	bmiClr[2].rgbRed = 0;
	bmiClr[2].rgbReserved = 0;
 
 
	bmfHdr.bfType = (WORD)0x4D42;//文件类型，0x4D42也就是字符'BM'
	bmfHdr.bfSize = (DWORD)(sizeof(BitMapFileHeader) + sizeof(BitMapInfoHeader) + sizeof(RgbQuad2) * 3 + bmiHdr.biSizeImage);//文件大小
	bmfHdr.bfReserved1 = 0;//保留，必须为0
	bmfHdr.bfReserved2 = 0;//保留，必须为0
	bmfHdr.bfOffBits = (DWORD)(sizeof(BitMapFileHeader) + sizeof(BitMapInfoHeader)+ sizeof(RgbQuad2) * 3);//实际图像数据偏移量
 
	if (!(fp = fopen(filename, "wb"))){
		return -1;
	} else {
		printf("file %s open success\n",filename);
	}
 
	fwrite(&bmfHdr, 1, sizeof(BitMapFileHeader), fp); 
	fwrite(&bmiHdr, 1, sizeof(BitMapInfoHeader), fp); 
	fwrite(&bmiClr, 1, 3*sizeof(RgbQuad2), fp);
 
//	fwrite(buf, 1, bmiHdr.biSizeImage, fp);	//mirror
	for(int i=0; i<height; i++){
		fwrite(buf+(width*(height-i-1)*2), 2, width, fp);
	}
 
	printf("Image size=%d, file size=%d, width=%d, height=%d\n", bmiHdr.biSizeImage, bmfHdr.bfSize, width, height);
	printf("%s over\n", __FUNCTION__);
	fclose(fp);
 
	 return 0;
}





int main(void)
{
    int ret = 0;
    WSADATA ws;
    SOCKET sock, newsock;
    struct sockaddr_in local;
    struct sockaddr_in client;
    int addrlen = 0;
    int nrecv = 0;
    unsigned  char buf[614400];
       unsigned char buf_short[SHORT_BUF];


    if (WSAStartup(MAKEWORD(2, 2), &ws) != 0)
    {
        ret = -1;
        goto __exit;
    }

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        ret = -2;
        goto __exit;
    }

    local.sin_family = AF_INET;
    local.sin_addr.S_un.S_addr = inet_addr(LOCAL_IP);
    local.sin_port = htons(LOCAL_PORT);
    memset(local.sin_zero, 0X00, sizeof(local.sin_zero));

    if (bind(sock, (const struct sockaddr *)&local, sizeof(local)) == SOCKET_ERROR)
    {
        ret = -3;
        printf("wrong\n");
        goto __exit;
    }
    BOOL bOptVal = FALSE;
    int iResult;
    iResult = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char *)&bOptVal, sizeof(BOOL));
    if (iResult == SOCKET_ERROR)
    {
        wprintf(L"setsockopt for SO_KEEPALIVE failed with error :%u\n ", WSAGetLastError());
    }
    else
    {
        wprintf(L"right pass\n");
    }

    listen(sock, 5);

    addrlen = sizeof(local);

    debug("accept %s:%d...\r\n", LOCAL_IP, LOCAL_PORT);

    int len=SHORT_BUF;
    int circles_end=(int)614400/len;
	int last_circle_bytes=614400%len;
    	printf("circle times %d,last bytes;%d\n",circles_end,last_circle_bytes);


    if ((newsock = accept(sock, (struct sockaddr *)&client, &addrlen)) == SOCKET_ERROR)
    {
        ret = -4;
        goto __exit;
    }
    debug("%s:%d connected\r\n", inet_ntoa(client.sin_addr), client.sin_port);
    char filename[128];
    unsigned char filename_len[1];
    int counter=0;
	if(last_circle_bytes!=0)circles_end+=1;


    while(counter<circles_end){

    if(counter==(circles_end-1)&&last_circle_bytes!=0)
    {
        len=last_circle_bytes;
        printf("last ------%d\n",len);


    }

   recv(newsock, buf_short, len, 0);
   memcpy(buf+counter*SHORT_BUF,buf_short,len);


//    if(buf_short[0]!=0&&buf_short[0]!=136){
//     printf("index %d=== ",index);  
//       for(int i=0;i<20;i++){
//  printf("%d,",buf_short[i]);
//    if(i%20==0&&i!=0)
//     printf("\n");

//     }

//     printf("\n===========================\n");

// }



if(counter%SHORT_BUF==0){
    printf("%d  ",counter);
//       for(int i=0;i<20;i++){
//  printf("%d,",buf_short[i]);

//     }

        printf("\n");


}

counter+=1;
    }

    printf("\n------------------------------------------------------\n");
    for(int i=0;i<2000;i++){
 printf("%-3d,",buf[i]);
   
    if(i%20==0&&i!=0)printf("\n");

    }





//   recv(newsock, buf, sizeof(buf), 0);
//    for(int i=0;i<1000;i++){
//     printf("%c,",buf[i]);
//     if(i%19==0&&i!=0){
//         printf("\n");
//     }
//     if(i%200==0&&i!=0){

//         printf("-------------------------------- seprate line\n");
//     }

//    }
//     printf("\n");
   Rgb565ConvertBmp(buf,640,480,"a.bmp");
 

__exit:
    closesocket(sock);
    closesocket(newsock);
    return ret;
}
