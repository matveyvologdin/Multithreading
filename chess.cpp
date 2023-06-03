#include <pthread.h>
#include <semaphore.h> 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const char* fileIn = "input.txt";
const char* fileOut = "output.txt";
const char* fileTime = "time.txt";

FILE* f;

int threadsCount, N, L, K, result;
int** Board, * inputArrI, * inputArrJ;
pthread_t* threads;
pthread_mutex_t mutex;

int error(const char* err);
int get_args(void);
void every_struct_init(void);
void every_struct_deinit(void);
void create_threads(void);
void* thread_entry(void* param);
int SetMaharajaOnBoard(int* value, int row, int col, int countFiguresSet);
bool position_check(int row, int col);
void time(void);

int main(void) {
	if (get_args())
		return error("fopen failed.");

	create_threads();
	time();
	every_struct_deinit();

	return 0;
}

int error(const char* err) {
	printf("ERROR: %s", err);
	return -1;
}

int get_args(void)
{
	int i = 0;
	char line1[20], line_buf[20];
	if ((f = fopen(fileIn, "rb")) == NULL)
		return 1;

	fscanf(f, "%d", &threadsCount);
	fscanf(f, "%d %d %d", &N, &L, &K);

	every_struct_init();

	while (i < K) {
		fscanf(f, "%d %d", &inputArrI[i], &inputArrJ[i]);
		i++;
	}

	fclose(f);

	return 0;
}

void every_struct_init(void)
{
	int i, j;
	result = 0;
	inputArrI = (int*)malloc(sizeof(int) * K);
	inputArrJ = (int*)malloc(sizeof(int) * K);
	threads = (pthread_t*)malloc(sizeof(pthread_t) * threadsCount);

	pthread_mutex_init(&mutex, 0);

	Board = (int**)malloc(sizeof(int*) * N);
	for (i = 0; i < N; i++) {
		Board[i] = (int*)malloc(sizeof(int) * N);
		for (j = 0; j < N; j++) {
			Board[i][j] = 0;
		}
	}
}

void every_struct_deinit(void)
{
	pthread_mutex_destroy(&mutex);
	free(threads);
	free(inputArrI);
	free(inputArrJ);

	for (int i = 0; i < N; i++)
		free(Board[i]);
	free(Board);
}

void create_threads(void)
{
	int ind = 0, status;
	while (ind < threadsCount)
	{
		status = pthread_create(&threads[ind], 0, thread_entry, (void*)(ind));
		if (status != 0)
			exit(printf("\nthread_creation ERROR!\n"));
		ind++;
	}
}

void* thread_entry(void* param) {
	int index, value = 0;

	index = (int)((long)param);

	pthread_mutex_lock(&mutex);
	SetMaharajaOnBoard(&value, 0, 0, 0);
	pthread_mutex_unlock(&mutex);

	if (index == 0) {
		pthread_mutex_lock(&mutex);
		result += value;
	}
	pthread_mutex_unlock(&mutex);
}

int SetMaharajaOnBoard(int* value, int row, int col, int countFiguresSet)
{
	int cnt, i, j, sdvig;
	bool flag, check_res;

	cnt = L + K;
	if (countFiguresSet == cnt) {
		flag = true;
		i = 0;
		while (i < K) {
			if (Board[inputArrI[i]][inputArrJ[i]] != 1) {
				flag = false;
				break;
			}
			++i;
		}

		if (flag)
			*value += 1;
	}

	if (row == N)
		return 0;

	for (j = col; j < N; ++j) {
		check_res = position_check(row, j);
		if (check_res == true) {
			sdvig = row;
			col = j + 1;
			Board[row][j] = 1;
			if (col == N) {
				col = 0;
				sdvig++;
			}

			SetMaharajaOnBoard(value, sdvig, col, countFiguresSet + 1);
			Board[row][j] = 0;
		}
	}

	for (i = row + 1; i < N; ++i) {
		for (j = 0; j < N; ++j) {
			check_res = position_check(i, j);

			if (check_res == true) {
				sdvig = i;
				col = j + 1;
				Board[i][j] = 1;

				if (col == N) {
					col = 0;
					sdvig++;
				}

				SetMaharajaOnBoard(value, sdvig, col, countFiguresSet + 1);
				Board[i][j] = 0;
			}
		}
	}
}

bool position_check(int row, int col)
{
	bool ret = false;

	for (int i = 0; i < N; i++)
		if (i != col && Board[row][i])
			return ret;
	for (int i = 0; i < N; i++)
		if (i != row && Board[i][col])
			return ret;

	int i = row - 1, j = col - 1;
	while (i >= 0 && j >= 0)
	{
		if (Board[i][j])
			return ret;
		i--;
		j--;
	}
	i = row + 1, j = col + 1;
	while (i < N && j < N)
	{
		if (Board[i][j])
			return ret;
		i++;
		j++;
	}
	i = row + 1, j = col - 1;
	while (i < N && j >= 0)
	{
		if (Board[i][j])
			return ret;
		i++;
		j--;
	}
	i = row - 1, j = col + 1;
	while (i >= 0 && j < N)
	{
		if (Board[i][j])
			return ret;
		i--;
		j++;
	}
	if ((col + 2) < N && (row + 1) < N && Board[row + 1][col + 2])
		return ret;
	if ((col + 1) < N && (row + 2) < N && Board[row + 2][col + 1])
		return ret;
	if ((col - 2) >= 0 && (row - 1) >= 0 && Board[row - 1][col - 2])
		return ret;
	if ((col - 1) >= 0 && (row - 2) >= 0 && Board[row - 2][col - 1])
		return ret;
	if ((col + 2) < N && (row - 1) >= 0 && Board[row - 1][col + 2])
		return ret;
	if ((col - 1) >= 0 && (row + 2) < N && Board[row + 2][col - 1])
		return ret;
	if ((col - 2) >= 0 && (row + 1) < N && Board[row + 1][col - 2])
		return ret;
	if ((col + 1) < N && (row - 2) >= 0 && Board[row - 2][col + 1])
		return ret;

	ret = !ret;
	return ret;
}

void join_all_threads(void) {
	int i = 0;
	while (i < threadsCount) {
		pthread_join(threads[i], 0);
		i++;
	}
}

void time(void) {
	int i;
	clock_t start, end, time_spent;

	start = clock();
	join_all_threads();
	end = clock();

	f = fopen(fileOut, "wb");
	fprintf(f, "%d", result);
	fclose(f);

	f = fopen(fileTime, "wb");
	time_spent = (end - start) / (CLOCKS_PER_SEC / 1000);
	fprintf(f, "%u", time_spent);
	fclose(f);
}