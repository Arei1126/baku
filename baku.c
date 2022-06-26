#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

int boxOpen = 1;//スイッチのこと１が開いてる、０が閉じてる
FILE *fp;//録音の一覧用のファイルポインタ

void play(int fileNumber){
	printf("entering playing func\n");
	if(fileNumber == 0){//ファイル数が0なら再生しない
		printf("No playable files");
		return;
	}
	int playNumber;
	time_t nowTime = time(NULL);
	char path[63];
	nowTime = (int)nowTime;
	if((nowTime % 3) == 0){ //30秒に一回再生
		playNumber = rand() % fileNumber + 1;
		sprintf(path, "/usr/bin/aplay %d.wav",playNumber);
		printf("Play:%d\n",playNumber);
		system(path);
		}
	return;
}

void record(int fileNumber){
	printf("entering recording function\n");
	pid_t pid;
	char path[63];
	char effectPath[63];
	sprintf(path, "%d.wav",fileNumber);//録音用のファイル名
	pid = fork();
	sprintf(effectPath, "/usr/bin/sox %d.wav %d.wav pitch 400 speed 1.5",fileNumber,fileNumber);//こっちは変換用の文字列
	switch(pid) {
		case -1:
			exit(1);
		case 0://子プロセスで録音
			execl("/usr/bin/arecord", "arecord","-D","plughw:2,0","-f","cd",path,NULL);//子プロセスで録音
		default: //こっちが親プロセス	
			printf("recording PID:%d\n",pid);
			for(int i = 0;i < 5000;i++){}
			boxOpen = 0;
			while(boxOpen == 1);//閉じてから動き出す
			printf("Killing record\n");
			kill(pid, SIGINT);//録音するプロセスを殺して
			printf("Recording finished\n");
			system(effectPath);//変換する
			fseek(fp,0,SEEK_SET);
			fputc(fileNumber,fp);//インデックス用のファイルに書き込む
	}
	return;

}

int main(){
	int fileNumber;//これが扱うファイル名になる

	fp = fopen("baku_record","r+");
	if(fp == NULL){
		fp = fopen("baku_record","w+");
		fputc(0, fp);
		printf("New index file created");
	}
	fileNumber = fgetc(fp);
	printf("Initial fileNum:%d\n",fileNumber);
	while(1){
		printf("Loop Start:filenumber:%d\n",fileNumber);
		if(boxOpen == 1){
			fileNumber++;
			record(fileNumber);
		}
		play(fileNumber);
		//printf("Loop End\n");
	}
}	



