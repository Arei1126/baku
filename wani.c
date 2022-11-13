#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pigpio.h>

//ACTION ボタンを押すなり、何らかのアクションがあったことを表す
#define BOTTON_NO 21
#define ACTION 1
#define NO_ACTION 0

//EMERGENCY STOPボタン、赤い方で、押し込んで停止(断絶)。フェイルセーフで断線しても止まるようになってる
#define STOP_NO 16
#define STOP 1
#define NO_STOP 0

//再生時のインターバル(秒)
#define INTERVAL 30 

//int boxOpen = NO_ACTION;//スイッチのこと１が開いてる、０が閉じてる
FILE *fp;//録音の一覧用のファイルポインタ
FILE *fp2;

//指定秒数だけ止まる関数
int recordWait(time_t duration) {
	time_t startTime = time(NULL);
	
	for(time_t currTime = time(NULL); (currTime - startTime ) <= duration; currTime = time(NULL)) {
		//printf("waiting\n");
	}
	printf("ok\n");
	return 0;
} 

//ボタンが押されているかどうか、押されていれば0(ACTION)が、押されていなければ1(NO_ACTION)が返る、だったけども
int boxOpen(){
	return gpioRead(BOTTON_NO);
}

//ワニが床から離れていればACTIONで、離れていなければNO_ACTIONにしようかな
int chkAction(){
	return gpioRead(BOTTON_NO);
}

//STOPボタンが押されていればSTOP(1),押されていなければNO_STOP(0),ボタンがNormalCloseなことに注意
int chkStop(){
	return gpioRead(STOP_NO);
}

void play(int fileNumber){
	//printf("entering playing func\n");
	if(fileNumber == 0){//ファイル数が0なら再生しない
		printf("No playable files");
		return;
	}
	int playNumber;
	time_t nowTime = time(NULL);
	char path[63];
	nowTime = (int)nowTime;
	if((nowTime % INTERVAL) == 0){ //30秒に一回再生
		playNumber = rand() % fileNumber + 1;
		sprintf(path, "/usr/bin/aplay %d.wav",playNumber);
		printf("Play:%d\n",playNumber);
		system(path);
		}
	return;
}

void record(int fileNumber){
	printf("entering recording function\n");

//録音前の音
	system("/usr/bin/aplay /home/naohiro/baku/baku/soundeffect/recStartZunda.wav");

	pid_t pid;
	char path[63];
	char effectPath[63];
	sprintf(path, "%d_original.wav",fileNumber);//録音用のファイル名
	sprintf(effectPath, "/usr/bin/sox %d_original.wav %d.wav norm pitch 250 speed 1.5",fileNumber,fileNumber);//こっちは変換用の文字列
	pid = fork();
	switch(pid) {
		case -1:
			exit(1);
		case 0://子プロセスで録音
			execl("/usr/bin/arecord", "arecord","-D","plughw:1,0","-f","cd",path,NULL);//子プロセスで録音
			
			 // execl("/usr/bin/arecord", "arecord","-f","cd",path,NULL);//子プロセスで録音
		default: //こっちが親プロセス	
			printf("recording PID:%d\n",pid);

			//while(boxOpen() == ACTION);//閉じてから動き出す
			//recordWait((time_t)10);//時間経過で抜ける
			while(chkAction() == ACTION); //持ち上げている間はループして待つ

			printf("Killing record\n");
			kill(pid, SIGINT);//録音するプロセスを殺して
			printf("Recording finished\n");
			system("/usr/bin/aplay /home/naohiro/baku/baku/soundeffect/recEndZunda.wav");
			system(effectPath);//変換する

			fseek(fp,0,SEEK_SET);
			fprintf(fp,"%d",fileNumber);
	}
	//録音の後
	return;

}

int main(){
	//gpio関連初期化
	if (gpioInitialise() < 0) exit(1);
	gpioSetMode(BOTTON_NO,PI_INPUT);
	gpioSetPullUpDown(BOTTON_NO,PI_PUD_UP);

	gpioSetMode(STOP_NO,PI_INPUT);
	gpioSetPullUpDown(STOP_NO,PI_PUD_UP);


	int fileNumber;//これが扱うファイル名になる

	fp = fopen("baku_record.txt","r");
	if(fp == NULL){
		exit(1);
		/*
		fp2 = fopen("baku_record.txt","w");
		fprintf(fp2,"%d",0);
		printf("New index file created\n");
		fclose(fp2);
		*/
	}
	else{
		fclose(fp);
	}
	while(1){

		if(chkStop() == NO_STOP)
		{

			fp2 = fopen("baku_record","r+");
			fseek(fp,0,SEEK_SET);
			fscanf(fp,"%d",&fileNumber);
			//printf("Loop Start:filenumber:%d\n",fileNumber);
			//printf("%d",boxOpen());

			//ワニが床から離れて、ボタンが押されていない、入力が1、かつ、緊急停止が押されていない、非常入力が0の時に動作
			if(chkAction() == ACTION){
				fileNumber++;
				record(fileNumber);
			}
			//ここがランダム再生、発表会用に無音
			play(fileNumber);

			fclose(fp2);
		}
	}
}	



