#include <time.h>
#include <fstream>
#include <iostream>
#include "ConcurrentHashMap.hpp"

#define MILLION 1000000L

int main(int argc, char const *argv[])
{
	printf("Empezando con maximum3...\n");

	struct timespec start, stop;
	clockid_t clk_id;
	clk_id = CLOCK_MONOTONIC;

	list<string> l = {"corpus-0", "corpus-1", "corpus-2", "corpus-3", "corpus-4"};
	std::vector<double> v;
	for (int i = 1; i <= 10; ++i)
	{
		printf("Tamaño: %d\n", i);
		vector<double> resultados;
		for (int k = 1; k <= 100; ++k)
		{
			clock_gettime(clk_id, &start);
			ConcurrentHashMap::maximum3(i, 1, l);
			clock_gettime(clk_id, &stop);

			double diff3 = ( stop.tv_sec - start.tv_sec ) * MILLION +
			               ( stop.tv_nsec - start.tv_nsec ) / MILLION;

			if (diff3 < 10000)
			{
				resultados.push_back(diff3);
			}
		}

		int asd = 0;
		for (std::vector<double>::iterator it = resultados.begin(); it != resultados.end(); ++it)
		{
			asd += *it;
		}

		v.push_back(asd/resultados.size());
	}

	// Guardo los resultados en un archivo de texto
	ofstream outfile;
	outfile.open("tiempos3.csv", ios::out);

	// Escribo todos los resultados en el archivo
	int asd = 1;
	for (uint i = 0; i < v.size(); ++i)
	{
		outfile << asd << "," << v[i] << endl;
		asd += 1;
	}

	outfile.close();

	printf("Terminado maximum3\n");

	return 0;
}