#include <time.h>
#include "ConcurrentHashMap.hpp"

#define BILLION 1000000000L

int main(int argc, char const *argv[])
{
	struct timespec start, stop;
	clockid_t clk_id;
	clk_id = CLOCK_MONOTONIC;

	clock_gettime(clk_id, &start);
	clock_gettime(clk_id, &stop);

	double diff = ( stop.tv_sec - start.tv_sec ) +
	              ( stop.tv_nsec - start.tv_nsec ) / BILLION;

	return 0;
}