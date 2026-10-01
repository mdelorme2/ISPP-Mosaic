#ifndef MAIN_H 
	#define MAIN_H
	using namespace std;

	#include <iostream> 
	#include <math.h> 
	#include <cstdlib>
	#include <algorithm>
	#include "gurobi_c++.h"
	#include "time.h"
	#include "helper_functions.h" 

	float EPSILON = 0.001;

	int ILP(Instance& inst);

#endif 
