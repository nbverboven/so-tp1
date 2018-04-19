#include "ConcurrentHashMap.hpp"

/****** Constructor y Destructor **********/

ConcurrentHashMap::ConcurrentHashMap(){
	tabla = new Lista<pair<string, unsigned int>>[26];
}


ConcurrentHashMap::ConcurrentHashMap(const ConcurrentHashMap &chm){
	tabla = new Lista<pair<string, unsigned int>>[26];
	for (int i = 0; i < 26; ++i){
		Lista< pair<string, unsigned int> >::Iterador it = chm.tabla[i].CrearIt();
		while(it.HaySiguiente()){
			for(unsigned int l=0; l<it.Siguiente().second; ++l)
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
			for(unsigned int l=0; l<it.Siguiente().second; ++l)
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


typedef struct info_maximum_no_static_str{
	atomic<int> *actual;
	mutex mtx;
	unsigned int cant_threads;
	vector<pair<string, unsigned int>> *resultados;
	ConcurrentHashMap *hash_map;
} info_maximum_no_static;


void *masAparicionesPorFila(void *info){
	info_maximum_no_static *datos = (info_maximum_no_static *) info;
	int i;

	/* Uso esta variable para evitar el caso de que un thread 
	   accede siempre primero a la variable atómica que indica 
	   la próxima fila. Si esto pasara, hay uno solo que hace 
	   la mayor parte del trabajo. */
	unsigned int kill_switch = 26/(datos->cant_threads) + 1;
	while(kill_switch > 0 && (i = datos->actual->fetch_add(1)) < 26){
		// Busco el elemento con más apariciones en la fila i
		pair<string, unsigned int> elem;
		Lista<pair<string, unsigned int>>::Iterador it = ((datos->hash_map)->tabla[i]).CrearIt();

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
			datos->mtx.lock();

			/* Actualizo la lista global de resultados con el de la fila
			   que revisé */
			pair<string, unsigned int> el_que_mas_aparece(elem.first, elem.second);
			(datos->resultados)->push_back(el_que_mas_aparece);
			datos->mtx.unlock();
		}
		--kill_switch;
	}
	return NULL;
}


pair<string, unsigned int> ConcurrentHashMap::maximum(unsigned int nt){
	vector<pthread_t> threads(nt);
	unsigned int tid;
	atomic<int> fila_actual(0);
	vector<pair<string, unsigned int>> resultados_filas;

	info_maximum_no_static datos_para_thread;
	datos_para_thread.resultados = &resultados_filas;
	datos_para_thread.hash_map = this;
	datos_para_thread.actual = &fila_actual;
	datos_para_thread.cant_threads = nt;

	for (tid = 0; tid < nt; ++tid){
		pthread_create(&threads[tid], NULL, masAparicionesPorFila, &datos_para_thread);
	}

	/* Espero a que todos terminen antes de seguir */
	for (tid = 0; tid < nt; ++tid){
		pthread_join(threads[tid], NULL);
	}

	/* Busco el elemento que más aparece posta posta.
	   Si estoy llamando a esta función, asumo que existe
	   alguno. */
	pair<string, unsigned int> solucion = resultados_filas[0];
	for (unsigned int k = 1; k < resultados_filas.size(); ++k){
		if ( resultados_filas[k].second > solucion.second ){
			solucion = resultados_filas[k];
		}
	}
	return solucion;
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
	thread_data_countWords therads_data[cantThreads];
	int tresp;

	for (int tid = 0; tid < cantThreads; ++tid){
		therads_data[tid].thread_id = tid;
		therads_data[tid].file = archs.front();
		archs.pop_front();
		therads_data[tid].hash_map = &dicc;

		tresp = pthread_create(&threads[tid], NULL, ConcurrentHashMap::CountWordsByFile, (void *)&therads_data[tid]);

      if (tresp) {
         exit(-1);
      }
	}

	for (int tid = 0; tid < cantThreads; ++tid){
		tresp = pthread_join(threads[tid], NULL);
		if (tresp) {
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

	thread_data_countWords_mutex therads_data;
	therads_data.files = archs;
	therads_data.hash_map = &dicc;

	// Initialize and set thread joinable
   	pthread_attr_init(&attr);
   	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	for (int tid = 0; tid < cantThreads; ++tid){
		tresp = pthread_create(&threads[tid], NULL, ConcurrentHashMap::CountWordsByFileList, (void *)&therads_data);
		if (tresp) {
			exit(-1);
		}
	}

	// free attribute and wait for the other threads
	pthread_attr_destroy(&attr);
	for (int tid = 0; tid < cantThreads; ++tid){
		tresp = pthread_join(threads[tid], &status);
		if (tresp) {
			exit(-1);
		}

	}
   	
	return dicc;
}


typedef struct info_maximum_static_read_str
{
	atomic<int> *actual;
	mutex mtx;
	vector<ConcurrentHashMap> *resultados;
	vector<string> archivos_a_leer;
	unsigned int cant_threads;
} info_maximum_static_read;


void *leoArchivos(void *info){
	info_maximum_static_read *datos = (info_maximum_static_read *) info;
	int i;
	int cant_archivos = datos->archivos_a_leer.size();
	unsigned int kill_switch = cant_archivos/(datos->cant_threads) + 1;
	ConcurrentHashMap h;

	while (kill_switch > 0 && (i = datos->actual->fetch_add(1)) < cant_archivos){
		string str = datos->archivos_a_leer[i];

		/* Creo un HashMap temporal para guardar la información del
		   archivo que procesé */
		ConcurrentHashMap g = ConcurrentHashMap::count_words(str);

		// Copio los elementos de g en h
		for (int i = 0; i < 26; ++i){
			Lista< pair<string, unsigned int> >::Iterador it = g.tabla[i].CrearIt();
			while(it.HaySiguiente()){
				for(unsigned int l=0; l<it.Siguiente().second; ++l)
					h.addAndInc(it.Siguiente().first);
				it.Avanzar();
			}
		}

		--kill_switch;
	}

	datos->mtx.lock();
	/* Actualizo la lista global de resultados con el del archivo
	   que revisé */
	(datos->resultados)->push_back(h);
	datos->mtx.unlock();
	return NULL;
}


pair<string, unsigned int> ConcurrentHashMap::maximum(unsigned int p_archivos, 
	                                                  unsigned int p_maximos, 
	                                                  list<string> archs){
	vector<pthread_t> threads_leyendo_archivos(p_archivos);
	vector<ConcurrentHashMap> archivos_leidos;
	atomic<int> siguiente_archivo(0);

	/* Copio los elementos de archs en un vector para reducir
	   el costo de acceder a cada posición cuando los threads
	   los procesen */
	vector<string> archivos_fuente;
	list<string>::iterator it;
	for (it = archs.begin(); it != archs.end(); ++it){
		archivos_fuente.push_back(*it);
	}

	unsigned int tid;

	info_maximum_static_read thread_data_read;
	thread_data_read.actual = &siguiente_archivo;
	thread_data_read.resultados = &archivos_leidos;
	thread_data_read.archivos_a_leer = archivos_fuente;
	thread_data_read.cant_threads = p_archivos;

	// Primero proceso todos los archivos
	for (tid = 0; tid < p_archivos; ++tid){
		pthread_create(&threads_leyendo_archivos[tid], NULL, leoArchivos, &thread_data_read);
	}

	// Espero a que todos terminen antes de seguir 
	for (tid = 0; tid < p_archivos; ++tid){
		pthread_join(threads_leyendo_archivos[tid], NULL);
	}

	// h va a contener toda la información de archivos_leidos
	ConcurrentHashMap h;
	for (uint k = 0; k < archivos_leidos.size(); ++k){
		h.agregarTodosLosElem(archivos_leidos[k]);
	}

	// Calculo el máximo de la combinación de HashMaps
	pair<string, unsigned int> solucion = h.maximum(p_maximos);
	
	return solucion;
}


void ConcurrentHashMap::agregarTodosLosElem(const ConcurrentHashMap &otro)
{
	for (int i = 0; i < 26; ++i){
		Lista< pair<string, unsigned int> >::Iterador it = otro.tabla[i].CrearIt();
		while(it.HaySiguiente()){
			for(unsigned int l=0; l<it.Siguiente().second; ++l)
				addAndInc(it.Siguiente().first);
			it.Avanzar();
		}
	}
}