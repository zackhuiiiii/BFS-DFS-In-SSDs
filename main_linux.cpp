
#include <stdio.h>
#include <vector>
#include <map>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

// node struct
struct Node
{
	Node* parnt;           // parent node
	vector<Node*> childs;  // childs node
	int id;                // node it
	int bufferSize;        // size of data
	char* buffer;          // data
};

// size of a page which is 4KB=4096b
unsigned int PAGE_SIZE = 4096;

// Linux direct write
// path:where file is saved
// buffer: 4kb size.data need to write in
// id: block id
// return times it take to finish, -1 if failed
int directWrite(const char* path, int number, vector<Node*>& dataList)
{
	// random seed
	srand((int)time(0));


	if (number > 1)
	{
		for (size_t index = 0; index < number; index++)
		{
			while (true)
			{
				// random parent node
				int pos = rand() % number;
				if (pos != index)
				{
					// set parent node
					dataList[index]->parnt = dataList[pos];

					dataList[pos]->childs.push_back(dataList[index]);
					break;
				}
			}
		}
	}


	struct timeval begin;
	struct timeval end;
	//lunix direct write
	int fd = open(path, O_RDWR | O_CREAT | O_DIRECT | O_DSYNC);


	// track how many byte write in to current node
	unsigned int byteOfUsed = 0;
	vector<char*> bufferList;

	// mask for child node
	map<int, unsigned int> offsetMap;
	for (size_t index = 0; index < number; index++)
	{
		offsetMap.insert(pair<int, unsigned int>(dataList[index]->id, index * PAGE_SIZE));
	}

	// Failed out to 4KB after data store
	for (size_t index = 0; index < number; index++)
	{
		byteOfUsed = 0;


		char* pBuffer = new char[4096];
		memset(pBuffer, 0, 4096);
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
			// write child node id
			p[subIndex + 2 + 1] = dataList[index]->childs[subIndex]->id;
			byteOfUsed += sizeof(int);

			// write mask of child node
			map<int, unsigned int>::iterator it = offsetMap.find(dataList[index]->childs[subIndex]->id);
			p[subIndex + 2 + 2] = it->second;
			byteOfUsed += sizeof(int);
		}

		// filled data
		sprintf(pBuffer + byteOfUsed, "CS%d", dataList[index]->id);
	}


	gettimeofday(&(begin), NULL);

	// write
	for (size_t index = 0; index < bufferList.size(); index++)
	{
		write(fd, bufferList[index], 4096);
	}


	close(fd);
	gettimeofday(&(end), NULL);
	return end - begin;
}

// linux direct read
// path:where file is store
// nodeId:which node we should read
// nodeCount:total number of node
// return time in ms for reading,-1 if fail
int directRead(const char* path, int nodeId, int nodeCount, Node& node)
{
	char buffer[4096] = { 0 };

	struct timeval begin;
	struct timeval end;
	int fd = open(path, O_RDWR | O_CREAT | O_DIRECT | O_DSYNC);


	gettimeofday(&(begin), NULL);
	for (size_t index = 0; index < nodeCount; index++)
	{
		read(fd, (void*)buffer, 4096));
		int* p = (int*)buffer;
		if (p[0] == nodeId)
		{
			node.id = p[0];

			// get number of child node
			int subNodeCount = p[2];

			// calculate mask for data
			int dataBegin = sizeof(int) * 3 + sizeof(int) * subNodeCount * 2;


			int bufferLength = 4096 - dataBegin;


			node.buffer = new char[bufferLength];

			// copy data
			memcpy(node.buffer, buffer + dataBegin, bufferLength);
			break;
		}
	}
	gettimeofday(&(end), NULL);

	close(fd);
	return end - begin;
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


	unsigned int usedTime = directWrite("d:\\data.data", nodeList.size(), nodeList);
    printf("Total write need %d ms \r\n",usedTime);

	Node node;
	node.buffer = 0;

	usedTime = directRead("d:\\data.data", 29, 9, node);

	if (node.buffer == 0)
	{
		printf("Node does not excit\r\n");
	}
	else
	{
		printf("Found! takes:%d ms NODE ID=29 text is %s\r\n", usedTime, node.buffer);
	}

	return 0;

}
