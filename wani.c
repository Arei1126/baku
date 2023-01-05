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

#define BT_OR_HUG 1  // 1でボタン、0でハグ
#define WEIT 10  //  録音秒数

//int boxOpen = NO_ACTION;//スイッチのこと１が開いてる、０が閉じてる
FILE *fp;//録音の一覧用のファイルポインタ
FILE *fp2;

#バリエーションのためのカウンタ
char Calling = 0;
char Greeting = 0;
char Explaination = 0;
char Rec = 0;
char Reaction = 0;

//  カウンタが循環するようにする
#define CHK_MAX(counter, max)	((counter) > (max) ? 0 : (counter))

#define CALLING_MAX 4
#define GREETING_MAX 6
#define EXPLAINATION_MAX 3
#define REC_MAX 2
#define REACTION_MAX 4



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
	pid_t pid;

	//printf("entering playing func\n");
	if(fileNumber == 0){//ファイル数が0なら再生しない
		printf("No playable files");
		return;
	}
	int playNumber;
	time_t nowTime = time(NULL);
	char path[1024];
	char call[1024];
	nowTime = (int)nowTime;
	if((nowTime % INTERVAL) == 0){ //30秒に一回再生
		playNumber = rand() % fileNumber + 1;  // ランダムに一つ選ぶ

		//  呼びかけ用の部分
		sprintf(call, "/usr/bin/splay call_%d.wav", Calling);
		Calling++;  // 
		CHK_MAX(Calling, CALLING_MAX);  

		sprintf(path, "/usr/bin/aplay %d.wav",playNumber);  // 再生用のコマンド


	//	printf("Play:%d\n",playNumber);
	//	system(path);
	
		pid = fork();
		switch(pid) {
			case -1:
				exit(-1);
			case 0://In Child Process
				system(call);  //  よびかけ再生
				system(path);  //  録音した音声を再生
			default://In Parent Process
				while (1) {  //  再生中に何かるかどうか待つ。中断するため
					if(kill(pid, 0) != 0)  // 存在しなければ抜ける
						break;
					if(chkAction() == ACTION)  // アクションがあったら子を殺す
						kill(pid, SIGKILL);
				}


		}
	return;
	}
}

void record(int fileNumber){
	printf("entering recording function\n");

//録音前の音
	
//	system("/usr/bin/aplay /home/naohiro/baku/baku/soundeffect/recStartZundaOhako.wav");

	pid_t pid;
	char path[1024];
	char effectPath[1024];
	char playPath[1024];

	char greeting[1024];
	char explaination[1024];
	char recStart[1024];
	char recStop[1024];
	char reaction[1024];
	
	//ここから反応のバリエーションを増やすための処理が入る 
	sprintf(greeting, "/usr/bin/aplay greet_%d.wav",Greeting);  //  あいさつ
	Greeting++;
	CHK_MAX(Greeing, GREETING_MAX);

	sprintf(recStart, "/usr/bin/aplay recStart_%d.wav",Rec);
	sprintf(recStop, "/usr/bin/aplay recStop_%d.wav",Rec);
	Rec++;
	CHK_MAX(Rec, REC_MAX);

	sprintf(explaination, "/usr/bin/aplay explain_%d.wav",Explaination);
	Explaination++;
	CHK_MAX(Explaination, EXPLAINATION_MAX);

	sprintf(reaction, "/usr/bin/aplay react_%d.wav",Reaction);
	Reaction++;
	CHK_MAX(Reaction, REACTION_MAX);
	////////////////////////////////////////////////////////
	system(greeting);  // あいさつ
	system(explaination);  // 説明
	system(recStart);  // 録音開始の合図

	sprintf(path, "%d_original.wav",fileNumber);//録音用のファイル名
	sprintf(effectPath, "/usr/bin/sox %d_original.wav %d.wav silence 1 5 10%% norm pitch 600",fileNumber,fileNumber);//Converting Strings for Muon
	//printf("Effect path is %s\n",effectPath);
	//sprintf(effectPath, "/usr/bin/sox %d_original.wav %d.wav norm pitch 250 speed 1.25",fileNumber,fileNumber);//こっちは変換用の文字列
	sprintf(playPath, "/usr/bin/aplay %d.wav", fileNumber );
	//printf("play path is %s\n",playPath);
	//printf("path is %s\n",path);
	pid = fork();
	switch(pid) {
		case -1:
			exit(1);
		case 0://子プロセスで録音
			//execl("/usr/bin/arecord", "arecord","-D","plughw:1,0","-f","cd",path,NULL);//子プロセスで録音
			//printf("In the Chiled what is path : %s\n",path);
			execl("/usr/bin/arecord", "arecord", "-D", "plughw:CARD=UCAMDLK130T,DEV=0", "-f", "cd",path,NULL);//子プロセスで録音
			
			
			printf("%s in child process \n",path);
			//execl("/usr/bin/arecord", "arecord","-f","cd",path,NULL);//子プロセスで録音
		default: //こっちが親プロセス	
			printf("recording PID:%d\n",pid);

			//while(boxOpen() == ACTION);//閉じてから動き出す
			
			if(BT_OR_HUG == 1)
			{
				recordWait((time_t)WEIT);//時間経過で抜ける
			}
			else
			{
				while(chkAction() == ACTION); //持ち上げている間はループして待つ
			}

			printf("Killing record\n");
			kill(pid, SIGINT);//録音するプロセスを殺して
			printf("Recording finished\n");
			
			//system("/usr/bin/aplay /home/naohiro/baku/baku/soundeffect/recStop.wav");
			system(recStop);  // 録音終了の合図
			system(reaction);  // ちょっとした反応を見せる
			printf("Starting Effect\n");
			system(effectPath);//変換する
			system(playPath);//Oumu Gaeshi
			
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



