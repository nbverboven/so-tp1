#include <iostream>
#include <cstdlib>
#include "../entregable/ConcurrentHashMap.hpp"

using namespace std;

int main(int argc, char **argv) {
	ConcurrentHashMap h;
	list<string> l = { "lorem-ipsum-1", "lorem-ipsum-2", "lorem-ipsum-3", "lorem-ipsum-4", "lorem-ipsum-5"};
    list<string> l_ext = { "lorem-ipsum-ext-1", "lorem-ipsum-ext-2", "lorem-ipsum-ext-3", "lorem-ipsum-ext-4", "lorem-ipsum-ext-5", "lorem-ipsum-ext-6"};
	int i;

	if (argc != 2) {
		cerr << "uso: " << argv[0] << " #threads" << endl;
		return 1;
	}
	
	h = ConcurrentHashMap::count_words(atoi(argv[1]), l_ext);
	//h = ConcurrentHashMap::count_words(2, l);
	//h = ConcurrentHashMap::count_words(l);
	for (i = 0; i < 26; i++) {
        cout<< i <<endl;
		for (auto it = h.tabla[i].CrearIt(); it.HaySiguiente(); it.Avanzar()) {
			auto t = it.Siguiente();
			cout << t.first << " " << t.second << endl;
		}
	}

	return 0;
}

