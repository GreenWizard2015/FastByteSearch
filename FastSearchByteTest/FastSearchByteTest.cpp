#include <iostream>
#include <intrin.h>
#include "FastByteSearch.h"

using namespace std;
const int bufferSize = 1024 * 1024; 
const int repeats = 100;

int naive(unsigned char* buffer, int bSize, unsigned char SearchB){
	for(int i = 0; i < bSize; i++)
		if(buffer[i] == SearchB)
			return(i);
	return(-1);
}

#pragma optimize("", off) // for benchmarks

int main(int argc, char* argv[]){
	// pos of matched byte
	int pos = (argc >= 2) ? atoi(argv[1]) : bufferSize - 1;
	// shift from buffer start... make unaligned start
	int shift = (argc >= 3) ? atoi(argv[2]) : 0;

	if(argc < 2){
		printf("%s [pos] [shift]\n", argv[0]);
		printf("pos - pos of matched byte (0..%d - shift, default: %d)\n"
			, bufferSize - 1, bufferSize - 1);
		printf("shift - shift from buffer start (0..%d, default: 0)\n\n\n",
			bufferSize - 1);
	}

	unsigned char* TestBuffer = new unsigned char[bufferSize + 1];
	memset(TestBuffer, 0, bufferSize);
	TestBuffer[pos + shift] = 1;
	
	// check results + prepare CPU for work
	int iNaive = naive(&TestBuffer[shift], bufferSize - shift, 1);
	int iSSE = fastFindByte(&TestBuffer[shift], bufferSize - shift, 1);
	bool isOk = iNaive == iSSE;
	printf("Status: %s (%d | %d)\n\n", isOk ? "ok" : "error", iNaive, iSSE);

	if(isOk){
		unsigned __int64 t1, t2;
	
		t1 = __rdtsc();
		for(int i = 0; i < repeats; i++)
			naive(&TestBuffer[shift], bufferSize - shift, 1);
		t1 = __rdtsc() - t1;

		t2 = __rdtsc();
		for(int i = 0; i < repeats; i++)
			fastFindByte(&TestBuffer[shift], bufferSize - shift, 1);
		t2 = __rdtsc() - t2;

		cout << "Naive loop search = " << t1 / repeats << "\n";
		cout << "SSE search = " << t2 / repeats << "\n";
		cout << "Naive/sse = " << (float)((float)t1 / t2) << "\n";
	}
	
	delete TestBuffer;
	return 0;
}
#pragma optimize("", on)