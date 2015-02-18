#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <vector>
#include <utility>
#include <ctime>
#include <cmath>
#include <sstream>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>

typedef double X;
typedef double Y;
typedef std::pair<X, Y> Data;
typedef std::vector<Data> Basis;

X input_of(const Data& d) {return d.first;}
Y label_of(const Data& d) {return d.second;}

#define NB_SAMPLES 2000
#define X_MIN (-2)
#define X_MAX (+2)

#define NB_FEATURES 100

double sawtooth_function(double x) {
  return x - floor(x);
}

// Let us define the feature function. It has to fill a internally
// allocated NB_FEATURES-sized vector.
// We take the harmonics : sin(2pi k)     k=0 ... NB_FEATURES-1
void phi(gsl_vector* phi_x, const X& x) {
  for(unsigned int i = 0 ; i < NB_FEATURES; ++i)
    gsl_vector_set(phi_x, i, sin(2.0 * M_PI * double(i) * x));
}

void fill_basis(Basis& b) {
  b.clear();
  for(unsigned int i = 0 ; i < NB_SAMPLES; ++i) {
    double x = X_MIN + double(i)*(X_MAX - X_MIN)/(NB_SAMPLES-1);
    b.push_back({x, sawtooth_function(x)});
  }
}
/*
void plot_features(std::string filename) {
  ostr.str("");
  ostr << filename << ".data";
  std::ofstream outfile;
  datefile.open(ostr.str().c_str());
  for(unsigned int i = 0 ; i < NB_SAMPLES; ++i) {
    double x = X_MIN + double(i)*(X_MAX - X_MIN)/(NB_SAMPLES-1);
    for(unsigned int j = 0 ; j < NB_FEATURES; ++j) {
      double y = 
    }
  }
}
*/

template<typename PREDICTOR>
void generate_plot(std::string filename, Basis& b, PREDICTOR& pred) {
  std::ostringstream ostr;

  ostr.str("");
  ostr << filename << ".data";
  std::ofstream datafile;
  datafile.open(ostr.str().c_str());
  datafile.exceptions(std::ios::failbit | std::ios::badbit);
  for(auto& bi: b) 
    datafile << bi.first << " " << bi.second << std::endl;
  datafile.close();

  ostr.str("");
  ostr << filename << ".pred";
  std::ofstream predfile;
  predfile.open(ostr.str().c_str());
  predfile.exceptions(std::ios::failbit | std::ios::badbit);
  for(auto& bi: b)
    predfile << bi.first << " " << pred(bi.first) << std::endl;
  predfile.close();


  ostr.str("");
  ostr << filename << ".plot";
  std::ofstream plotfile;
  plotfile.open(ostr.str().c_str());
  plotfile << "set termoption dash" << std::endl
	   << "set xrange [" << X_MIN << ":" << X_MAX << "]" << std::endl
	   << "set yrange [-.1:1.1]" << std::endl
	   << "set title 'Approximation of the sawtooth wave'" << std::endl
	   << "set style line 1 lt 1 lc rgb \"black\" lw 2" << std::endl
	   << "set style line 2 lt 1 lc rgb \"blue\" lw 2" << std::endl
	   << "plot \"" << filename <<".data\" using 1:2 with lines ls 1 notitle, \"" << filename << ".pred\" using 1:2 with lines ls 2 notitle" << std::endl;
  plotfile.close();

  std::cout << ostr.str() << " gnuplot file generated" << std::endl;

}
