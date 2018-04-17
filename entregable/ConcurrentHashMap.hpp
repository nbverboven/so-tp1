#ifndef CONCURRENT_HASH_MAP_HPP
#define CONCURRENT_HASH_MAP_HPP

#include <atomic>
#include <utility>
#include <string>
#include <list>
#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>

#include "ListaAtomica.hpp"

using namespace std;

class ConcurrentHashMap
{
public:

	Lista< pair<string, unsigned int> >* tabla;

	ConcurrentHashMap();
	ConcurrentHashMap(ConcurrentHashMap &chm);
	~ConcurrentHashMap();

	/*
		Si key existe, incrementa su valor. Si no, crea
		el par (key, 1). La función de hash es la 
		primer letra de key.

		-- Solo existe contención en caso de colisión --
		      (locking a nivel de cada elemento)
	*/
	void addAndInc(string key);

	/*
		Devuelve true sii el par (key,x) pertenece
		al hash map para algún x.

	 	-- Wait free --
	*/
	bool member(string key);

	/*
		Devuelve el par (k,m) tal que k es la clave con
		máxima cantidad de apariciones y m es ese valor.
		El parámetro nt indica la cantidad de threads a 
		utilizar. Los threads procesarán una fila del
		arreglo. Si no tienen filas, terminan su ejecución.
		`
		-- Es concurrente con member, no así con addAndInc --
	*/
	pair<string, unsigned int> maximum(unsigned int nt);

	static ConcurrentHashMap count_words(string arch);
	static ConcurrentHashMap count_words(list<string> archs);
	static ConcurrentHashMap count_words(unsigned int n, list<string> archs);

protected:

	int cant_elementos;

	int Hash (const string& str) const {
		int hash = (int)(str[0]) % 26;
		return hash;
	}

};

#endif // CONCURRENT_HASH_MAP_HPP