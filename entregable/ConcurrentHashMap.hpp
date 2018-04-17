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

	int cant_elementos=0;

	int Hash (const string& str) const {
		int hash = (int)(str[0]) % 26;
		//for (int i=0 ; i < str.size() ; ++i)
		//	hash = ((hash*33) + str[i]) % 26;
		return hash;
	}

};


/****** Constructor y Destructor **********/
ConcurrentHashMap::ConcurrentHashMap(){
	tabla = new Lista< pair<string, unsigned int> >[26];
}

ConcurrentHashMap::ConcurrentHashMap(ConcurrentHashMap &chm){
	tabla = new Lista< pair<string, unsigned int> >[26];
}

ConcurrentHashMap::~ConcurrentHashMap(){
	delete [] tabla;
}


/************ Metodos *************/

void ConcurrentHashMap::addAndInc(string key){
	int _hash = Hash(key);
	Lista< pair<string, unsigned int> >::Iterador it;
	bool encontrado=false;

	for(it = tabla[_hash].CrearIt(); it.HaySiguiente(); it.Avanzar()){
		if(it.Siguiente().first == key){
			it.Siguiente().second++;
			encontrado = true;
		}
	}

	if(!encontrado){
		tabla[_hash].push_front( make_pair(key,1) );
		cant_elementos++;
	}

	return;
}


bool ConcurrentHashMap::member(string key){
	int _hash = Hash(key);
	Lista< pair<string, unsigned int> >::Iterador it;
	bool encontrado=false;

	for(it = tabla[_hash].CrearIt(); it.HaySiguiente(); it.Avanzar()){
		if(it.Siguiente().first == key)
			encontrado = true;
	}

	return encontrado;
}


pair<string, unsigned int> ConcurrentHashMap::maximum(unsigned int nt){
	pair<string, unsigned int> asd("tujavie", 2);
	return asd;
}


/************ Metodos estaticos *************/

/*
	Devuelve el ConcurrentHashMap cargado con las
	palabras (entendiéndose como tales las separadas 
	por espacios) del archivo arch.
	No concurrente.
*/
ConcurrentHashMap ConcurrentHashMap::count_words(string arch){
	ConcurrentHashMap* dicc = new ConcurrentHashMap();

	string line;
	ifstream file(arch);
	if (file.is_open()){
		while ( getline (file,line) ){
			string word;
			istringstream buf(line);
    		while(buf >> word){
				dicc->addAndInc(word);
			}
		}
		file.close();
	}

	return *dicc;
}


/*
	Devuelve el ConcurrentHashMap cargado con las
	palabras (entendiéndose como tales las separadas 
	por espacios) de cada archivo de la lista que se 
	pasa por parámetro.
	Un thread por archivo.
*/
ConcurrentHashMap ConcurrentHashMap::count_words(list<string> archs){
	return *(new ConcurrentHashMap());
}


/* 
	Devuelve el ConcurrentHashMap cargado con las
	palabras (entendiéndose como tales las separadas 
	por espacios) de cada archivo de la lista que se 
	pasa por parámetro utilizando n threads.
	n puede ser menor que la cantidad de archivos. 
	No hay threads sin trabajo mientras queden archivos 
	sin procesar.
*/
ConcurrentHashMap ConcurrentHashMap::count_words(unsigned int n, list<string> archs){
	return *(new ConcurrentHashMap());
}


pair<string, unsigned int> maximum(unsigned int p_archivos, 
	                               unsigned int p_maximos, 
	                               list<string> archs){
	pair<string, unsigned int> asd("tujavie", 2);
	return asd;
}

#endif // CONCURRENT_HASH_MAP_HPP