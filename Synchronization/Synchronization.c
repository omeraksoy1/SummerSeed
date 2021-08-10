#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

pthread_t tid[2];
pthread_mutex_t lock;

int num;

void increment() {
	while (num < 99) {
		pthread_mutex_lock(&lock);
		num++;
		printf("Incremented to: %d\n", num);
		sleep(1);		
		pthread_mutex_unlock(&lock);
	}
	return;
}

int main() {
	FILE * fd;	
	fd = fopen("file.txt", "r");
	fscanf(fd, "%d", &num);
	printf("The number is: %d\n", num);
	fclose(fd);
	
	for (int i=0; i<2; i++) {
		pthread_create(&(tid[i]), NULL, increment, NULL);
	}
	
	for (int j=0; j<2; j++) {
		pthread_join(tid[j], NULL);
	}
	
	return 0;
}
