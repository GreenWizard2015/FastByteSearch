#include "FastByteSearch.h"

typedef unsigned char BYTE;
typedef unsigned int _UINT;

__forceinline int 
SSE_findByte_alligned32(BYTE* buffer, _UINT ChunksBy32, _UINT EXPANDED_MASK){
	__asm{
		PUSH	 ESI
		PUSH	 EBX

		MOV		 EDX, buffer
		MOV		 ECX, ChunksBy32
		MOV		 EAX, 0 // Pos

		MOVD     XMM0, EXPANDED_MASK
		SHUFPS   XMM0, XMM0, 0 // XMM0 = SearchB | ... | SearchB

		MOVAPS   XMM6, [EDX]
		MOVAPS   XMM7, [EDX + 16]
LOOP_START:
		PCMPEQB  XMM6, XMM0
		PCMPEQB  XMM7, XMM0

		ADD		 EAX, 32 // pos += 32
		PMOVMSKB ESI, XMM6
		PMOVMSKB EBX, XMM7

		ADD		 EDX, 32
		SHL		 EBX, 16

		DEC		 ECX // ChunksBy32--
		JZ		 TAIL

		MOVAPS   XMM6, [EDX]
		MOVAPS   XMM7, [EDX + 16]

		OR       ESI, EBX
		JNZ      TAIL_2
		JMP      LOOP_START

TAIL:
		OR       ESI, EBX
TAIL_2:
		MOV		 EDX, -1 + 32
		BSF      EBX, ESI            // Scan bits for 1
		CMOVZ	 EAX, EDX // -1
		ADD		 EAX, EBX
		SUB		 EAX, 32
		
		POP		 EBX
		POP		 ESI
	}
}

int fastFindByte(void* _buffer, int bSize, unsigned char SearchB){
	BYTE* buffer = (BYTE*)_buffer;
	if(bSize < 16) {
		// naive loop for small data
		for(int i = 0; i < bSize; ++i, ++buffer)
			if(*buffer == SearchB)
				return(i);
		return(-1);
	}

	BYTE* alligned = (BYTE*)((_UINT)(buffer + 0xF) & ~0xF);
	int headN = alligned - buffer;

	// check first unaligned bytes
	if(headN > 0){
		for(int i = 0; i < headN; ++i, ++buffer)
			if(*buffer == SearchB)
				return(i);

		bSize -= headN;
		buffer = alligned;
	}

	_UINT Pos = headN;
	_UINT ChunksBy32 = bSize >> 5; // floor(bSize / 32)
	if(ChunksBy32 > 0) {
		_UINT EXPAND_MASK = SearchB | (SearchB << 8);
		EXPAND_MASK = EXPAND_MASK | (EXPAND_MASK << 16);

		int res = SSE_findByte_alligned32(buffer, ChunksBy32, EXPAND_MASK);
		if(res >= 0)
			return(Pos + res);

		_UINT tmp = ChunksBy32 << 5;
		bSize -= tmp;
		buffer += tmp;
		Pos += tmp;
	}

	// check last bytes
	for(int i = 0; i < bSize; ++i, ++buffer)
		if(*buffer == SearchB)
			return(i + Pos);
	return(-1);
}