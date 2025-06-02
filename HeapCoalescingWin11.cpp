// HeapCoalescingWin11.cpp : Ten plik zawiera funkcję „main”. W nim rozpoczyna się i kończy wykonywanie programu.
//

#include <iostream>
#include <Windows.h>

int main()
{

    const int ch_size = 0x110;//0x100 - smallest one, relatively stable;// 0x90 - that's what we need;

    void* be_heaps[11] = { 0 };

    for (int i = 0; i < 10; i++) {

        be_heaps[i] = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ch_size);
        if (be_heaps[i] == NULL) {
            std::cout << "HeapAlloc fail\r\n";
            return -1;
        }
        std::cout << i<<"]. allocated chunk size " << std::hex <<ch_size<<" 0x" << be_heaps[i] << "\r\n";

    }

    void* coala = be_heaps[1];
    std::cout << "Coalescing heap from index 1 - 3: " <<"0x"<< be_heaps[1] << "\r\n";

    HeapFree(GetProcessHeap(), 0, be_heaps[1]);
    HeapFree(GetProcessHeap(), 0, be_heaps[2]);
    HeapFree(GetProcessHeap(), 0, be_heaps[3]);

    std::cout << "Reallocating chunk of size: 0x"<<ch_size<<" * 3\r\n";
    for (int i = 0; i < 5; i++) {
         void* heap = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ch_size * 3);
         if (heap == coala) {
             std::cout << "You win, You are the best of the best ! We rellocated 3 times bigger chunk at memory:0x"<<coala<<"\r\n";
         } 
         std::cout << "reallocated heap: 0x" << heap << "\r\n";
    }

}

// Uruchomienie programu: Ctrl + F5 lub menu Debugowanie > Uruchom bez debugowania
// Debugowanie programu: F5 lub menu Debugowanie > Rozpocznij debugowanie

// Porady dotyczące rozpoczynania pracy:
//   1. Użyj okna Eksploratora rozwiązań, aby dodać pliki i zarządzać nimi
//   2. Użyj okna programu Team Explorer, aby nawiązać połączenie z kontrolą źródła
//   3. Użyj okna Dane wyjściowe, aby sprawdzić dane wyjściowe kompilacji i inne komunikaty
//   4. Użyj okna Lista błędów, aby zobaczyć błędy
//   5. Wybierz pozycję Projekt > Dodaj nowy element, aby utworzyć nowe pliki kodu, lub wybierz pozycję Projekt > Dodaj istniejący element, aby dodać istniejące pliku kodu do projektu
//   6. Aby w przyszłości ponownie otworzyć ten projekt, przejdź do pozycji Plik > Otwórz > Projekt i wybierz plik sln
