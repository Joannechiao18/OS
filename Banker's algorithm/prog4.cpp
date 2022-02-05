#include<iostream>
#include<vector>
#include<string>
#include<fstream>
#include<sstream>
using namespace std;

struct task
{
	int gid;
	int instance[5];
	char aORr;
}Array;

vector<task>Available;
vector<task>Max;
vector<task>Allocation;
vector<task>Request;
vector<task>Need;
vector<task>Work;
vector<int>safe_seq;
vector<task>wait;
vector<int>wait_num;
bool found = false, finish_need = false;

void read(char* argv[]);
void cal_need();
void check_safe();
void print(struct task array);
void modify_first(struct task arr1, struct task arr2);
void modify_second(struct task arr1, struct task arr2);
bool check_request(int num);

int main(int argc, char* argv[])
{
	read(argv);

	cal_need();

	Available[0].gid = -1;
	Work.push_back(Available[0]);

	check_safe();
	if (found == true)
	{
		cout << "Initial state: safe, safe sequence = [";
		for (int s = 0; s < safe_seq.size() - 1; s++)
			cout << safe_seq[s] << ",";
		cout << safe_seq[safe_seq.size() - 1] << "]" << endl;
	}
	else
		cout << "Initial state : unsafe" << endl;

	for (int i = 0; i < Request.size(); i++)
	{
		if (Request[i].aORr == 'r')  
		{
			cout << "(" << Request[i].gid << ",";
			print(Request[i]);
			cout << ": RELEASE granted, AVAILABLE = (";

			for (int j = 0; j < 5; j++)
				Available[0].instance[j] += Request[i].instance[j];
			print(Available[0]);
			cout << endl;

			for (int w = 0; w < wait.size(); w++)
			{
				if (check_request(wait_num[w]))
				{
					Work.clear();  
					safe_seq.clear();

					for (int j = 0; j < 5; j++)
						Array.instance[j] = (Available[0].instance[j] - Request[w].instance[j]);
					struct task buf = Available[0];
					Available.clear();
					Available.push_back(Array);
					Work = Available;

					modify_first(Allocation[Request[w].gid], Request[w]);
					modify_second(Need[Request[w].gid], Request[w]);
					check_safe();

					print(Request[w]);
					if (found == true)
					{
						cout << ": request granted, safe sequence = [" << endl;
						for (int s = 0; s < safe_seq.size() - 1; s++)
							cout << safe_seq[s] << ",";
						cout << safe_seq[safe_seq.size() - 1] << "]" << endl;
					}
					else
					{
						cout << ": request granted, unsafe" << endl;
						Available[0] = buf;
					}
						
					wait.erase(wait.begin(), wait.begin() + w);
				}
			}
		}
		else
		{
			if (!check_request(i))  //request不能被許可，
			{
				cout << "(" << Request[i].gid << ",";
				print(Request[i]);
				cout << ": ";
				for (int j = 0; j < 5; j++)  //檢查need
				{
					if (Request[i].instance[j] > Need[i].instance[j]) 
					{
						finish_need = true;
						cout << "NEED error, request aborted" << endl;
						break;
					}				
				}

				if (!finish_need)
				{
					for (int j = 0; j < 5; j++)  //檢查available
					{
						if (Request[i].instance[j] > Available[0].instance[j])
						{
							cout << "AVAILABLE error, request aborted" << endl;
							break;
						}
					}
				}
				
				wait.push_back(Request[i]);  //合法request，但還不能被granted，先存到wait
				wait_num.push_back(i);
			}
			else  //request可以被許可，進行safety判斷
			{
				cout << "(" << Request[i].gid << ",";
				print(Request[i]);
				cout << ": NEED OK" << endl;

				cout << "(" << Request[i].gid << ",";
				print(Request[i]);
				cout << ": AVAILABLE OK" << endl;

				Work.clear();  //work和safe sequence都重新計算
				safe_seq.clear();	

				Array.gid = -1;
				for (int j = 0; j < 5; j++) 
					Array.instance[j] = (Available[0].instance[j] - Request[i].instance[j]);
				struct task buf = Available[0];
				Available.clear();
				Available.push_back(Array);
				Work = Available;

				cout << "(" << Request[i].gid << ",";
				print(Request[i]);
				cout << ": WORK = (";
				print(Work[Work.size() - 1]);
				cout << endl;
				
				modify_first(Allocation[Request[i].gid], Request[i]);
				modify_second(Need[Request[i].gid], Request[i]);

				cout << "(" << Request[i].gid << ",";
				print(Request[i]);
				cout << ": ";

				check_safe();
				if (found == true)
				{
					cout << "request granted, safe sequence = [";
					for (int i = 0; i < safe_seq.size() - 1; i++)
						cout << safe_seq[i] << ",";
					cout << safe_seq[safe_seq.size() - 1] << "]" << endl;
				}
				else
				{
					cout << "reqeust cannot be granted" << endl;
					Available[0] = buf;
				}		
			}
		}
	}
}

