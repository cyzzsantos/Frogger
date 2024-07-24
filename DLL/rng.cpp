#include "rng.h"

Results rng(int min, int max) {
	Results results;

	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> random(min, max);

	do {
		results.num1 = random(rng);
		results.num2 = random(rng);
	} while (results.num1 == results.num2);

	return results;
}

int rngSetup(int min, int max) {
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> random(min, max);
	
	return random(rng);
}