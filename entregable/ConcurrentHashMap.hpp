#ifndef CONCURRENT_HASH_MAP_HPP
#define CONCURRENT_HASH_MAP_HPP

#include <pthread.h>
#include <atomic>
#include <mutex>
#include <condition_variable>
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
	Lista<pair<string, unsigned int>> *tabla;

	ConcurrentHashMap();
	ConcurrentHashMap(const ConcurrentHashMap &chm);
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
		
		-- Es concurrente con member, no así con addAndInc --
	*/
	pair<string, unsigned int> maximum(unsigned int nt);

	/*
		Devuelve el ConcurrentHashMap cargado con las
		palabras (entendiéndose como tales las separadas 
		por espacios) del archivo arch.
		
		-- No concurrente --
	*/
	static ConcurrentHashMap count_words(string arch);

	/*
		Devuelve el ConcurrentHashMap cargado con las
		palabras (entendiéndose como tales las separadas 
		por espacios) de cada archivo de la lista que se 
		pasa por parámetro.
		
		-- Un thread por archivo --
	*/
	static ConcurrentHashMap count_words(list<string> archs);

	/* 
		Devuelve el ConcurrentHashMap cargado con las
		palabras (entendiéndose como tales las separadas 
		por espacios) de cada archivo de la lista que se 
		pasa por parámetro utilizando n threads.
		n puede ser menor que la cantidad de archivos. 
		No hay threads sin trabajo mientras queden archivos 
		sin procesar.
	*/
	static ConcurrentHashMap count_words(unsigned int n, list<string> archs);

	static pair<string, unsigned int> maximum(unsigned int p_archivos, unsigned int p_maximos, list<string>archs);
	static pair<string, unsigned int> maximum2(unsigned int p_archivos, unsigned int p_maximos, list<string>archs);
	static pair<string, unsigned int> maximum3(unsigned int p_archivos, unsigned int p_maximos, list<string>archs);

	ConcurrentHashMap& operator=(const ConcurrentHashMap &otro);

protected:
	mutex addAndInc_mtx;

	int Hash (const string& str){
		int hash = (int)(str[0]) % 26;
		return hash;
	}

	/* Estructuras utilizadas por maximum y count_words */
	typedef struct thread_data_countWords {
		int thread_id;
		string file;
		ConcurrentHashMap* hash_map;
	} thread_data_countWords;

	typedef struct thread_data_countWords_list {
		list<string> files;
		atomic<int> actual;
		ConcurrentHashMap* hash_map;
	} thread_data_countWords_list;

	typedef struct info_maximum_no_static_str{
		atomic<int> *actual;
		mutex mtx;
		unsigned int cant_threads;
		vector<pair<string, unsigned int>> *resultados;
		ConcurrentHashMap *hash_map;
	} info_maximum_no_static;

	typedef struct info_maximum_static_read_str
	{
		atomic<int> *actual;
		mutex mtx;
		ConcurrentHashMap *resultados;
		vector<string> archivos_a_leer;
		unsigned int cant_threads;
	} info_maximum_static_read;

	static void *CountWordsByFile(void *arguments);
	static void *CountWordsByFileList(void *arguments);
	static void *leoArchivos(void *info);
	static void *masAparicionesPorFila(void *info);

private:
	/* Agrega todos los elementos de otro por copia */
	void agregarTodosLosElem(const ConcurrentHashMap &otro);
};

#endif // CONCURRENT_HASH_MAP_HPP