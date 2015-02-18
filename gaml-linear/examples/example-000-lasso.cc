// In this example, we build a learning basis from a linear combination of basis
// We check at the end if the weights that are learned correspond to the weights used for creating the basis
// You can specify how many basis you want to use for fitting the data;
// Actually, the data are created with a random number of basis with random weights

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

#define NB_SAMPLES 2000
#define FILE_PREFIX "lasso"
#define PLOT_FILE FILE_PREFIX".plot"
#define DATA_FILE FILE_PREFIX".data"
#define PRED_FILE FILE_PREFIX".pred"


#define PLOT_PATH_FILE FILE_PREFIX"_path.plot"
#define DATA_PATH_FILE FILE_PREFIX"_path.data"

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

int main(int argc, char* argv[]) {
  std::srand(std::time(0));

  if(argc != 2) {
    std::cerr << "Usage : " << argv[0] << " lambda" << std::endl;
    return -1;
  }
  double lambda = atof(argv[1]);

  // Feature initialization.
  unsigned int i = 0;
  for(auto& c : centers) c = -10 + (i++)*20/(double)(NB_CENTERS-1);
  sigmas_2 = {{2*.2*.2, 2*.5*.5, 2*1*1, 2*2*2, 2*5*5}}; // 2*sigma^2

  // The output will be a linear combination of the features
  std::map<unsigned int, double> weights;
  for(unsigned int i = 0 ; i < 5; ++i) {
    weights[int(gaml::random::uniform(0, NB_FEATURES))] = gaml::random::uniform(0, 1.0);
  }
  
  std::cout << "I will use the following linear combination : " << std::endl;
  for(auto& kv: weights) 
    std::cout << "Basis " << kv.first << " w = " << kv.second << std::endl;

  Basis b;
  gsl_vector* phi_x = gsl_vector_alloc(NB_FEATURES);
  for(unsigned int i = 0; i < NB_SAMPLES; ++i) {
    X x = gaml::random::uniform(-10,10);
    // Let us compute all the features at x
    phi(phi_x, x);
    // The output is a linear combination of these features;
    Y y = 0;
    for(auto& kv: weights)
      y += kv.second * gsl_vector_get(phi_x, kv.first);
    b.push_back({x,y});
  }

  // Learn a predictor
  auto learner = gaml::linear::lasso::target_lambda_learner<X>(phi, NB_FEATURES, lambda);
  auto pred = learner(b.begin(), b.end(), input_of, label_of);

  std::cout << "I found the following weights (out of " << NB_FEATURES << ") for the predictor : " << std::endl;
  for(auto& kv: pred.w)
    std::cout << "Basis " << kv.first << " w = " << kv.second << std::endl;

  // We compute the euclidean distance between the learned weights and the ones we used
  // to build up the basis
  double dist = 0.0;
  for(unsigned int i = 0 ; i < NB_FEATURES; ++i) {
    double w = 0;
    double pw = 0;
    auto itw = weights.find(i);
    if(itw != weights.end())
      w = (*itw).second;
    auto itpw = pred.w.find(i);
    if(itpw != pred.w.end()) 
      pw = (*itpw).second;
    dist += (pw - w) * (pw - w);
  }
  dist = sqrt(dist);
  std::cout << "Distance to the weights used to generate the data : " << dist << std::endl;

  auto predictor_evaluator = gaml::risk::empirical(gaml::loss::Quadratic<double>());
  double risk = predictor_evaluator(pred,
				    b.begin(),b.end(),
				    input_of, label_of);

  std::cout << "Empirical risk : " << risk << std::endl;
    
  
  // Dump the results for plotting
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


  // Create the files for plotting the regularization path
  data.open(DATA_PATH_FILE);
  data.exceptions(std::ios::failbit | std::ios::badbit);
  for(auto& d: learner.regularization_path) {
    data << d.first << " ";
    auto w = d.second;
    for(unsigned int i = 0 ; i < w->size ; ++i)
      data << gsl_vector_get(w, i) << " ";
    data << std::endl;
  }
  data.close();

  plot.open(PLOT_PATH_FILE);
  plot << "set title 'LASSO regularization path'" << std::endl;
  plot << "plot " ;
  for(unsigned int i = 0 ; i < NB_FEATURES; ++i) {
    plot << "\"" << DATA_PATH_FILE << "\" using 1:" << i+2 << " with lines notitle";
    if(i != NB_FEATURES - 1)
      plot << ",";
    else
      plot << std::endl;
  }
  plot.close();
  std::cout << '\"' << PLOT_PATH_FILE << "\" generated." << std::endl; 

  return 0;  

}
