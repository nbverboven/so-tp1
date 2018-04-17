#include "ConcurrentHashMap.hpp"

/****** Constructor y Destructor **********/

ConcurrentHashMap::ConcurrentHashMap() : cant_elementos(0) {
	tabla = new Lista< pair<string, unsigned int> >[26];
}

ConcurrentHashMap::ConcurrentHashMap(ConcurrentHashMap &chm) : cant_elementos(chm.cant_elementos) {
	tabla = new Lista< pair<string, unsigned int> >[26];
}

ConcurrentHashMap::~ConcurrentHashMap(){
	delete[] tabla;
}


/************ Metodos *************/

void ConcurrentHashMap::addAndInc(string key){
	int _hash = Hash(key);
	Lista< pair<string, unsigned int> >::Iterador it;
	bool encontrado = false;

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
}


bool ConcurrentHashMap::member(string key){
	int _hash = Hash(key);
	Lista< pair<string, unsigned int> >::Iterador it;
	bool encontrado = false;

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