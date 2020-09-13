
#include <stdio.h>
#include <vector>
#include <Windows.h>
#include <map>
#include <stdlib.h>
#include <time.h>

using namespace std;

// Struct Node
struct Node
{
	Node* parnt;           // parent node//comb
	vector<Node*> childs;  // childs node
	int id;                // node it
	int bufferSize;        // size of data
	char* buffer;          // data
};

// size of page
unsigned int PAGE_SIZE = 4096;


// windows direct write
// path:where file is saved
// buffer: 4kb size.data need to write in
// id: block id
// return times it take to finish, -1 if failed
unsigned int directWrite(const char* path, int number, vector<Node*>& dataList)
{
	// random seed
	srand((int)time(0));


	if (number > 1)
	{
		for (size_t index = 0; index < number; index++)
		{
			while (true)
			{
				// random father node
				int pos = rand() % number;
				if (pos != index)
				{

					dataList[index]->parnt = dataList[pos];

					dataList[pos]->childs.push_back(dataList[index]);
					break;
				}
			}
		}
	}

	//Direct write in Windows
	HANDLE hFile = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_NO_BUFFERING, NULL);

	// check ig open
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("Open failed\r\n");
		return  -1;
	}

	// How many byte write in current node
	unsigned int byteOfUsed = 0;
	vector<char*> bufferList;

	// mask for node id and file
	map<int, unsigned int> offsetMap;
	for (size_t index = 0; index < number; index++)
	{
		offsetMap.insert(pair<int, unsigned int>(dataList[index]->id, index * PAGE_SIZE));
	}

	// Filled out to 4KB after data store
	for (size_t index = 0; index < number; index++)
	{
		byteOfUsed = 0;

		char* pBuffer = new char[4096];
		ZeroMemory(pBuffer, 4096);
		int* p = (int*)pBuffer;
		bufferList.push_back(pBuffer);

		byteOfUsed = 0;
		// First 4 byte is node id
		p[0] = dataList[index]->id;
		byteOfUsed += sizeof(int);
		// Next 4 byte is parents ID
		p[1] = dataList[index]->parnt->id;
		byteOfUsed += sizeof(int);
		// Next 4 byte saves number of childs
		p[2] = dataList[index]->childs.size();
		byteOfUsed += sizeof(int);

		// use for loop to write in
		for (size_t subIndex = 0; subIndex < p[2]; subIndex++)
		{
			// write in child node id
			p[subIndex + 2 + 1] = dataList[index]->childs[subIndex]->id;
			byteOfUsed += sizeof(int);

			// write in mask for child node
			map<int, unsigned int>::iterator it = offsetMap.find(dataList[index]->childs[subIndex]->id);
			p[subIndex + 2 + 2] = it->second;
			byteOfUsed += sizeof(int);
		}

		// filled out
		sprintf(pBuffer + byteOfUsed, "CS%d", dataList[index]->id);
	}

	//time it takes
	unsigned int begin = GetTickCount();

	// Write 4kb in to desk
	for (size_t index = 0; index < bufferList.size(); index++)
	{
		DWORD numberOfByteWrite = 4096;
		WriteFile(hFile, bufferList[index], numberOfByteWrite, &numberOfByteWrite, 0);
	}


	CloseHandle(hFile);
	return GetTickCount() - begin;
}

// windows direct read
// path:where file is store
// nodeId:which node we should read
// nodeCount:total number of node
// return time in ms for reading,-1 if fail
DWORD directRead(const char* path, int nodeId, int nodeCount, Node& node)
{

	HANDLE hFile = NULL;
	char buffer[4096] = { 0 };

	// direct read
	hFile = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);

	// check if open
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("Open fail\r\n");
		return  -1;
	}

	// start time of reading
	DWORD begin = GetTickCount();
	DWORD numberOfByteWrite = 4096;

	for (size_t index = 0; index < nodeCount; index++)
	{

		ReadFile(hFile, buffer, numberOfByteWrite, &numberOfByteWrite, NULL);
		int* p = (int*)buffer;
		if (p[0] == nodeId)
		{
			node.id = p[0];

			// get number of child node
			int subNodeCount = p[2];

			// check where the data store and leave filled 0s
			int dataBegin = sizeof(int) * 3 + sizeof(int) * subNodeCount * 2;

			int bufferLength = 4096 - dataBegin;


			node.buffer = new char[bufferLength];

			// copy data
			memcpy(node.buffer, buffer + dataBegin, bufferLength);
			break;
		}
	}


	CloseHandle(hFile);
	return GetTickCount() - begin;
}

int main()
{
	vector<Node*> nodeList;

	Node* p1 = new Node();
	p1->id = 51;
	nodeList.push_back(p1);

	Node* p2 = new Node();
	p2->id = 81;
	nodeList.push_back(p2);

	Node* p3 = new Node();
	p3->id = 77;
	nodeList.push_back(p3);

	Node* p4 = new Node();
	p4->id = 33;
	nodeList.push_back(p4);

	Node* p5 = new Node();
	p5->id = 103;
	nodeList.push_back(p5);

	Node* p6 = new Node();
	p6->id = 3;
	nodeList.push_back(p6);

	Node* p7 = new Node();
	p7->id = 11;
	nodeList.push_back(p7);

	Node* p8 = new Node();
	p8->id = 29;
	nodeList.push_back(p8);

	Node* p9 = new Node();
	p9->id = 37;
	nodeList.push_back(p9);

    Node* p10 = new Node();
	p10->id = 55;
	nodeList.push_back(p10);

	unsigned int usedTime =  directWrite("d:\\data.data", nodeList.size(), nodeList);

	printf("Total write need %d ms \r\n",usedTime);

	Node node;
	node.buffer = 0;

	usedTime = directRead("d:\\data.data", 10223, 10, node);

	if (node.buffer == 0)
	{
		printf("Node does not excit\r\n");
	}
	else
	{
		printf("Found! takes:%d ms NODE ID=55 text is %s\r\n", usedTime, node.buffer);
	}

	return 0;

}
