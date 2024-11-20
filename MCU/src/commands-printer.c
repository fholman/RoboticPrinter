#include <stdio.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <dirent.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>



#define BAUDRATE B115200
//#define ARDUINOPORT "/dev/ttyUSB0"
#define ARDUINOPORT "/dev/ttyACM0"
#define FALSE 0
#define TRUE 1

#define PI 3.1415927

char FileName[1000] = "";


int fd = 0;
struct termios TermOpt;

int plotterSteps = 0;

char PicturePath[1000];


//+++++++++++++++++++++++ Start readport ++++++++++++++++++++++++++
char  readport(void){
  int n;
  char buff;
  n = read(fd, &buff, 1);
  if(n > 0){
    return buff;
  }
  return 0;
}
//------------------------ End readport ----------------------------------

//+++++++++++++++++++++++ Start sendport ++++++++++++++++++++++++++
void sendport(unsigned char ValueToSend){
  int n;

  n = write(fd, &ValueToSend, 1);

  if (n < 0){
    printf("write() of value failed!\r");
  }
  else{
    while(readport() != 'R');
  }

}
//------------------------ End sendport ----------------------------------

//+++++++++++++++++++++++ Start openport ++++++++++++++++++++++++++
void openport(void){

    fd = open(ARDUINOPORT, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1)  {
      printf("init_serialport: Unable to open port ");
    }

    if (tcgetattr(fd, &TermOpt) < 0) {
      printf("init_serialport: Couldn't get term attributes");
    }
    speed_t brate = BAUDRATE; // let you override switch below if needed

    cfsetispeed(&TermOpt, brate);
    cfsetospeed(&TermOpt, brate);

    // 8N1
    TermOpt.c_cflag &= ~PARENB;
    TermOpt.c_cflag &= ~CSTOPB;
    TermOpt.c_cflag &= ~CSIZE;
    TermOpt.c_cflag |= CS8;
    // no flow control
    TermOpt.c_cflag &= ~CRTSCTS;

    TermOpt.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
    TermOpt.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

    TermOpt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    TermOpt.c_oflag &= ~OPOST; // make raw

    // see: http://unixwiz.net/techtips/termios-vmin-vtime.html
    TermOpt.c_cc[VMIN]  = 0;
    TermOpt.c_cc[VTIME] = 20;

    if( tcsetattr(fd, TCSANOW, &TermOpt) < 0) {
      printf("init_serialport: Couldn't set term attributes");
    }

}
//------------------------ End openport ----------------------------------



//######################################################################
//################## Main ##############################################
//######################################################################

