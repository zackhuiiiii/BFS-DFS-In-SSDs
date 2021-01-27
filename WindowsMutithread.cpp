
#include <stdio.h>
#include <vector>
#include <Windows.h>
#include <map>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <set>
#include <queue>
#include <stack>
#include <thread>
#include <iostream>

using namespace std;

// node structure
struct Node
{
	vector<int> paths;  // save path
	int id;               // node ID
};

// 4kb
unsigned int PAGE_SIZE = 4096;

bool isFind = false;

vector<Node> nodes;

// init
void init(const char* fileNmae)
{
	ifstream fin;
	int index = 0;
	char buffer[256] = { 0 };
	char temp[256] = { 0 };
	int tempIndex = 0;

	// open file
	fin.open(fileNmae, ios::in);

	// read first line how many node and how many edge
	fin.getline(buffer, 256);


	while (buffer[index] != ' ')
	{
		temp[tempIndex++] = buffer[index++];
	}
	temp[tempIndex] = '\0';
	int nodeCount = atoi(temp);

	tempIndex = 0;
	while (buffer[index] != '\0')
	{
		temp[tempIndex++] = buffer[index++];
	}
	temp[tempIndex] = '\0';
	int pathCount = atoi(temp);

	// contruct graph
	for (int i = 0; i < nodeCount; i++)
	{
		Node node;
		node.paths.clear();
		node.id = i;
		nodes.push_back(node);
	}


	for (int i = 0; i < pathCount; i++)
	{

		fin.getline(buffer, 256);


		index = 0;
		tempIndex = 0;
		while (buffer[index] != ' ')
		{
			temp[tempIndex++] = buffer[index++];
		}
		temp[tempIndex] = '\0';
		int from = atoi(temp);


		tempIndex = 0;
		while (buffer[index] != '\0')
		{
			temp[tempIndex++] = buffer[index++];
		}
		temp[tempIndex] = '\0';
		int to = atoi(temp);


		nodes[from].paths.push_back(to);
	}

	// close file
	fin.close();
}

// windows no buffer wirte
// path:where file located
// return,how long it take to write the graph,unit:ms,return -1 if fail
unsigned int directWrite(const char* path)
{
	//windows no buffer wirte
	HANDLE hFile = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_NO_BUFFERING, NULL);

	// check if file open
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("file can't open\r\n");
		return  -1;
	}

	// how many byte already in
	unsigned int byteOfUsed = 0;
	vector<char*> bufferList;


    //start write
	for (size_t index = 0; index < nodes.size(); index++)
	{
		byteOfUsed = 0;


		char* pBuffer = new char[PAGE_SIZE];
		ZeroMemory(pBuffer, PAGE_SIZE);
		int* p = (int*)pBuffer;

		bufferList.push_back(pBuffer);

		// first 4 byte saves node id
		p[0] = nodes[index].id;
		byteOfUsed += sizeof(int);
		// next 4 byte save child node path
		p[1] = nodes[index].paths.size();
		byteOfUsed += sizeof(int);


		for (size_t subIndex = 0; subIndex < p[1]; subIndex++)
		{
			p[subIndex + 2] = nodes[index].paths[subIndex];
			byteOfUsed += sizeof(int);
		}

		sprintf(pBuffer + byteOfUsed, "CS%d", nodes[index].id);
	}


	unsigned int begin = GetTickCount();

	for (size_t index = 0; index < bufferList.size(); index++)
	{
		DWORD numberOfByteWrite = 4096;
		WriteFile(hFile, bufferList[index], numberOfByteWrite, &numberOfByteWrite, nullptr);
	}


	CloseHandle(hFile);


	for (size_t index = 0; index < bufferList.size(); index++)
	{
		delete[] bufferList[index];
	}
	return GetTickCount() - begin;
}

