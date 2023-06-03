#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <windows.h>

volatile int N = 0;
int length, threadsVolume, arrayVolume, * a;
CRITICAL_SECTION cs;
HANDLE threads[64];

void msort_ext(int left, int middle, int right)
{
	int n1 = middle - left + 1, n2 = right - middle, * L = new int[n1], * R = new int[n2], i = 0, j = 0, k = left;

	for (int i = 0; i < n1; i++)
	{
		L[i] = a[left + i];
	}

	for (int j = 0; j < n2; j++)
	{
		R[j] = a[middle + 1 + j];
	}

	i = 0;
	j = 0;

	while (i < n1 && j < n2)
	{
		if (L[i] <= R[j])
		{
			a[k] = L[i];
			i++;
		}
		else
		{
			a[k] = R[j];
			j++;
		}
		k++;
	}

	while (i < n1)
	{
		a[k] = L[i];
		i++;
		k++;
	}

	while (j < n2)
	{
		a[k] = R[j];
		j++;
		k++;
	}

	delete[]L;
	delete[]R;
}

void msort(int left, int right)
{
	if (left >= right)
	{
		return;
	}

	int middle = left + (right - left) / 2;

	msort(left, middle);
	msort(middle + 1, right);

	msort_ext(left, middle, right);
}

DWORD WINAPI thread(void* par)
{
	EnterCriticalSection(&cs);
	int left = N * length;
	N++;
	LeaveCriticalSection(&cs);

	int right = left + length;
	if (right > arrayVolume)
	{
		right = arrayVolume;
	}

	msort(left, right - 1);
	return 0;
}

int main(void)
{
	FILE* fileInput = fopen("input.txt", "r");
	fscanf(fileInput, "%d%d", &threadsVolume, &arrayVolume);
	a = new int[arrayVolume];

	for (int i = 0; i < arrayVolume; i++)
		fscanf(fileInput, "%d", &a[i]);
	fclose(fileInput);

	length = arrayVolume / threadsVolume + 1;
	int time = GetTickCount64();
	InitializeCriticalSection(&cs);
	memset(&threads, 0, sizeof(threads));

	for (int i = 0; i < threadsVolume; i++)
		threads[i] = CreateThread(0, 0, thread, (void*)((char*)0 + i), 0, 0);

	WaitForMultipleObjects(threadsVolume, threads, TRUE, INFINITE);
	DeleteCriticalSection(&cs);

	for (int i = 0; i < threadsVolume; i++)
		CloseHandle(threads[i]);

	time = GetTickCount64() - time;

	
	FILE* fileOutput = fopen("output.txt", "w");
	fprintf(fileOutput, "%d\n%d\n", threadsVolume, arrayVolume);

	for (int i = 0; i < arrayVolume; i++)
		fprintf(fileOutput, "%d ", a[i]);
	fclose(fileOutput);

	FILE* fileTime = fopen("time.txt", "w");
	fprintf(fileTime, "%d", time);
	fclose(fileTime);

	delete[] a;
	return 0;
}