int main(int argc, char **argv){

  char FullFileName[1000] = "";
  char *pEnd;
  FILE *bmpFile;
  char TextLine[30000];
  char a;

  char FileInfo[3];
  long FileSize;
  long LongTemp;
  long DataOffset;
  long HeaderSize;
  long PictureWidth;
  long PictureHeight;
  int  IntTemp;
  int  ColorDepth;
  long CompressionType;
  long PictureSize;
  long XPixelPerMeter;
  long YPixelPerMeter;
  long ColorNumber;
  long ColorUsed;
  int PixelRed, PixelGreen, PixelBlue;
  int PixOK=0;
  long BrightestX=0, BrightestY=0;
  int BrightR = 0, BrightG = 0, BrightB = 0;
  int ServoX = 150, ServoY = 150;
  long CurrentPicNo = 1, LastPicNo = 1;

  unsigned char bitmapData[24][10000];

  unsigned char dataToSend1, dataToSend2;

  long PictureWidthFilled = 0;

  openport();

  printf("\n\nWaiting for 'X' from Arduino (Arduino pluged in?)...\n");


  //Wait for 'X' from Arduino
  while(readport() != 'X');
  printf("Connection to Arduino established.\n");


  strcpy(FileName, "tux-caveman.bmp");

  getcwd(PicturePath, 1000);
  strcat(PicturePath, "/pictures");
  printf("PicturePath=>%s<", PicturePath);


  if((bmpFile=fopen(FileName,"rb"))==NULL){
    printf("Datei '%s' kann nicht zum Lesen geoeffnet werden!\n", FileName);
    return(1);
  }

  fread(&FileInfo, 2, 1, bmpFile);
  fread(&FileSize, 4, 1, bmpFile);
  fread(&LongTemp, 4, 1, bmpFile);
  fread(&DataOffset, 4, 1, bmpFile);
  fread(&HeaderSize, 4, 1, bmpFile);
  fread(&PictureWidth, 4, 1, bmpFile);
  fread(&PictureHeight, 4, 1, bmpFile);
  fread(&IntTemp, 2, 1, bmpFile);
  fread(&ColorDepth, 2, 1, bmpFile);
  fread(&CompressionType, 4, 1, bmpFile);
  fread(&PictureSize, 4, 1, bmpFile);
  fread(&XPixelPerMeter, 4, 1, bmpFile);
  fread(&YPixelPerMeter, 4, 1, bmpFile);
  fread(&ColorNumber, 4, 1, bmpFile);
  fread(&ColorUsed, 4, 1, bmpFile);

  printf("FileInfo=%s\n", FileInfo);
  printf("FileSize=%ld\n", FileSize);
  printf("LongTemp=%ld\n", LongTemp);
  printf("DataOffset=%ld\n", DataOffset);
  printf("HeaderSize=%ld\n", HeaderSize);
  printf("PictureWidth=%ld\n", PictureWidth);
  printf("PictureHeight=%ld\n", PictureHeight);
  printf("IntTemp=%d\n", IntTemp);
  printf("ColorDepth=%d\n", ColorDepth);
  printf("CompressionType=%ld\n", CompressionType);
  printf("PictureSize=%ld\n", PictureSize);
  printf("XPixelPerMeter=%ld\n", XPixelPerMeter);
  printf("YPixelPerMeter=%ld\n", YPixelPerMeter);
  printf("ColorNumber=%ld\n", ColorNumber);
  printf("ColorUsed=%ld\n", ColorUsed);


  if(FileInfo[0] != 'B' || FileInfo[1] != 'M'){
    printf("Wrong Fileinfo (BM)!\n");
    return(1);
  }
  if(ColorDepth != 24){
    printf("Wrong ColorDepth (24)!\n");
    return(1);
  }
  if(CompressionType != 0){
    printf("Wrong CompressionType (0)!\n");
    return(1);
  }


  fseek(bmpFile, DataOffset, SEEK_SET);

  PictureWidthFilled = PictureWidth*3;
  while(PictureWidthFilled%4 !=0){
    PictureWidthFilled++;
  }

  printf("PictureWidthFilled = %ld, PictureWidth = %ld\n", PictureWidthFilled, PictureWidth);


  while(!feof(bmpFile)){

     // Reset buffer to 255 (White)
    for(int i = 0; i < 24; i++){
      for(int j = 0; j < 10000; j++){
        bitmapData[i][j]=255;
      }
    }
    // Read 24 lines into buffer
    for(long j = 0; j < 24; j++){
      if(!feof(bmpFile)){
        fread(bitmapData[j], 1, PictureWidthFilled, bmpFile);
      }
      printf("%d\n", 1<<j);
    }

    for(long k = 0; k < 2; k++){
      for(long i = 0; i < PictureWidth * 3; i+=3){
        dataToSend1 = 0;
        dataToSend2 = 0;
        for(long j = 0; j < 8; j++){
          if(bitmapData[j*2+k][i]<127 && bitmapData[j*2+k][i+1]<127 && bitmapData[j*2+k][i+2]<127){// Black dot
            dataToSend1 += 1<<j;
            //printf(" %d\n", dataToSend1);
            //printf("X");
          }
          else{
            //printf(" ");
          }
        }
        //printf(" %d\n", dataToSend1);
        for(long j = 8; j < 12; j++){
          if(bitmapData[j*2+k][i]<127 && bitmapData[j*2+k][i+1]<127 && bitmapData[j*2+k][i+2]<127){// Black dot
            dataToSend2 += 1<<(j-8);
          }
        }
/*
        for(long j = 0; j < 8; j++){
          if((dataToSend1 & (1<<j)) != 0){
            printf("X");
          }
          else{
            printf(" ");
          }
        }
        for(long j = 0; j < 8; j++){
          if(dataToSend2 & (1<<j)){
            printf("X");
          }
          else{
            printf(" ");
          }
        }
        printf(" %d %d\n", dataToSend1, dataToSend2);
*/
//        if(k==1){
          sendport('d');
          sendport(dataToSend1);
          sendport(dataToSend2);
//        }
      }
      if(k==0){
        sendport('r');  // Return carriage
        sendport('N');  // Forward paper one dot
      }
      else{
        sendport('r');  // Return carriage
        sendport('n');  // New line
      }
    }

  }





  fclose(bmpFile);


  return 0;
}