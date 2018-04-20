#include <time.h>
#include <fstream>
#include <iostream>
#include "ConcurrentHashMap.hpp"

#define BILLION 1000000000L

int main(int argc, char const *argv[])
{
	struct timespec start, stop;
	clockid_t clk_id;
	// clk_id = CLOCK_MONOTONIC;
	clk_id = CLOCK_PROCESS_CPUTIME_ID;


	

	std::vector<double> v;
	for (int i = 1; i <= 100; ++i)
	{
		printf("Tamaño: %d\n", i);
		vector<double> resultados;
		for (int k = 1; k <= i; ++k)
		{
			clock_gettime(clk_id, &start);
			ConcurrentHashMap h = ConcurrentHashMap::count_words("corpus");
			clock_gettime(clk_id, &stop);

			double diff = ( stop.tv_sec - start.tv_sec ) * BILLION +
			              ( stop.tv_nsec - start.tv_nsec ) / BILLION;
			resultados.push_back(diff);
		}

		double asd = 0;
		for (std::vector<double>::iterator it = resultados.begin(); it != resultados.end(); ++it)
		{
			asd += *it;
		}
		v.push_back(asd/resultados.size());
	}


	// Guardo los resultados en un archivo de texto
	ofstream outfile;
	outfile.open("tiempos.csv", ios::out);

	// Escribo todos los resultados en el archivo
	int asd = 1;
	for (unsigned int i = 0; i < v.size(); ++i)
	{
		outfile << asd << ","
			    << v[i] << endl;
		asd += 1;
	}

	outfile.close();


	return 0;
}