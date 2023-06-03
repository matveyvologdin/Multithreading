#define _CRT_SECURE_NO_WARNINGS

#include <cstdint>
#include <vector>
#include <windows.h>
#include <stdio.h>

using namespace std;

struct BORDER
{
	uint32_t lowIndex;
	uint32_t highIndex;
};

vector <struct BORDER> borderBuffer;

uint32_t POOL_SIZE = 0;
uint32_t ARRAY_SIZE = 0;

HANDLE mutex = 0;

int32_t* workArray = NULL;
HANDLE execThreads[MAXIMUM_WAIT_OBJECTS];
uint8_t execThreadsNumbers[MAXIMUM_WAIT_OBJECTS];

void init(void);
void deinit(void);
DWORD WINAPI thread(void*);
void partition(uint32_t, uint32_t);
uint32_t qsort(uint32_t, uint32_t);
int main(void);

void init(void)
{
	mutex = CreateMutex(NULL, FALSE, NULL);
	ReleaseMutex(mutex);

	struct BORDER borderStackElem = { 0, ARRAY_SIZE - 1 };
	borderBuffer.push_back(borderStackElem);

	memset(&execThreads, 0, sizeof(execThreads));
	for (uint8_t i = 0; i < POOL_SIZE; i++)
	{
		execThreads[i] = CreateThread(0, 0, thread, execThreadsNumbers + i, 0, 0);
	}
}

void deinit(void)
{
	CloseHandle(mutex);

	for (uint8_t i = 0; i < POOL_SIZE; i++)
	{
		CloseHandle(execThreads[i]);
	}
}

DWORD WINAPI thread(void* processParameter)
{
	while (true)
	{
		WaitForSingleObject(mutex, INFINITE);
		if (borderBuffer.size() == 0)
		{
			ReleaseMutex(mutex);
			return *(uint8_t*)processParameter;
		}
		struct BORDER borderStackElem = borderBuffer.back();
		borderBuffer.pop_back();
		if (borderBuffer.size() != 0)
		{
			ReleaseMutex(mutex);
		}
		partition(borderStackElem.lowIndex, borderStackElem.highIndex);
		if (borderBuffer.size() != 0)
		{
			ReleaseMutex(mutex);
		}
	}
}

void partition(uint32_t lowIndex, uint32_t highIndex)
{
	if (lowIndex >= highIndex)
		return;

	uint32_t middleIndex = qsort(lowIndex, highIndex);

	if (middleIndex - lowIndex > 4096)
	{
		struct BORDER borderStackElem = { lowIndex, middleIndex };
		borderBuffer.push_back(borderStackElem);
	}
	else
	{
		partition(lowIndex, middleIndex);
	}

	if (highIndex - middleIndex > 4096)
	{
		struct BORDER borderStackElem = { middleIndex + 1, highIndex };
		borderBuffer.push_back(borderStackElem);
	}
	else
	{
		partition(middleIndex + 1, highIndex);
	}
}

uint32_t qsort(uint32_t lowBorder, uint32_t highBorder)
{
	int32_t pivot = workArray[lowBorder];
	uint32_t lowIndex = lowBorder - 1;
	uint32_t highIndex = highBorder + 1;

	while (true)
	{
		while (++lowIndex < highBorder && workArray[lowIndex] < pivot);
		while (--highIndex > lowBorder && workArray[highIndex] > pivot);

		if (lowIndex >= highIndex)
		{
			return highIndex;
		}

		int32_t tempValue = workArray[lowIndex];
		workArray[lowIndex] = workArray[highIndex];
		workArray[highIndex] = tempValue;
	}
}

int main(void)
{
	FILE* inputFile = fopen("input.txt", "r");
	fscanf(inputFile, "%lu%lu", &POOL_SIZE, &ARRAY_SIZE);
	workArray = new int32_t[ARRAY_SIZE];

	for (uint32_t i = 0; i < ARRAY_SIZE; i++)
		fscanf(inputFile, "%ld", workArray + i);

	fclose(inputFile);
	inputFile = NULL;

	for (uint8_t i = 0; i < POOL_SIZE; i++)
		execThreadsNumbers[i] = i;

	init();
	uint64_t timeElapsed = GetTickCount64();
	WaitForMultipleObjects(POOL_SIZE, execThreads, TRUE, INFINITE);
	timeElapsed = GetTickCount64() - timeElapsed;
	deinit();

	FILE* outputFile = fopen("output.txt", "w");
	fprintf(outputFile, "%lu\n%lu\n", POOL_SIZE, ARRAY_SIZE);

	for (uint32_t i = 0; i < ARRAY_SIZE; i++)
		fprintf(outputFile, "%ld ", workArray[i]);

	fclose(outputFile);
	outputFile = NULL;
	delete[]workArray;

	FILE* timeFile = fopen("time.txt", "w");
	fprintf(timeFile, "%llu", timeElapsed);
	fclose(timeFile);
	timeFile = NULL;

	return 0;
}