// windows no buffer dfs read
// path:file located
// beginNode:start node ID
// targetNode:target ID
void dfsDirectRead(const char* path, int beginNode, int targetNode)
{

	HANDLE hFile = NULL;
	char buffer[4096] = { 0 };
	char result[4096] = { 0 };
	int deatBegin = 0;
	int count = 0;
	vector<int> paths;
	ULONGLONG timeConsuming = 0;
	set<int> nodeSet;
	// dfs stack
	stack<int> nodeStack;
	DWORD numberOfByteWrite = 4096;
	int* pData = nullptr;

	// open file
	hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);

	// check
	if (hFile == INVALID_HANDLE_VALUE)
	{
		cout << "Read fail" << endl;
		return;
	}


	ULONGLONG begin = GetTickCount64();

	// read start node
	// move pointer to file start location
	SetFilePointer(hFile, beginNode * 4096, nullptr, FILE_BEGIN);
	// read
	if (false == ReadFile(hFile, buffer, numberOfByteWrite, &numberOfByteWrite, NULL))
	{
		CloseHandle(hFile);
		cout << "Read fail" << endl;
		return;
	}

	count++;
	pData = (int*)buffer;
	paths.push_back(pData[0]);

	// if first node is target, print
	if (pData[0] == targetNode)
	{
		isFind = true;
		deatBegin = pData[1] * sizeof(int) + sizeof(int) * 2;
		memcpy(result, buffer + deatBegin, 4096 - deatBegin);
		cout << result << endl;
		timeConsuming = GetTickCount64() - begin;

		cout << "DFS takes " << timeConsuming << " ms,Number of I/0: " << count << " TIMES" << endl;
		cout << "Path:" << endl;
		for (int item : paths)
		{
			cout << item << endl;
		}
		return;
	}

	// read all node connect to this node
	for (int i = 0; i < pData[1]; i++)
	{
		// check if duplicate
		if (nodeSet.find(pData[2 + i]) == nodeSet.end())
		{
			nodeSet.insert(pData[2 + i]);
			// all to stack
			nodeStack.push(pData[2 + i]);
		}
	}

	// DFS
	while (!nodeStack.empty())
	{
		if (isFind == true)
		{
			return;
		}

		int curId = nodeStack.top();
		nodeStack.pop();

		SetFilePointer(hFile, curId * 4096, nullptr, FILE_BEGIN);
		numberOfByteWrite = 4096;
		if (false == ReadFile(hFile, buffer, numberOfByteWrite, &numberOfByteWrite, NULL))
		{
			CloseHandle(hFile);
			cout << "READ FAILS" << endl;
			return;
		}
		count++;
		pData = (int*)buffer;
		paths.push_back(pData[0]);

		// if found, return
		if (curId == targetNode)
		{
			isFind = true;
			deatBegin = pData[1] * sizeof(int) + sizeof(int) * 2;
			memcpy(result, buffer + deatBegin, 4096 - deatBegin);
			cout << result << endl;
			timeConsuming = GetTickCount64() - begin;

			cout << "DFS takes " << timeConsuming << " ms,Number of I/0: " << count << " TIMES" << endl;
			cout << "Path:" << endl;
			for (int item : paths)
			{
				cout << item << endl;
			}
			return;
		}
		else
		{

			for (int i = 0; i < pData[1]; i++)
			{

				if (nodeSet.find(pData[2 + i]) == nodeSet.end())
				{

					nodeSet.insert(pData[2 + i]);

					nodeStack.push(pData[2 + i]);
				}
			}
		}
	}

	cout << "Node not found" << endl;
	timeConsuming = GetTickCount64() - begin;

	cout << "DFS takes " << timeConsuming << " ms,Number of I/0: " << count << " TIMES" << endl;
	cout << "Path:" << endl;
	for (int item : paths)
	{
		cout << item << endl;
	}
	return;
}

