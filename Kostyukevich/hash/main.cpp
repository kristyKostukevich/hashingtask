#include <Windows.h>
#include <fstream>
#include "../src/hl_md5wrapper.h"
#include "../src/hl_sha1wrapper.h"

using namespace std;

HANDLE inputMutex, outputMutex;
ifstream *fin;
ofstream *fout;
hashwrapper *md5 = new md5wrapper();
hashwrapper *sha1 = new sha1wrapper();


int sizeBlock = 0;
char* hashType;

void Thread(void* pParams)
{
		char* block = new char[sizeBlock];
		while (!fin->eof())
		{
			WaitForSingleObject(inputMutex, INFINITE);
				fin->read((char*)(block), sizeBlock);
			ReleaseMutex(inputMutex);
			
			WaitForSingleObject(outputMutex, INFINITE);
				if (!strcmp(hashType, "md5"))	*fout << md5->getHashFromString(block) << endl;
				else	*fout << sha1->getHashFromString(block) << endl;
			ReleaseMutex(outputMutex);
		}

}

int main ( int argc, char **argv)
{
	char fileName[256];
	int countThreads = 1;
	if (!strcmp(argv[4], "sync")) countThreads = 3;
	sizeBlock = atoi(argv[2]);
	hashType = argv[3];

	HANDLE *threads = new HANDLE[countThreads];
	inputMutex = CreateMutex(NULL, FALSE, (LPCTSTR)"file input");
	outputMutex = CreateMutex(NULL, FALSE, (LPCTSTR)"file output");

	strcpy(fileName, argv[1]);
	strcat(fileName, ".hashes_");
	strcat(fileName, argv[3]);

	fin = new ifstream(argv[1], ios::binary | ios::in);
	fout = new ofstream(fileName, ios::out);

	for (int i = 0; i < countThreads; i++) threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread, NULL, 0, NULL);
	WaitForMultipleObjects(countThreads, threads, TRUE, INFINITE);

	fin->close();
	fout->close();
	CloseHandle(inputMutex);
	CloseHandle(outputMutex);
	for (int i = 0; i < countThreads; i++) CloseHandle(threads[i]);
	delete threads;
	delete md5;
	delete sha1;

	return 0;
}
