// In this example, we generate noisy sinc data
// that we seek to fit with a linear predictor learned with LASSO

#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <vector>
#include <utility>
#include <ctime>
#include <cmath>

#include <gaml.hpp>
#include <gaml-linear.hpp>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>


typedef double            X;
typedef double            Y;
typedef std::pair<X,Y>    Data;
typedef std::vector<Data> Basis;

X input_of(const Data& d) {return d.first;}
Y label_of(const Data& d) {return d.second;}

template<typename RND_FUNC>
Y oracle(X x, RND_FUNC& rnd_func) {
  double res;
  if(x != 0)
    res = std::sin(x)/x;
  else 
    res = 1;
  return res + rnd_func();
}

#define NB_SAMPLES 200
#define FILE_PREFIX "lasso"
#define PLOT_FILE FILE_PREFIX".plot"
#define DATA_FILE FILE_PREFIX".data"
#define PRED_FILE FILE_PREFIX".pred"

#define NB_CENTERS 11
#define NB_SIGMAS   5
#define NB_FEATURES NB_CENTERS*NB_SIGMAS+1

std::array<X,NB_CENTERS>     centers;
std::array<double,NB_SIGMAS> sigmas_2;

// Let us define the feature function. It has to fill a internally
// allocated NB_FEATURES-sized vector.
void phi(gsl_vector* phi_x, const X& x) {
  unsigned int k = 0;
  gsl_vector_set(phi_x, k++, 1); // The offset.
  for(auto c : centers) 
    for(auto s2 : sigmas_2) {
      double tmp = x - c;
      gsl_vector_set(phi_x, k++, std::exp(-tmp*tmp/s2));
    }
}

int main(int agrc, char* argv[]) {

  std::random_device rd;
  std::mt19937 gen(rd());
  auto rnd_uniform = std::uniform_real_distribution<>(-1.0, 1.0);
  auto rnd_dis_input = [&gen,&rnd_uniform]() { return 10.0 * rnd_uniform(gen);};
  auto rnd_dis_noise = [&gen,&rnd_uniform]() { return rnd_uniform(gen);};
  
  // Feature initialization.
  unsigned int i = 0;
  for(auto& c : centers) c = -10 + (i++)*20/(double)(NB_CENTERS-1);
  sigmas_2 = {{ 2*.1*.1, 2*.5*.5, 2*1*1, 2*2*2, 2*5*5}}; // 2*sigma^2

  Basis b;
  for(unsigned int i = 0; i < NB_SAMPLES; ++i) {
    X x = rnd_dis_input();
    b.push_back({x,oracle(x, rnd_dis_noise)});
  }


  // Learn a predictor
  double lambda = 1e-1;
  auto learner = gaml::linear::lasso::target_lambda_learner<X>(phi, NB_FEATURES, lambda, true);
  auto pred = learner(b.begin(), b.end(), input_of, label_of);
  
  std::cout << "For lambda =" << lambda << ", I selected " << pred.w.size() << " basis" << std::endl;
  std::ofstream data;
  data.open(DATA_FILE);
  data.exceptions(std::ios::failbit | std::ios::badbit);
  for(auto& d : b)
    data << d.first << ' ' << d.second << std::endl;
  data.close();

  std::ofstream predfile;
  predfile.open(PRED_FILE);
  predfile.exceptions(std::ios::failbit | std::ios::badbit);
  for(X x = -10; x <= 10; x += .1) 
    predfile << x << ' ' << pred(x) << std::endl; 
  predfile.close();

  std::ofstream plot;
  plot.open(PLOT_FILE);
  plot.exceptions(std::ios::failbit | std::ios::badbit);
  plot << "set title 'LASSO'" << std::endl
       << "plot 'lasso.data' with points lw .5 lc 3 pt 7 notitle, "
       << "'lasso.pred' using 1:2 with lines lw 2 lc 1 notitle" << std::endl;
  plot.close();

  std::cout << '\"' << PLOT_FILE << "\" generated." << std::endl; 
  return 0;
}
