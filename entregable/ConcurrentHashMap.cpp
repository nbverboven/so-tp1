#include "ConcurrentHashMap.hpp"

/****** Constructor y Destructor **********/

ConcurrentHashMap::ConcurrentHashMap(){
	tabla = new Lista<pair<string, unsigned int>>[26];
}


ConcurrentHashMap::ConcurrentHashMap(ConcurrentHashMap &chm){
	tabla = new Lista<pair<string, unsigned int>>[26];
	for (int i = 0; i < 26; ++i){
		Lista< pair<string, unsigned int> >::Iterador it = chm.tabla[i].CrearIt();
		while(it.HaySiguiente()){
			addAndInc(it.Siguiente().first);
			it.Avanzar();
		}
	}
}


ConcurrentHashMap::~ConcurrentHashMap(){
	delete[] tabla;
}


/****** Operadores **********/

ConcurrentHashMap& ConcurrentHashMap::operator=(const ConcurrentHashMap &chm){
	Lista< pair<string, unsigned int> > *tabla_vieja = tabla;
	tabla = new Lista<pair<string, unsigned int>>[26];
	for (int i = 0; i < 26; ++i){
		Lista< pair<string, unsigned int> >::Iterador it = chm.tabla[i].CrearIt();
		while(it.HaySiguiente()){
			for(int l=0; l<it.Siguiente().second; ++l)
				addAndInc(it.Siguiente().first);
			it.Avanzar();
		}
	}
	delete[] tabla_vieja;
	return *this;
}


/************ Metodos *************/

void ConcurrentHashMap::addAndInc(string key){
	int _hash = Hash(key);
	Lista<pair<string, unsigned int>>::Iterador it = tabla[_hash].CrearIt();
	bool encontrado = false;
	while(it.HaySiguiente() && !encontrado){
		if(it.Siguiente().first == key){
			++it.Siguiente().second;
			encontrado = true;
		}
		it.Avanzar();
	}
	if(!encontrado){
		tabla[_hash].push_front(make_pair(key,1));
		// cant_elementos++;
	}
}


bool ConcurrentHashMap::member(string key){
	int _hash = Hash(key);
	Lista<pair<string, unsigned int>>::Iterador it = tabla[_hash].CrearIt();
	bool encontrado = false;
	while(it.HaySiguiente() && !encontrado){
		if(it.Siguiente().first == key){
			encontrado = true;
		}
		it.Avanzar();
	}
	return encontrado;
}


// TODO: Pensar otro nombre
typedef struct info_aux_nico_str
{
	vector<pair<string, unsigned int>> *resultados;
	ConcurrentHashMap *hash_map;
	atomic<int> *prox_fila;

} info_aux_nico;


void *maximumEnFila_nico(void *info)
{
	info_aux_nico asd = *((info_aux_nico *) info);

	// Para que nadie se meta en la misma fila que yo
	int i = asd.prox_fila->fetch_add(1);

	// pthread_id_np_t tid;
	// tid = pthread_getthreadid_np();

	while ( i < 26 )
	{
	printf("Hola! Soy el thread %u. Estoy laburando en la fila: %d\n", (unsigned int) pthread_self(), i);
		// Busco el elemento con más apariciones en la fila i
		pair<string, unsigned int> elem;
		Lista<pair<string, unsigned int>>::Iterador it = ((asd.hash_map)->tabla[i]).CrearIt();

		// Quiero hacer esto solamente si la lista tiene elementos
		if (it.HaySiguiente())
		{
			elem = it.Siguiente();

			while ( it.HaySiguiente() )
			{
				if ( it.Siguiente().second > elem.second )
				{
					// Si encuentro una clave en la fila que se repite más,
					// actualizo la solución
					elem = it.Siguiente();
				}

				it.Avanzar();
			}

			// Actualizo la lista global de resultados con el de la fila
			// que revisé
			// pair<string, unsigned int> mas_apariciones(elem.first, elem.second);
			// (asd.resultados)->push_back(mas_apariciones);
		}

		// Si en la lista no había nada, el puntero de la lista de 
		// resultados queda en NULL
		i = asd.prox_fila->fetch_add(1);
	}

	return NULL;
}


