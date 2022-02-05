#include<iostream>
#include<vector>
#include<sys/syscall.h>
#include<pthread.h>
#include<ctime>
#include<unistd.h>
#include<errno.h>
using namespace std;

#define NUM_STUDENT 20

struct thread
{
	int num;
	int count;
	bool inChair;
	bool verify;
};

pthread_mutex_t shot, chair, observe;
int total, robot_data, n;
time_t now;
bool shotting = false, wait = false;
struct tm ltm;
vector<thread> student_data(NUM_STUDENT), c;
void *robot_func(void *robot_data);
void *student_func(void *student_data);
void gettime();

int main(int argc, char *argv[])
{
	pthread_t r;
	pthread_t s[20];

	pthread_mutex_init(&shot, NULL);
	pthread_mutex_init(&chair, NULL);

	if(atoi(argv[1]) >= 10 && atoi(argv[1]) <= 20)
		n = atoi(argv[1]);
	else
		exit(1);

	if(atoi(argv[2]) >= 0 && atoi(argv[2]) <= 100)
		srand(atoi(argv[2]));
	else
		exit(1);

	for(int i = 0; i < n; i++)
	{
		student_data[i].num = i + 1;
		student_data[i].count = 0;
		student_data[i].inChair = false;
		student_data[i].verify = false;
		pthread_create(&s[i], NULL, student_func, (void*) &student_data[i]);
	}

	pthread_create(&r, NULL, robot_func, (void*) &robot_data);

	for(int i = 0; i < n; i++)
		pthread_join(s[i], NULL);

	pthread_join(r, NULL);
}

void *robot_func(void* robot_data)
{
	while (true)
	{
		if(total < n && c.size() == 0 && !shotting)
		{
			gettime();
			cout << " Robot: Sleeping..." << endl;
		}

		if(total == n)
		{
			gettime();
			cout << " Robot: All " << n << " students recieved vaccines." << endl;
			break;
		}
		sleep(1);
	}

	return 0;
}

void *student_func(void* studentdata)
{
	struct thread stu;
	int waiting = 0;
	thread* studentData = (thread*) studentdata;

	if (studentData->count == 0)
	{
		waiting = (rand() % 11 + 0);
		sleep(waiting);
	}

	while (true)
	{
		// 一開始先排隊，檢查座位滿了沒有
		pthread_mutex_lock(&observe);
		if (c.size() < 3 && !studentData->inChair)		// c.size() < 3 代表還有空椅子，inChair判斷他是不是已經坐在椅子上了，如果已經坐著等了，就不用再把他丟進去排隊
		{
			stu.num = studentData->num;
			stu.count = studentData->count;

			if (c.size() != 0 || shotting)				// 要排隊(1.椅子可能有坐人 / 2.有人正在打針)
			{
				pthread_mutex_lock(&chair);
				c.push_back(stu);
				pthread_mutex_unlock(&chair);
				gettime();
				cout << " Student " << studentData->num << ": Sitting #" << c.size() << "." << endl;
				studentData->inChair = true;
			}
			else										// 不用排隊 可以直接打針(椅子上沒有人)
			{
				shotting = true;
				studentData->verify = true;				// verify=True 代表可以直接打針
			}
			pthread_mutex_unlock(&observe);
		}
		else											// 椅子都有人坐 沒有空位 就繼續慢慢等
		{
			pthread_mutex_unlock(&observe);
			waiting = (rand() % 6 + 0);
			sleep(waiting);
			continue;
		}

		// 檢查能不能打針
		pthread_mutex_lock(&shot);
		if((c.size() == 0 && studentData->verify) || c[0].num == studentData->num)  // 1.沒有人排隊的時候 / 2.排在第一個的時候
		{
			shotting = true;
			if (c.size())															// if c.size() == if c.size() > 0, 代表椅子目前有人坐 要把他叫出來進去打針
			{
				pthread_mutex_lock(&chair);
				c.erase(c.begin(), c.begin() + 1);
				pthread_mutex_unlock(&chair);
			}

			gettime();
			cout << " Student " << studentData->num << ": Entering to get a shot..." << endl;
			studentData->count++;
			sleep(2);

			if(studentData->count == 3)
			{
				total++;
				gettime();
				cout << " Student " << studentData->num << ": Finished three shots!" << endl;
				pthread_mutex_unlock(&shot);
				shotting = false;
				break;
			}
			else
			{
				gettime();
				cout << " Student " << studentData->num << ": Leaving, but need to be back." << endl;
				studentData->inChair = false;
				studentData->verify = false;
				pthread_mutex_unlock(&shot);

				shotting = false;
				waiting = (rand() % 21 + 10);
				sleep(waiting);
			}
		}
		else
			pthread_mutex_unlock(&shot);
	}

	return 0;
}

void gettime()
{
	now = time(0);
	localtime_r(&now, &ltm);

	if(ltm.tm_hour < 10)
		cout << "0";
	cout << ltm.tm_hour << ":";
	if(ltm.tm_min < 10)
		cout << "0";
	cout << ltm.tm_min << ":";
	if(ltm.tm_sec < 10)
		cout << "0";
	cout << ltm.tm_sec;
}
