#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

int boxOpen = 0;//スイッチのこと１が開いてる、０が閉じてる

void play(int fileNumber){
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
	return;
}

void record(int fileNumber){
	pid_t pid;
	char path[63];
	char effectPath[63];
	sprintf(path, "~/baku/%d.wav",fileNumber);//録音用のファイル名
	pid = fork();
	sprintf(effectPath, "/usr/bin/sox ~/baku/%d.wav ~/baku/%d.wav pitch 400 speed 1.5",fileNumber,fileNumber);//こっちは変換用の文字列
	switch(pid) {
		case -1:
			exit(1);
		case 0:
			while(boxOpen == 1);//閉じてから動き出す
			kill(pid, SIGINT);//録音するプロセスを殺して
			system(effectPath);//変換する
		default :
			execl("/usr/bin/arecord", "arecord","-D","plughw:2,0","-f","cd",path,NULL);//子プロセスで録音
	}
	return;

}

int main(){
	int fileNumber;//これが扱うファイル名になる
	FILE *fp;

	fp = fopen("baku_record","r+");
	if(fp == NULL){
		fp = fopen("baku_record","w+");
		fputc(0, fp);
	}
	fileNumber = fgetc(fp);
	printf("%d",fileNumber);
	while(1){
		if(boxOpen == 1){
			fileNumber++;
			record(fileNumber);
		}
		play(fileNumber);
	}
}	



