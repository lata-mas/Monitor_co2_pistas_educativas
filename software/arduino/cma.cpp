#include "cma.h"

/*
 * Cumulative moving average
 * 
 *   CMA_n = (X_1 + --- X_n)/n
 * 
 */

CMA::CMA(int N) {
  maxFreq = N;
  freq = 0;
  sum = 0;
}

float CMA::addData(float data) {
  if(freq<=maxFreq) ++freq;
  else sum -= sum/freq;
  sum+=data;
  return data;
};

float CMA::avg(void) {
  if(freq>0) return sum/freq;
  else return 0;
};
