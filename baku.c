#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

int fileNumber;
int boxOpen = 0;

void play(){
	if(fileNumber == 0){//ファイル数が0なら再生しない
		return;
	}
	int playNumber;
	time_t nowTime = time(NULL);
	char path[63];
	nowTime = (int)nowTime;
	if((nowTime % 30) == 0){//30秒に一回再生
		playNumber = rand() % fileNumber + 1;
		sprintf(path, "/usr/bin/aplay ~/baku/%d.wav",playNumber);
		system(path);
		}
}

void record(){
}

int main(){
	FILE *fp;

	fp = fopen("baku_record","r+");
	if(fp == NULL){
		fp = fopen("baku_record","w+");
		fputc(0, fp);
	}
	fileNumber = fgetc(fp);
	printf("%d",fileNumber);
	while(1){
		if(boxOpen = 1){
			record();
		}
		play();
	}
}	



