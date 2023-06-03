#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>


volatile int number, threadVolume;

typedef struct
{
    volatile int numFirst;
    volatile int numSecond;
    volatile int cond;
    volatile int result;
    volatile int size;
    int* volatile workArray;
} num_status;

num_status * info;
pthread_t *threads;
pthread_mutex_t mutex;

void* partition(void* param);
int comp(const void* a, const void* b);
void readyForPart(void);
void checkNum(void);
void divNum(int index);
void init(void);
void out(double time);
int prePartition(int n);


int main(void)
{
    FILE* inputFile = fopen("input.txt", "r");
    fscanf(inputFile, "%d", &threadVolume);
    fscanf(inputFile, "%d", &number);
    fclose(inputFile);

    if (threadVolume > number || threadVolume == 1)
    {
        time_t t = clock();
        int result = prePartition(number);
        double timeSpent = 1000*((double)clock() - (double)t)/(double)CLOCKS_PER_SEC;
        FILE* outputFile = fopen("output.txt", "w+");
        fprintf(outputFile, "%d\n", threadVolume);
        fprintf(outputFile, "%d\n", number);
        fprintf(outputFile, "%d", result - 1);
        fclose(outputFile);

        FILE* timeFile = fopen("time.txt", "w+");
        fprintf(timeFile, "%d\n", (unsigned int)timeSpent);
        fclose(timeFile);

        return 0;
    }
    info = (num_status*)malloc(threadVolume * sizeof(num_status));
    threads = (pthread_t*)malloc(threadVolume * sizeof(pthread_t));
	readyForPart();
	checkNum();
    pthread_mutex_init(&mutex, 0);
    for (int i = 0; i < threadVolume; i++)
        pthread_create(&threads[i], 0, partition, (void *)((char *)0 + i));

    time_t t = clock();
    for (int i = 0; i < threadVolume; i++)
        pthread_join(threads[i], 0);
    pthread_mutex_destroy(&mutex);
    double time = 1000*((double)clock() - (double)t)/(double)CLOCKS_PER_SEC;
    out(time);

	for (int i = 0; i < threadVolume; i++)
        free(info[i].workArray);
	free(info);
    free(threads);

	return 0;
}

void readyForPart(void)
{
	for (int i = 0; i < threadVolume; i++)
	{
		info[i].numFirst = number - i * (int)(number / threadVolume);
		info[i].numSecond = i * (int)(number / threadVolume);
		if (i + 1 == threadVolume)
			info[i].cond = 0;
		else
			info[i].cond = number - (i + 1) * (int)(number / threadVolume);
	}
		
}

void checkNum(void)
{
	for (int j = 0; j < threadVolume; j++)
	{
		if (info[j].numFirst >= info[j].numSecond)
		{
			info[j].workArray = (int*)malloc(2 * sizeof(int));
			info[j].workArray[0] = info[j].numFirst;
			info[j].workArray[1] = info[j].numSecond;
			info[j].size = 2;
		}
		else
		{
			info[j].workArray = (int*)malloc(2 * sizeof(int));
			info[j].workArray[0] = info[j].numFirst;
			info[j].workArray[1] = info[j].numSecond;
			divNum(j);
		}
	}
}

void divNum(int index)
{
	int num_volume = 2;
	while(info[index].workArray[0] < info[index].workArray[1])
	{
		num_volume++;
		info[index].workArray = (int*)realloc(info[index].workArray, num_volume * sizeof(int));
		info[index].workArray[1] -= info[index].workArray[0];
		info[index].workArray[num_volume - 1] = info[index].workArray[0];
	}
	info[index].size = num_volume;
	qsort(info[index].workArray, num_volume, sizeof(int), comp);
}

int comp (const void* a, const void* b)
{ 
   return *(const int*)b - *(const int*)a;
}

void out(double time)
{
    FILE* inputFile = fopen("output.txt", "w+");
    int numPart = 0; 
    if (threadVolume <= number && threadVolume != 1)
    {
        fprintf(inputFile, "%d\n", threadVolume);
        fprintf(inputFile, "%d\n", number);

        pthread_mutex_lock(&mutex);
        for (int i = 0; i < threadVolume; i++)
            numPart += info[i].result;
        pthread_mutex_unlock(&mutex);

        fprintf(inputFile, "%d", numPart - threadVolume);
    }
    fclose(inputFile);

    inputFile = fopen("time.txt", "w+");
    fprintf(inputFile, "%d\n", (unsigned int)time);
    fclose(inputFile);
}

void* partition(void* param)
{
    int idx = ((char*)param - (char*)0), count = 0;  
    int * buffer = (int*)malloc(number * sizeof(int)); // An array to store a partition

	// Index of last element in a partition
	volatile int index = info[idx].size - 1;  // Index of last element in a partition
	if (info[idx].workArray[index] == 0)
		index--;

    for (int i = 0; i < info[idx].size; i++)
    	buffer[i] = info[idx].workArray[i];
    
    int rem_val;
    // This loop first prints current partition, then generates next
    // partition. The loop stops when the current partition has all 1s
    
    while (1)
    {
        count++;
        rem_val = 0;
        while (index >= 0 && buffer[index] == 1)
        {
            rem_val += buffer[index];
            index--;
        }
 
        // if k < 0, all the values are 1 so there are no more partitions
        if (buffer[0] == info[idx].cond || index < 0)
        {  
            info[idx].result = count;
            return 0;
        }
 
        // Decrease the buffer[index] found above and adjust the rem_val
        buffer[index]--;
        rem_val++;
 
 
        // If rem_val is more, then the sorted order is violated.  Divide
        // rem_val in different values of size buffer[index] and copy these values at
        // different positions after buffer[index]
        while (rem_val > buffer[index])
        {
            buffer[index+1] = buffer[index];
            rem_val = rem_val - buffer[index];
            index++;
        }
 
        // Copy rem_val to next position and increment position
        buffer[index+1] = rem_val;
        index++;
    }
}

int prePartition(int n)
{
    int *buffer = (int*)malloc(n*sizeof(int)); // An array to store a partition
    int k = 0, count = 0;  // Index of last element in a partition
    buffer[k] = n;  // Initialize first partition as number itself
    int rem_val;
    // This loop first prints current partition, then generates next
    // partition. The loop stops when the current partition has all 1s
    while (1)
    {
        // print current partition
		count++;
        rem_val = 0;
        while (k >= 0 && buffer[k] == 1)
        {
            rem_val += buffer[k];
            k--;
        }
 
        // if k < 0, all the values are 1 so there are no more partitions
		if (k < 0)
			return count;
	
        // Decrease the buffer[k] found above and adjust the rem_val
        buffer[k]--;
        rem_val++;
 
 
        // If rem_val is more, then the sorted order is violated.  Divide
        // rem_val in different values of size buffer[k] and copy these values at
        // different positions after buffer[k]
        while (rem_val > buffer[k])
        {
            buffer[k+1] = buffer[k];
            rem_val = rem_val - buffer[k];
            k++;
        }
 
        // Copy rem_val to next position and increment position
        buffer[k+1] = rem_val;
        k++;
    }
}
