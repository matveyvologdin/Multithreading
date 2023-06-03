#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

using namespace std;

unsigned long long spGetCurrentTimeMS(void)
{
	struct timespec time;
	clock_gettime(CLOCK_REALTIME, &time);
	unsigned long long curTime = time.tv_nsec / 1000000;
	curTime += time.tv_sec * 1000;
	return curTime;
}

const uint8_t PHILOSOPHER_COUNT = 5;

unsigned long long startTimeMs = spGetCurrentTimeMS();
unsigned long long endTimeMs = 0;
unsigned long long sleepTimeMs = 0;

vector<uint8_t> callStack; // Стек вызовов номеров филосософов

const uint8_t philNumArray[PHILOSOPHER_COUNT] = {0, 1, 2, 3, 4}; // Массив номеров философов
pthread_t philThreadArray[PHILOSOPHER_COUNT];						 // Массив потоков философов
sem_t philSemArray[PHILOSOPHER_COUNT];						 // Массив семафоров философов

pthread_t servThread; // Поток слуги
sem_t servSem;  // Семафор слуги (на 2 потока)

int main(int, char **); // Главный поток

void init(int, char **); // Инициализация дополнительных потоков, заполнения стека вызовов
void deinit(void);		 // Деинициализация потоков, освобождение памяти

void *srvThreadExecution(void *);  // Функция потока слуги
void *philThreadExecution(void *); // Функция потока философа

void init(int argc, char **argv)
{
	char *pointerArgvEnd = NULL;
	endTimeMs = strtoull(argv[1], &pointerArgvEnd, 10) + startTimeMs;
	sleepTimeMs = strtoull(argv[2], &pointerArgvEnd, 10);
	pointerArgvEnd = NULL;

	uint16_t callCount = (endTimeMs - startTimeMs) / sleepTimeMs * 2;
	uint16_t callIterator = 0;
	for (uint8_t callStackElem = 0; callIterator < callCount; callStackElem = (callStackElem + 3) % PHILOSOPHER_COUNT, callIterator++)
		callStack.push_back(callStackElem);

	for (uint8_t i = 0; i < PHILOSOPHER_COUNT; i++)
		sem_init(&philSemArray[i], 1, 0);
	sem_init(&servSem, 2, 2);

	memset(&servThread, 0, sizeof(pthread_t));
	pthread_create(&servThread, 0, srvThreadExecution, 0);

	memset(philThreadArray, 0, PHILOSOPHER_COUNT * sizeof(pthread_t));
	for (uint8_t i = 0; i < PHILOSOPHER_COUNT; i++)
		pthread_create(philThreadArray + i, 0, philThreadExecution, (void *)(philNumArray + i));
}

void deinit(void)
{
	for (uint8_t i = 0; i < PHILOSOPHER_COUNT; i++)
		pthread_join(philThreadArray[i], 0);
	pthread_join(servThread, 0);

	for (uint8_t i = 0; i < PHILOSOPHER_COUNT; i++)
		sem_destroy(&philSemArray[i]);
	sem_destroy(&servSem);

	vector<uint8_t>().swap(callStack);
}

void *srvThreadExecution(void *threadParameter)
{
	while (spGetCurrentTimeMS() < endTimeMs)
	{
		uint8_t callPhilOne = callStack.front();
		callStack.front() = move(callStack.back());
		callStack.pop_back();
		uint8_t callPhilTwo = callStack.front();
		callStack.front() = move(callStack.back());
		callStack.pop_back();
		sem_wait(&servSem);
		sem_post(&philSemArray[callPhilOne]);
		sem_post(&philSemArray[callPhilTwo]);
		usleep(sleepTimeMs * 1000);
		sem_wait(&philSemArray[callPhilOne]);
		sem_wait(&philSemArray[callPhilTwo]);
	}
	for (uint8_t i = 0; i < PHILOSOPHER_COUNT; i++)
		sem_post(&philSemArray[i]);

	return 0;
}

void *philThreadExecution(void *threadParameter)
{
	uint8_t selectedPhil = *(uint8_t *)threadParameter;

	while (true)
	{
		printf("%llu:%u:E->T\n", spGetCurrentTimeMS() - startTimeMs, selectedPhil + 1);
		usleep(sleepTimeMs * 1000);

		sem_wait(&philSemArray[selectedPhil]);

		if (spGetCurrentTimeMS() >= endTimeMs)
		{
			sem_post(&servSem);
			sem_post(&philSemArray[selectedPhil]);
			return 0;
		}

		printf("%llu:%u:T->E\n", spGetCurrentTimeMS() - startTimeMs, selectedPhil + 1);
		usleep(sleepTimeMs * 1000);

		sem_post(&servSem);
		sem_post(&philSemArray[selectedPhil]);
	}
}

int main(int argc, char **argv)
{
	init(argc, argv);
	deinit();

	return 0;
}