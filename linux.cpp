
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
#include <fstream>
#include <set>
#include <queue>
#include <stack>

using namespace std;

// struct
struct Node
{
	vector<int> paths;  // path
	int id;               // ID
};

// size of a page
unsigned int PAGE_SIZE = 4096;

vector<Node> nodes;

// init all node
void init(const char* fileNmae)
{
	ifstream fin;
	int index = 0;
	char buffer[256] = { 0 };
	char temp[256] = { 0 };
	int tempIndex = 0;

	// open file
	fin.open(fileNmae, ios::in);

	// read node and edge size
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


	fin.close();
}

// linux directwrite
// path:where file located
// return total ms
int directWrite(const char* path)
{
	struct timeval begin;
	struct timeval end;
	int fd = open(path, O_RDWR | O_CREAT | O_DIRECT | O_DSYNC);


	unsigned int byteOfUsed = 0;
	vector<char*> bufferList;



	for (size_t index = 0; index < nodes.size(); index++)
	{
		byteOfUsed = 0;


		char* pBuffer = new char[PAGE_SIZE];
		memset(pBuffer, 0, PAGE_SIZE);
		int* p = (int*)pBuffer;

		bufferList.push_back(pBuffer);

		// first 4 byte save node id
		p[0] = nodes[index].id;
		byteOfUsed += sizeof(int);
		// nexr 4 is nerigbor info
		p[1] = nodes[index].paths.size();
		byteOfUsed += sizeof(int);
		for (size_t subIndex = 0; subIndex < p[1]; subIndex++)
		{
			p[subIndex + 2] = nodes[index].paths[subIndex];
			byteOfUsed += sizeof(int);
		}

		sprintf(pBuffer + byteOfUsed, "CS%d", nodes[index].id);
	}


	gettimeofday(&(begin), NULL);

	for (size_t index = 0; index < bufferList.size(); index++)
	{
		write(fd, bufferList[index], 4096);
	}


	for (size_t index = 0; index < bufferList.size(); index++)
	{
		delete[] bufferList[index];
	}

	close(fd);
	gettimeofday(&(end), NULL);
	return end - begin;
}


// linux dfs directread
// path:file located
// nodeId:the node we want
// return total ms for search
int dfsDirectRead(const char* path, int nodeId)
{

	char buffer[4096] = { 0 };
	char result[4096] = { 0 };
	int deatBegin = 0;
	set<int> nodeSet;
	stack<int> nodeStack;
	int* pData = nullptr;

	struct timeval begin;
	struct timeval end;
	int fd = open(path, O_RDWR | O_CREAT | O_DIRECT | O_DSYNC);



	gettimeofday(&(begin), NULL);


	read(fd, (void*)buffer, 4096));
	pData = (int*)buffer;


	if (pData[0] == nodeId)
	{
		deatBegin = pData[1] * sizeof(int) + sizeof(int) * 2;

		memcpy(result, buffer + deatBegin, 4096 - deatBegin);
		printf("%s\r\n", result);

		gettimeofday(&(end), NULL);
		close(fd);
		return end - begin;
	}


	for (int i = 0; i < pData[1]; i++)
	{

		if (nodeSet.find(pData[2 + i]) == nodeSet.end())
		{

			nodeSet.insert(pData[2 + i]);

			nodeStack.push(pData[2 + i]);
		}
	}


	while (!nodeStack.empty())
	{
		int curId = nodeStack.top();
		nodeStack.pop();


		lseek(fd, curId * 4096, SEEK_BEGIN)

		read(fd, (void*)buffer, 4096));
		pData = (int*)buffer;


		if (curId == nodeId)
		{
			deatBegin = pData[1] * sizeof(int) + sizeof(int) * 2;

			memcpy(result, buffer + deatBegin, 4096 - deatBegin);
			printf("%s\r\n", result);

			gettimeofday(&(end), NULL);
			close(fd);
			return end - begin;
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

	printf("Node not found\r\n");
	gettimeofday(&(end), NULL);
	close(fd);
	return end - begin;
}

// linux bfs direct read
// path:file located
// nodeId:node id we want tor ead
// return total ms for search
int bfsDirectRead(const char* path, int nodeId)
{

	char buffer[4096] = { 0 };
	char result[4096] = { 0 };
	int deatBegin = 0;

	set<int> nodeSet;

	queue<int> nodeQueue;
	int* pData = nullptr;

	struct timeval begin;
	struct timeval end;
	int fd = open(path, O_RDWR | O_CREAT | O_DIRECT | O_DSYNC);



	gettimeofday(&(begin), NULL);
	read(fd, (void*)buffer, 4096));
	pData = (int*)buffer;

	if (pData[0] == nodeId)
	{
		deatBegin = pData[1] * sizeof(int) + sizeof(int) * 2;

		memcpy(result, buffer + deatBegin, 4096 - deatBegin);
		printf("%s\r\n", result);

		gettimeofday(&(end), NULL);
		close(fd);
		return end - begin;
	}


	for (int i = 0; i < pData[1]; i++)
	{

		if (nodeSet.find(pData[2 + i]) == nodeSet.end())
		{

			nodeSet.insert(pData[2 + i]);

			nodeQueue.push(pData[2 + i]);
		}
	}


	while (!nodeQueue.empty())
	{
		int curId = nodeQueue.front();
		nodeQueue.pop();

		lseek(fd, curId * 4096, SEEK_BEGIN)

		read(fd, (void*)buffer, 4096));
		pData = (int*)buffer;


		if (curId == nodeId)
		{
			deatBegin = pData[1] * sizeof(int) + sizeof(int) * 2;

			memcpy(result, buffer + deatBegin, 4096 - deatBegin);
			printf("%s\r\n", result);

			gettimeofday(&(end), NULL);
			close(fd);
			return end - begin;
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

	printf("Node not found\r\n");
	gettimeofday(&(end), NULL);
	close(fd);
	return end - begin;
}

int main()
{
	return 0;

}
