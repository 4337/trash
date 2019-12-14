#include <stdio.h>
#include <Windows.h>

const unsigned int BE_HEAP = 8;
void xAllocator();

void xAllocator() {
	 
	const unsigned int alloc_size = 0x38;
	unsigned char* alloc_hnd[BE_HEAP] = { nullptr };
	unsigned char* alloc_copy[BE_HEAP] = { nullptr };

	HANDLE proc_heap = GetProcessHeap();

	for (unsigned int i = 0; i < BE_HEAP; i++) {

		  alloc_hnd[i] = (unsigned char*)HeapAlloc(proc_heap, HEAP_ZERO_MEMORY, 0x38 + 0x20);
		  printf("heap %d : 0x%p\r\n", i, alloc_hnd[i]);
	}

	printf("Free chunk index 4 : 0x%p\r\n", alloc_hnd[4]);

	HeapFree(proc_heap, 0, alloc_hnd[4]);
	//alloc_hnd[4] = nullptr;

	/*unsigned char* p = (unsigned char*)HeapAlloc(proc_heap, HEAP_ZERO_MEMORY, 0x38);*/

	printf("----------------------------\r\n");

	for (unsigned int i = 0; i < BE_HEAP; i++) {

		alloc_copy[i] = (unsigned char*)HeapAlloc(proc_heap, HEAP_ZERO_MEMORY, 0x38 + 0x20);
		if (alloc_hnd[4] == alloc_copy[i]) {
			printf("You must avoid LFH otherwise you'll be unhappy forever : new chunk allocated at freed mem 0x%p you win be happy now\r\n", alloc_copy[i]);
		}
		printf("heap %d : 0x%p\r\n", i, alloc_copy[i]);
	}

	/*printf("Alloc new chunk : 0x%x\r\n", alloc_hnd[4]);*/


}

int main() {

	xAllocator();
	return 0;
}