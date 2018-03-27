// In this example, we compute the LASSO path on the standardized diabetes data

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

#include "diabetes-parser.hpp"
#define PLOT_PATH_FILE FILE_PREFIX"_path.plot"
#define DATA_PATH_FILE FILE_PREFIX"_path.data"
#define DIABETES_DATA_FILE "/usr/share/gaml-linear/diabetes.data"
#define FILE_PREFIX "diabetes-lars"

int main(int argc, char* argv[]) {

  // Let us parse the diabetes.sdata datafile
  std::ifstream ifile;
  
  ifile.exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);
  try {
    ifile.open(DIABETES_DATA_FILE);
  }
  catch(std::exception) {
    std::cerr << "Cannot open " << DIABETES_DATA_FILE << " for reading " << std::endl;
  }

  diabetes::Parser parser;
  auto input_customer_stream = gaml::make_input_data_stream(ifile, parser);
  auto begin = gaml::make_input_data_begin(input_customer_stream);
  auto end = gaml::make_input_data_end (input_customer_stream);
  
  diabetes::Basis b(diabetes::nb_samples);
  std::copy(begin, end, b.begin());

  // Let us see what the data look like
  //for(auto it = b.begin() ; it != b.end() ; ++it)
  //  diabetes::print(*it);
  
  // We can now learn our regressor on the data
  // We ask the algorithm to normalize the data and to disable verbosity
  auto learner = gaml::linear::lars::target_lambda_learner<diabetes::X>(diabetes::phi, diabetes::nb_features, 1e-15, true, false);
  auto pred = learner(b.begin(), b.end(), diabetes::input_of, diabetes::label_of);

  // Create the files for plotting the regularization path
  std::ofstream data;
  data.open(DATA_PATH_FILE);
  data.exceptions(std::ios::failbit | std::ios::badbit);
  std::vector<double> xbreaks;
  for(auto& d: learner.regularization_path) {
    auto w = d.second;
    double norm1 = 0;
    for(unsigned int i = 0 ; i < w->size ; ++i)
      norm1 += fabs(gsl_vector_get(w, i));
    data << norm1 << " ";
    xbreaks.push_back(norm1);
    for(unsigned int i = 0 ; i < w->size ; ++i)
      data << gsl_vector_get(w, i) << " ";
    data << std::endl;
  }
  data.close();

  std::ofstream plot;
  plot.open(PLOT_PATH_FILE);
  plot << "set title 'LARS regularization path'" << std::endl;
  plot << "set parametric" << std::endl;
  plot << "set trange [-1000:1000]" << std::endl;
  plot << "plot " ;
  for(unsigned int i = 0 ; i < diabetes::nb_features; ++i) 
    plot << "\"" << DATA_PATH_FILE << "\" using 1:" << i+2 << " with lines notitle,";
  for(unsigned int i = 0 ; i < xbreaks.size(); ++i) {
    plot << xbreaks[i] << ",t with lines notitle lt 2 lc 7 ";
    if(i < xbreaks.size() - 1)
      plot << ", ";
    else 
      plot << std::endl;
  } 
  plot.close();
  std::cout << PLOT_PATH_FILE << " generated. " << std::endl;
}
