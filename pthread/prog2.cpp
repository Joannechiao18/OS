#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<map>
#include<sys/syscall.h>
#include<time.h>
#include<math.h>
#include<pthread.h>
#include<unistd.h>
#include<algorithm>
using namespace std;

int n;
string search_string;
vector<struct thread>c;
vector<string> pat, v;
map<string, int>m;
vector<vector<int>>indexes;

struct thread
{
	int start, index;
	string search;
	string pattern;
}Thread;

void load(char* argv[]);
void *child(void* param);
void firstAdjust(string search);
void secondAdjust(string search);

int main(int argc, char* argv[])
{
	clock_t start, end;
	double cpu_time_used;

	start = clock();

	vector<pthread_t> tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pat.clear();

	load(argv);

	tid.resize(pat.size()*n);
	c.resize(pat.size()*n);
	indexes.resize(pat.size());

	for(int i = 0; i < c.size(); i++)
		pthread_create(&tid[i], &attr, child, (void*) &c[i]);

	for(int i = 0; i < c.size(); i++)
		pthread_join(tid[i], NULL);

	for(int i = 0; i < pat.size(); i++)
	{
		sort(indexes[m[pat[i]]].begin(), indexes[m[pat[i]]].end());

		cout<<"[" << pat[i] << "] ";
		for(int j = 0; j < indexes[m[pat[i]]].size(); j++)
			cout << indexes[m[pat[i]]][j] << ' ';
		cout << endl;
	}

	end = clock();

	cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

	cout << "CPU time: " << cpu_time_used * 1000 << " ms" << endl;
}

void load(char* argv[])
{
	int where = 0;
	string N, test;
	string str[4], check[4];
	ifstream inFile;
	inFile.open(argv[1]);

	getline(inFile, search_string);
	getline(inFile, test);
	n = stoi(test);

	while(inFile.peek() != EOF)
	{
		getline(inFile, test);

		int POS1 = test.find('?');
		int POS2 = test.find('{');

		if(POS1 < 0 && POS2 < 0)
		{
			vector<string>::iterator it = find(pat.begin(), pat.end(), test);
			if(it == pat.end())
				pat.push_back(test);
		}
		else
		{
			if(POS1 >= 0)
				firstAdjust(test);
			if(POS2 >= 0)
				secondAdjust(test);
			for(int i = 0; i < v.size(); i++)
			{
				int p1 = v[i].find('?');
				int p2 = v[i].find('{');
				if(p1 < 0 && p2 < 0)
					pat.push_back(v[i]);
			}
		}
	}

	vector<string>::iterator it = pat.begin();
	for(; it != pat.end(); it++)
	{
		for(vector<string>::iterator it2 = (it + 1); it2 != pat.end(); it2++)
		 	if(*it2 == *it)
				pat.erase(it2);
	}

	int count = 0, pos = 0, len = 0;
	for(int i = 0; i < pat.size(); i++)
	{
		for(int j = 0; j < n; j++)
		{
			Thread.start = pos;
			len = min(search_string.size()/n + 31, search_string.size() - pos);
			Thread.search = search_string.substr(pos, len);
			Thread.pattern = pat[count];
			Thread.index = j;
			c.push_back(Thread);
			pos += search_string.size()/n;
		}
		pos = 0;
		count++;
	}

	for(int i = 0; i < pat.size(); i++)  //assign a number for each target string
		m[pat[i]] = i;
}

void firstAdjust(string search)
{
	string str[4];
	for (int i = 0; i < 4; i++)
		str[i] = search;

	string::iterator it = search.begin();
	string::iterator it2 = it;
	int pos;

	if (search.find('?') >= 0)
	{
		str[0].replace(str[0].find('?'), 1, "A");
		str[1].replace(str[1].find('?'), 1, "C");
		str[2].replace(str[2].find('?'), 1, "G");
		str[3].replace(str[3].find('?'), 1, "T");
		for (int i = 0; i < 4; i++)
		{
			pos = str[i].find('?');
			if (pos >= 0)
				firstAdjust(str[i]);
			else
			{
				pos = str[i].find('{');
				if(pos >= 0)
					secondAdjust(str[i]);
				else
					v.push_back(str[i]);
			}
		}
	}
}

void secondAdjust(string search)
{
	string str;
	str = search;
	string a = search.substr(0, search.find('{'));
	string s = search.substr(search.find('{'), search.find('}') + 1 - search.find('{'));
	string b = search.substr(search.find('}') + 1, search.size() - search.find('}'));

	int pos = 0;
	for (int i = 1; i < s.size(); i++)
	{
		if (s[i] =='A')
			str = a + "A" + b;
		else if (s[i] == 'C')
			str = a + "C" + b;
		else if (s[i] == 'G')
			str = a + "G" + b;
		else if (s[i] == 'T')
			str = a + "T" + b;

		pos = str.find('{');
		if (pos >= 0)
			secondAdjust(str);
		else
			v.push_back(str);

		i++;
	}
}

void *child(void* param)
{
	int actual_pos = 0;
	thread *C = (thread*) param;
	pthread_t tid = gettid();

	cout<< "[Tid=" << tid << "] search " << C->pattern << " at " << C->start << " ";
	for(int k = C->start; k < (C->start + 8); k++)
		cout << search_string[k];
	cout << endl;

	for(int k = 0; k <= (C->search.size() - C->pattern.size()); k++)
	{
		int j;
		for(j = 0; j < C->pattern.size(); j++)
			if(C->search[k + j] != C->pattern[j])
				break;
		if(j == C->pattern.size())
		{
			actual_pos = 0 + C->index * floor(search_string.size()/n) + k;

			vector<int>::iterator it2 = find(indexes[m[C->pattern]].begin(), indexes[m[C->pattern]].end(), actual_pos);
			if(it2 == indexes[m[C->pattern]].end())
				indexes[m[C->pattern]].push_back(actual_pos);
		}
	}

	pthread_exit(0);
}

