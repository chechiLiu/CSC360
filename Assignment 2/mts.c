#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

//This program is a simulator of an automated control system for the railway track shown in PDF.
//To emulate the scheduling of multiple threads sharing a common resource in a real operating system.

typedef struct train{
	int id;
	char dir;
	char *direction;
	int load;
	int cross;
	int finished;  //finished loading
	int go;        //allowed to use track?	
	int position;  //position in the readyQueue	      		
}train;

struct timeval tv;

int startloading = 0; //start loading 
int used = 0; //track being used?
int readyque = 0; //length of the readyQue
int eorw = 0; //east or west of the main track

train trains[1024];
train* ready[1024];

pthread_cond_t con;
pthread_cond_t conload;
pthread_cond_t conque;

pthread_mutex_t track;
pthread_mutex_t que;

//pick less loading time from train, if the same loading time pick the less ID.
/*int sort(const void* one, const void* two) {
	int first = ((struct train *)one)->load;
	int second = ((struct train *)two)->load;
	if(first == second) {
		int f = ((struct train *)one)->id;
		int s = ((struct train *)two)->id;
		return (f-s);
	}
	return (first - second);
}*/

double gettime() {
	struct timeval end;

   	gettimeofday(&end, NULL);

   	double time = (end.tv_sec + ((double)(end.tv_usec) / 1000000)) - (tv.tv_sec + ((double)(tv.tv_usec) / 1000000));
	return time;
}

void removeQ(int u) {
	while(u < readyque - 1) {
		ready[u] = ready[u+1];
		u += 1;
	}
	readyque -= 1;
}

void insertQ(train* t) {
	ready[readyque] = t;
	readyque += 1;
}

/*int find() {
	int t;
	int numh = 0;
	int numl = 0;
	int pos;
	for(t=0; t<readyque; t++) {
		if(ready[t]->dir == 'E' || ready[t]->dir == 'W') {
			pos = t;
			numh += 1;
		}
	}	
	if(numh == 1) {
		return pos;
	}else{	
		return 0; 
	}
}*/

void* trainsThread(void* arg) {
	train* t = (train* ) arg;

	//waiting for load
	pthread_mutex_lock(&que);
	while(startloading != 1) {
		pthread_cond_wait(&conload, &que);
	}
	pthread_mutex_unlock(&que);

	//crossing
	usleep(t->load * 100000);

	//inserting into readyQue
	pthread_mutex_lock(&que);
	insertQ(t);
	pthread_cond_signal(&conque);
printf("%02d:%02d:%04.1f ", (int)(gettime()/120), (int)(gettime()/60), gettime());
printf("Train %d is ready to go %4s\n", t->id, t->direction);
	pthread_mutex_unlock(&que);

	//waiting to cross
	pthread_mutex_lock(&track);
	while(t->go != 1) {
		pthread_cond_wait(&con, &track);
	}
	//Crossing
	used += 1;
printf("%02d:%02d:%04.1f ", (int)(gettime()/120), (int)(gettime()/60), gettime());
printf("Train %d is ON the main track going %4s\n", t->id, t->direction);
	usleep(t->cross * 100000);
	used -= 1;
	//done crossing
printf("%02d:%02d:%04.1f ", (int)(gettime()/120), (int)(gettime()/60), gettime());
printf("Train %d is OFF the main track after going %4s\n", t->id, t->direction);
	pthread_cond_signal(&con);
	pthread_mutex_unlock(&track);

	pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
	if(argc < 2) {
		printf("Please enter the input filename\n");
		return 1;	
	}

	FILE *fp;
	fp = fopen(argv[1], "r");
	if(fp == NULL) {
		printf("Error opening the file!!!\n");
		return 1;	
	}

int i= 0;	
char a;
int l, c;
int g = 0;
char *e = "East";
char *w = "West";

	//reading input
	while (fscanf(fp, "%c %d %d ", &a, &l, &c) == 3) {
		if(a == 'e' || a == 'E') {
			train t = {
				i,
				a,
				e,
				l,
				c,
				g,
				g,
				g	
			};
			trains[i] = t;
			i++;
		}else {
			train t = {
				i,
				a,
				w,
				l,
				c,
				g,
				g,
				g	
			};
			trains[i] = t;
			i++;
		}
	}

pthread_t threads[i];

pthread_attr_t attr;
pthread_attr_init(&attr);

pthread_cond_init(&con, NULL);
pthread_cond_init(&conload, NULL);
pthread_cond_init(&conque, NULL);

pthread_mutex_init(&track, NULL);
pthread_mutex_init(&que, NULL);

	int k;
	for(k=0; k<i; k++) {
		if(pthread_create(&threads[k], &attr, trainsThread, (void*)&trains[k]) != 0) {
				printf("Error when creating threads!!!\n");
				return 1;	
		}
		usleep(1);
	}

	//start loading
	pthread_mutex_lock(&que);
	startloading += 1;
	pthread_cond_broadcast(&conload);
	pthread_mutex_unlock(&que);

	gettimeofday(&tv, NULL);

	//wait for readyQue to not be empty
	pthread_mutex_lock(&track);
	while (readyque == 0) {
		pthread_cond_wait(&conque, &track);
	}

	//dispatch next train to cross
	while(i>0) {
		ready[0]->go += 1;
		i -= 1;

		pthread_cond_broadcast(&con);
		while(used != 0) {
			pthread_cond_wait(&con, &track);
		}
		removeQ(0);
		if (readyque == 0) {
			pthread_cond_wait(&conque, &track);
		}
		if(readyque == 0 && i == 0) {
			break;
		}
	}
	pthread_mutex_unlock(&track);
	
	for(k=0; k<i; k++) {
		if(pthread_join(threads[k], NULL) != 0) {
			printf("Error when joining threads");
			return 1;
		}	
	}

pthread_attr_destroy (&attr);
pthread_cond_destroy (&con);
pthread_cond_destroy (&conload);
pthread_cond_destroy (&conque);
pthread_mutex_destroy (&track);
pthread_mutex_destroy (&que);
}