// windows bfs read
// path:file path
// beginNode:start ID
// targetNode:target ID
void bfsDirectRead(const char* path, int beginNode, int targetNode)
{
	HANDLE hFile = NULL;
	char buffer[4096] = { 0 };
	char result[4096] = { 0 };
	int dataBegin = 0;
	vector<int> paths;
	int count = 0;
	ULONGLONG timeConsuming = 0;

	set<int> nodeSet;
	// bfs queue
	queue<int> nodeQueue;
	DWORD numberOfByteWrite = 4096;
	int* pData = nullptr;
	hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		cout << "Read fail" << endl;
		return;
	}

	ULONGLONG begin = GetTickCount64();


	SetFilePointer(hFile, beginNode * 4096, nullptr, FILE_BEGIN);

	if (false == ReadFile(hFile, buffer, numberOfByteWrite, &numberOfByteWrite, NULL))
	{
		CloseHandle(hFile);
		cout << "Read fail" << endl;
		return;
	}

	count++;
	pData = (int*)buffer;
	paths.push_back(pData[0]);

	// if the first is target return
	if (pData[0] == targetNode)
	{
		isFind = true;
		dataBegin = pData[1] * sizeof(int) + sizeof(int) * 2;

		memcpy(result, buffer + dataBegin, 4096 - dataBegin);
		cout << result << endl;
		timeConsuming = GetTickCount64() - begin;

		cout << "BFS takes " << timeConsuming << " ms,Number of I/0: " << count << " TIMES" << endl;
		cout << "Path:" << endl;
		for (int item : paths)
		{
			cout << item << endl;
		}
		return;
	}

	// read all neigbor
	for (int i = 0; i < pData[1]; i++)
	{
		// check if duplicate
		if (nodeSet.find(pData[2 + i]) == nodeSet.end())
		{

			nodeSet.insert(pData[2 + i]);
			// add to queue
			nodeQueue.push(pData[2 + i]);
		}
	}

	// BFS
	while (!nodeQueue.empty())
	{
		if (isFind == true)
		{
			CloseHandle(hFile);
			return;
		}
		int curId = nodeQueue.front();
		nodeQueue.pop();


		SetFilePointer(hFile, curId * 4096, nullptr, FILE_BEGIN);

		numberOfByteWrite = 4096;
		if (false == ReadFile(hFile, buffer, numberOfByteWrite, &numberOfByteWrite, NULL))
		{
			CloseHandle(hFile);
			cout << "Read fail" << endl;
			return;
		}

		count++;
		pData = (int*)buffer;
		paths.push_back(pData[0]);

		// if found, return
		if (curId == targetNode)
		{
			isFind = true;
			dataBegin = pData[1] * sizeof(int) + sizeof(int) * 2;

			memcpy(result, buffer + dataBegin, 4096 - dataBegin);
			cout << result << endl;

			CloseHandle(hFile);
			timeConsuming = GetTickCount64() - begin;

			cout << "BFS takes " << timeConsuming << " ms,Number of I/0: " << count << " TIMES" << endl;
			cout << "Path:" << endl;
			for (int item : paths)
			{
				cout << item << endl;
			}
			return;
		}
		else
		{

			for (int i = 0; i < pData[1]; i++)
			{

				if (nodeSet.find(pData[2 + i]) == nodeSet.end())
				{
					nodeSet.insert(pData[2 + i]);
					nodeQueue.push(pData[2 + i]);
				}
			}
		}
	}

	CloseHandle(hFile);
	cout << "NOT Found" << endl;
	cout << "BFS takes " << timeConsuming << " ms,Number of I/0: " << count << " TIMES" << endl;
	cout << "Path:" << endl;
	for (int item : paths)
	{
		cout << item << endl;
	}
}


int main()
{
	int count;
	vector<int> paths;
	init("acyclic_100_250");// file name
	directWrite("data.data");
	count = 0;
	bfsDirectRead("data.data", 1, 97);

	thread* threads = new thread[4];
	int beginIds[4];
	beginIds[0] = 0;
	beginIds[1] = 2;
	beginIds[2] = 3;
	beginIds[3] = 4;

	for (unsigned int i = 0; i < 4; i++) {
		threads[i] = thread(bfsDirectRead, "data.data", beginIds[i], 97);
	}

	for (unsigned int i = 0; i < 4; i++) {
		threads[i].join();
	}
	delete[] threads;


	isFind = false;
	threads = new thread[4];
	for (unsigned int i = 0; i < 4; i++) {
		threads[i] = thread(dfsDirectRead, "data.data", beginIds[i], 97);
	}

	for (unsigned int i = 0; i < 4; i++) {
		threads[i].join();
	}
	delete[] threads;
	return 0;
}