void read(char *argv[])
{
	string temp;
	ifstream inFile;
	inFile.open(argv[1]);

	while(inFile.peek() != EOF)
	{
		inFile >> temp;

		if (temp == "#AVAILABLE")
		{
			while (getline(inFile, temp) && temp[0] != '#')
			{
				for (int i = 0; i < 5; i++)
					inFile >> Array.instance[i];				
				Available.push_back(Array);
				inFile.ignore();
			}
		}
		if (temp == "#MAX")
		{
			char id;
			while (inFile.get(id) && id != '#')
			{
				stringstream ss;
				ss <<id;
				ss >> Array.gid;
				for (int i = 0; i < 5; i++) 
					inFile >> Array.instance[i];		
				Max.push_back(Array);
				inFile.ignore();
			}
		}

		getline(inFile, temp);

		if (temp == "ALLOCATION")
		{
			char id;
			while (inFile.get(id) && id != '#')
			{
				stringstream ss;
				ss << id;
				ss >> Array.gid;
				for (int i = 0; i < 5; i++)
					inFile >> Array.instance[i];
				Allocation.push_back(Array);
				inFile.ignore();
			}
		}

		getline(inFile, temp);

		if (temp == "REQUEST")
		{
			char id;
			while (inFile.get(id) && id != '#')
			{
				stringstream ss;
				ss << id;
				ss >> Array.gid;
				for (int i = 0; i < 5; i++)
					inFile >> Array.instance[i];
				inFile >> Array.aORr;
				Request.push_back(Array);
				inFile.ignore();
			}
		}
	}
}

void cal_need()
{
	for (int i = 0; i < Allocation.size(); i++)
	{
		Array.gid = Allocation[i].gid;
		for (int j = 0; j < 5; j++)
			Array.instance[j] = (Max[i].instance[j] - Allocation[i].instance[j]);
		Need.push_back(Array);
	}
}

void check_safe()
{
	int count = 0;
	bool* finish = new bool[Need.size()]();

	while (count != Need.size())  //1. 還沒結束(count=process 數量代表結束) 2.系統是unsafe state
	{
		found = false;
		for (int i = 0; i < Need.size(); i++)
		{
			if (!finish[i])
			{
				int j;
				for (j = 0; j < 5; j++)
					if (Need[i].instance[j] > Work[Work.size() - 1].instance[j])
						break;

				if (j == 5)
				{
					for (int k = 0; k < 5; k++)
						Array.instance[k] = (Work[Work.size() - 1].instance[k] + Allocation[i].instance[k]);

					Array.gid = i;
					Work.push_back(Array);
					safe_seq.push_back(i);
					count++;
					finish[i] = true;
					found = true;
				}
			}
		}

		if (!found) 
			return;
	}
}

void print(struct task array)
{
	for (int i = 0; i < 4; i++)
		cout << array.instance[i] << ",";
	cout << array.instance[4] << ")";
}

void modify_first(struct task arr1, struct task arr2)
{
	for (int i = 0; i < 5; i++)
		arr1.instance[i] += arr2.instance[i];
}

void modify_second(struct task arr1, struct task arr2)
{
	for (int i = 0; i < 5; i++)
		arr1.instance[i] -= arr2.instance[i];
}

bool check_request(int num)
{
	for (int i = 0; i < 5; i++)
		if (Request[num].instance[i] > Available[0].instance[i])
			return false;
	return true;
}
