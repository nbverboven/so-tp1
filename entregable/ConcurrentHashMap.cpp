#include "ConcurrentHashMap.hpp"

/****** Constructor y Destructor **********/

ConcurrentHashMap::ConcurrentHashMap()
: cant_threads_addAndInc(0), cant_threads_maximum(0){
	tabla = new Lista<pair<string, unsigned int>>[26];
}


ConcurrentHashMap::ConcurrentHashMap(const ConcurrentHashMap &chm)
: cant_threads_addAndInc(0), cant_threads_maximum(0){
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

	/* Me fijo si el semáforo me indica que hay al menos
	   un thread ejecutando maximum. Si es así, espero
	   a que se liberen todos. */
	unique_lock<mutex> lck(maximum_mtx);
	int cuantos_en_maximum = cant_threads_maximum.load();

	while (cuantos_en_maximum != 0)
	{
		addAndInc_cond.wait(lck);
		cuantos_en_maximum = cant_threads_maximum.load();
	}

	++cant_threads_addAndInc;

	int hash = Hash(key);

	/* Bloqueo la fila en la que está el elemento que
	   quiero agregar */
	addAndInc_filas_mtx[hash].lock();

	Lista<pair<string, unsigned int>>::Iterador it = tabla[hash].CrearIt();
	bool encontrado = false;

	while(it.HaySiguiente() && !encontrado){
		if(it.Siguiente().first == key){
			++it.Siguiente().second;
			encontrado = true;
		}
		it.Avanzar();
	}

	if(!encontrado){
		tabla[hash].push_front(make_pair(key,1));
	}

	/* Libero el mutex de la fila dada por hash */
	addAndInc_filas_mtx[hash].unlock();

	/* Despierto a todos los que pudieran querer
	   ejecutar maximum */
	--cant_threads_addAndInc;
	maximum_cond.notify_all();
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


void *ConcurrentHashMap::masAparicionesPorFila(void *info){
	ConcurrentHashMap::info_maximum_no_static *datos = (ConcurrentHashMap::info_maximum_no_static *) info;
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

					/* Si encuentro una clave en la fila que se repite más,
					   actualizo la solución */
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

	/* Me fijo si el semáforo me indica que hay al menos
	   un thread ejecutando addAndInc. Si es así, espero
	   a que se liberen todos. */
	unique_lock<mutex> lck(addAndInc_mtx);
	int cuantos_en_addAndInc = cant_threads_addAndInc.load();

	while (cuantos_en_addAndInc != 0)
	{
		maximum_cond.wait(lck);
		cuantos_en_addAndInc = cant_threads_addAndInc.load();
	}

	++cant_threads_maximum;

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
		int status_create = pthread_create(&threads[tid], NULL, masAparicionesPorFila, &datos_para_thread);
		if (status_create) {
			printf("Error: unable to create thread\n");
			exit(-1);
		}
	}

	/* Espero a que todos terminen antes de seguir */
	for (tid = 0; tid < nt; ++tid){
		int status_join = pthread_join(threads[tid], NULL);
		if (status_join) {
			printf("Error: unable to join thread\n");
			exit(-1);
		}
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

	/* Despierto a todos los que pudieran querer
	   ejecutar addAndInc. */
	--cant_threads_maximum;
	addAndInc_cond.notify_all();

	return solucion;
}


/************ Metodos estaticos *************/

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
	ConcurrentHashMap::thread_data_countWords *thread_data;
   	thread_data = (ConcurrentHashMap::thread_data_countWords *) arguments;

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
	ConcurrentHashMap::thread_data_countWords_list *thread_data;
   	thread_data = (ConcurrentHashMap::thread_data_countWords_list *) arguments;
	string filename;
	bool hasFile = true;

	while(hasFile){
		uint localActual = thread_data->actual.fetch_add(1);
		if(localActual < thread_data->files.size()){
			list<string>::iterator itFiles = thread_data->files.begin();
			advance(itFiles, localActual); //Avanzo hasta el archivo localActual-esimo
			filename = (*itFiles);
		}else {
			hasFile = false;
		}

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


ConcurrentHashMap ConcurrentHashMap::count_words(unsigned int n, list<string> archs){
	ConcurrentHashMap dicc;
	int cantThreads = n;
	if(archs.size() < n)
		cantThreads = archs.size();
	vector<pthread_t> threads(cantThreads);
	int tresp;
	pthread_attr_t attr;
   	void *status;

	thread_data_countWords_list therads_data;
	therads_data.files = archs;
	therads_data.hash_map = &dicc;
	therads_data.actual = 0;

	// Initialize and set thread joinable
   	pthread_attr_init(&attr);
   	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	for (int tid = 0; tid < cantThreads; ++tid){
		tresp = pthread_create(&threads[tid], NULL, ConcurrentHashMap::CountWordsByFileList, (void *)&therads_data);
		if (tresp) {
			printf("Error: unable to create thread \n");
			exit(-1);
		}
	}

	// free attribute and wait for the other threads
	pthread_attr_destroy(&attr);
	for (int tid = 0; tid < cantThreads; ++tid){
		tresp = pthread_join(threads[tid], &status);
		if (tresp) {
			printf("Error: unable to join thread \n");
			exit(-1);
		}

	}
   	
	return dicc;
}


void *ConcurrentHashMap::leoArchivos(void *info){
	ConcurrentHashMap::info_maximum_static_read *datos = (ConcurrentHashMap::info_maximum_static_read *) info;
	int i;
	int cant_archivos = datos->archivos_a_leer.size();
	unsigned int kill_switch = cant_archivos/(datos->cant_threads) + 1;
	ConcurrentHashMap h;

	while (kill_switch > 0 && (i = datos->actual->fetch_add(1)) < cant_archivos){
		string arch = datos->archivos_a_leer[i];

		/* Guardo la información de arch en h */
		string line;
		ifstream file(arch);
		if (file.is_open()){
			while (getline (file,line)){
				string word;
				istringstream buf(line);
				while(buf >> word){
					h.addAndInc(word);
				}
			}
			file.close();
		}

		--kill_switch;
	}

	datos->mtx.lock();
	/* Actualizo el HashMap global de resultados con la información de los archivos
	   que revisé */
	(datos->resultados)->agregarTodosLosElem(h);
	datos->mtx.unlock();
	return NULL;
}


pair<string, unsigned int> ConcurrentHashMap::maximum(unsigned int p_archivos, 
	                                                  unsigned int p_maximos, 
	                                                  list<string> archs)
{
	vector<pthread_t> threads_leyendo_archivos(p_archivos);
	ConcurrentHashMap archivos_leidos;
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
		int status_create = pthread_create(&threads_leyendo_archivos[tid], NULL, leoArchivos, &thread_data_read);
		if (status_create) {
			printf("Error: unable to create thread\n");
			exit(-1);
		}
	}

	// Espero a que todos terminen antes de seguir 
	for (tid = 0; tid < p_archivos; ++tid){
		int status_join = pthread_join(threads_leyendo_archivos[tid], NULL);
		if (status_join) {
			printf("Error: unable to join thread\n");
			exit(-1);
		}
	}

	pair<string, unsigned int> solucion = archivos_leidos.maximum(p_maximos);
	
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


/* Usa count_words(list<string> archs) */
pair<string, unsigned int> ConcurrentHashMap::maximum2(unsigned int p_archivos, 
	                                                   unsigned int p_maximos, 
	                                                   list<string> archs)
{
	ConcurrentHashMap h = ConcurrentHashMap::count_words(archs); 
	pair<string, unsigned int> res = h.maximum(p_maximos);
	return res;
}


/* Usa count_words(unsigned int n, list<string> archs) */
pair<string, unsigned int> ConcurrentHashMap::maximum3(unsigned int p_archivos, 
	                                                   unsigned int p_maximos, 
	                                                   list<string> archs)
{
	ConcurrentHashMap h = ConcurrentHashMap::count_words(p_archivos, archs); 
	pair<string, unsigned int> res = h.maximum(p_maximos);
	return res;
}