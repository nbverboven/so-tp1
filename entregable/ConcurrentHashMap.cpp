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
			addAndInc_mtx.lock();
			++it.Siguiente().second;
			addAndInc_mtx.unlock();
			encontrado = true;
		}
		it.Avanzar();
	}

	if(!encontrado){
		tabla[_hash].push_front(make_pair(key,1));
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
typedef struct info_aux_nico_str{
	atomic<int> *actual;
	mutex *str_mtx;
	unsigned int cant_threads;
	vector<pair<string, unsigned int>> *resultados;
	ConcurrentHashMap *hash_map;
} info_aux_nico;


void *masAparicionesPorFila(void *info){
	info_aux_nico *asd = (info_aux_nico *) info;

	// Para que nadie se meta en la misma fila que yo
	int i;

	/* Uso esta variable para evitar el caso de que un thread 
	   accede siempre primero a la variable atómica que indica 
	   la próxima fila. Si esto pasara, hay uno solo que hace 
	   la mayor parte del trabajo. */
	unsigned int kill_switch = 26/(asd->cant_threads) + 1;
	while(kill_switch > 0 && (i = asd->actual->fetch_add(1)) < 26){
		// Busco el elemento con más apariciones en la fila i
		pair<string, unsigned int> elem;
		Lista<pair<string, unsigned int>>::Iterador it = ((asd->hash_map)->tabla[i]).CrearIt();

		// Quiero hacer esto solamente si la lista tiene elementos
		if (it.HaySiguiente()){
			elem = it.Siguiente();
			while (it.HaySiguiente()){
				if (it.Siguiente().second > elem.second){

					// Si encuentro una clave en la fila que se repite más,
					// actualizo la solución
					elem = it.Siguiente();
				}
				it.Avanzar();
			}

			/* Bloqueo la ejecución del resto de los threads que
			   quieran modificar resultados al mismo tiempo que yo.
			   Evito que se produzca una condición de carrera. */
			asd->str_mtx->lock();

			/* Actualizo la lista global de resultados con el de la fila
			   que revisé */
			pair<string, unsigned int> el_que_mas_aparece(elem.first, elem.second);
			(asd->resultados)->push_back(el_que_mas_aparece);
			asd->str_mtx->unlock();
		}
		--kill_switch;
	}
	return NULL;
}


pair<string, unsigned int> ConcurrentHashMap::maximum(unsigned int nt){
	vector<pthread_t> threads(nt);
	unsigned int tid;
	atomic<int> fila_actual(0);
	mutex mtx;
	vector<pair<string, unsigned int>> results;
	info_aux_nico aux;
	aux.resultados = &results;
	aux.hash_map = this;
	aux.actual = &fila_actual;
	aux.str_mtx = &mtx;
	aux.cant_threads = nt;
	for (tid = 0; tid < nt; ++tid){
		pthread_create(&threads[tid], NULL, masAparicionesPorFila, &aux);
	}

	/* Espero a que todos terminen antes de seguir */
	for (tid = 0; tid < nt; ++tid){
		pthread_join(threads[tid], NULL);
	}

	/* Busco el elemento que más aparece posta posta.
	   Si estoy llamando a esta función, asumo que existe
	   alguno. */
	pair<string, unsigned int> res = results[0];
	for (unsigned int k = 1; k < results.size(); ++k){
		if ( results[k].second > res.second ){
			res = results[k];
		}
	}
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

void* ConcurrentHashMap::CountWordsByFile(void *arguments){
	struct ConcurrentHashMap::thread_data_countWords *thread_data;
   	thread_data = (struct ConcurrentHashMap::thread_data_countWords *) arguments;

	//cout << "Thread with id : " << thread_data->thread_id << endl;

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

	pthread_exit(NULL);
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
	int tresp;


	for (int tid = 0; tid < cantThreads; ++tid){
		therads_data[tid].thread_id = tid;
		therads_data[tid].file = archs.front();
		archs.pop_front();
		therads_data[tid].hash_map = &dicc;

		tresp = pthread_create(&threads[tid], NULL, ConcurrentHashMap::CountWordsByFile, (void *)&therads_data[tid]);

      if (tresp) {
         //cout << "Error:unable to create thread, " << tresp << endl;
         exit(-1);
      }

	}

	for (int tid = 0; tid < cantThreads; ++tid){
		tresp = pthread_join(threads[tid], NULL);
		if (tresp) {
			//cout << "Error:unable to join," << tresp << endl;
         	exit(-1);
      }
	}

	return dicc;
}

void* ConcurrentHashMap::CountWordsByFileList(void *arguments){
	struct ConcurrentHashMap::thread_data_countWords_mutex *thread_data;
   	thread_data = (struct ConcurrentHashMap::thread_data_countWords_mutex *) arguments;
	string filename;
	bool hasFile = true;
	//cout << "Thread with id : " << pthread_self() << endl;

	while(hasFile){
		thread_data->getNextFile.lock(); //Mutex, entro zona critica
		if(thread_data->files.empty()){
			hasFile = false;
		}else {
			filename = thread_data->files.front();
			thread_data->files.pop_front();
		}
		thread_data->getNextFile.unlock(); //Mutex, salgo zona critica

		if(hasFile){
			//cout << "Thread with id:" << pthread_self() << " use file: " << filename << endl;
			string line;
			ifstream file(filename);
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
	}

	pthread_exit(NULL);
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
	int cantThreads = n;
	if(archs.size() < n)
		cantThreads = archs.size();
	vector<pthread_t> threads(cantThreads);
	int tresp;
	pthread_attr_t attr;
   	void *status;

	struct thread_data_countWords_mutex therads_data;
	therads_data.files = archs;
	therads_data.hash_map = &dicc;

	// Initialize and set thread joinable
   	pthread_attr_init(&attr);
   	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	for (int tid = 0; tid < cantThreads; ++tid){
		//cout << "main() : creating thread, " << tid << endl;
		tresp = pthread_create(&threads[tid], NULL, ConcurrentHashMap::CountWordsByFileList, (void *)&therads_data);
		if (tresp) {
			//cout << "Error:unable to create thread, " << tresp << endl;
			exit(-1);
		}
	}

	// free attribute and wait for the other threads
	pthread_attr_destroy(&attr);
	for (int tid = 0; tid < cantThreads; ++tid){
		tresp = pthread_join(threads[tid], &status);
		if (tresp) {
			//cout << "Error:unable to join," << tresp << endl;
			exit(-1);
		}

		//cout << "Main: completed thread id :" << tid << "  exiting with status :" << status << endl;
	}
	//cout << "Main: program exiting." << endl;
   	
	return dicc;
}


pair<string, unsigned int> ConcurrentHashMap::maximum(unsigned int p_archivos, 
	                               unsigned int p_maximos, 
	                               list<string> archs){
	pair<string, unsigned int> asd("tujavie", p_archivos);
	return asd;
}