pair<string, unsigned int> ConcurrentHashMap::maximum(unsigned int nt)
{
	vector<pthread_t> threads(nt);
	unsigned int tid;

	vector<pair<string, unsigned int>> results;

	atomic<int> fila_actual(0);

	info_aux_nico aux;
	aux.resultados = &results;
	aux.hash_map = this;
	aux.prox_fila = &fila_actual;

	for (tid = 0; tid < nt; ++tid)
	{
		pthread_create(&threads[tid], NULL, maximumEnFila_nico, &aux);
	}

	for (tid = 0; tid < nt; ++tid)
	{
		pthread_join(threads[tid], NULL);
	}

	/* Busco el elemento que más aparece posta posta.
	   Si estoy llamando a esta función, asumo que existe
	   alguno. */
	// pair<string, unsigned int> res = results[0];

	// for (unsigned int k = 1; k < results.size(); ++k)
	// {
	// 	if ( results[k].second > res.second )
	// 	{
	// 		res = results[k];
	// 	}
	// }

	pair<string, unsigned int> res("a",3);

	return res;
}


/************ Metodos estaticos *************/

/*
	Devuelve el ConcurrentHashMap cargado con las
	palabras (entendiéndose como tales las separadas 
	por espacios) del archivo arch.
	No concurrente.
*/
ConcurrentHashMap ConcurrentHashMap::count_words(string arch){
	ConcurrentHashMap dicc;
	string line;
	ifstream file(arch);
	if (file.is_open()){
		while (getline (file,line)){
			string word;
			istringstream buf(line);
			while(buf >> word){
				dicc.addAndInc(word);
			}
		}
		file.close();
	}
	return dicc;
}


typedef struct thread_data_countWords
{
	int thread_id;
	string file;
	ConcurrentHashMap* hash_map;

} thread_data_countWords;

void* CountWordsByFile_andy_mejorQueEsaFuncionBasuraDeNico(void *arguments){
	struct thread_data_countWords *thread_data;
   	thread_data = (struct thread_data_countWords *) arguments;

	string line;
	ifstream file(thread_data->file);
	if (file.is_open()){
		while (getline (file,line)){
			string word;
			istringstream buf(line);
			while(buf >> word)
				thread_data->hash_map->addAndInc(word);
		}
		file.close();
	}

}

/*
	Devuelve el ConcurrentHashMap cargado con las
	palabras (entendiéndose como tales las separadas 
	por espacios) de cada archivo de la lista que se 
	pasa por parámetro.
	Un thread por archivo.
*/
ConcurrentHashMap ConcurrentHashMap::count_words(list<string> archs){
	ConcurrentHashMap dicc;
	int cantThreads = archs.size();
	vector<pthread_t> threads(cantThreads);
	struct thread_data_countWords therads_data[cantThreads];

	for (int tid = 0; tid < cantThreads; ++tid){
		therads_data[tid].thread_id = tid;
		therads_data[tid].file = archs.front();
		archs.pop_front();
		therads_data[tid].hash_map = &dicc;

		pthread_create(&threads[tid], NULL, CountWordsByFile_andy_mejorQueEsaFuncionBasuraDeNico, (void *)&therads_data[tid]);
	}

	for (int tid = 0; tid < cantThreads; ++tid){
		pthread_join(threads[tid], NULL);
	}

	return dicc;
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
	ConcurrentHashMap dicc;
	return dicc;
}


pair<string, unsigned int> ConcurrentHashMap::maximum(unsigned int p_archivos, 
	                               unsigned int p_maximos, 
	                               list<string> archs){
	pair<string, unsigned int> asd("tujavie", p_archivos);
	return asd;
}