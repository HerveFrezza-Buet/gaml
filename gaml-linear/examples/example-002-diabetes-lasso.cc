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
#include <gaml-datasets.hpp>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>

#define FILE_PREFIX "diabetes-lasso"
#define PLOT_PATH_FILE FILE_PREFIX"_path.plot"
#define DATA_PATH_FILE FILE_PREFIX"_path.data"

using Dataset = decltype(gaml::make_diabetes_dataset());
using input_type = Dataset::input_type;

void phi(gsl_vector* phi_x, const input_type& x) {
    unsigned int i = 0;
    for(auto& xi: x)
        gsl_vector_set(phi_x, i++, xi);
}


int main(int argc, char* argv[]) {

  auto dataset = gaml::make_diabetes_dataset();
    using Dataset = decltype(dataset);

  // Let us see what the data look like
  for(const auto& s: dataset) {
      for(const auto& v: Dataset::input_of_data(s))
          std::cout << v << " ";
      std::cout << " -> " << Dataset::output_of_data(s) << std::endl;
  }
  
  unsigned int nb_features = Dataset::input_dim;

  // We can now learn our regressor on the data
  // We ask the algorithm to normalize the data and to disable verbosity
  auto learner = gaml::linear::lasso::target_lambda_learner<Dataset::input_type>(phi, nb_features, 1e-9, true, false);
  auto pred = learner(dataset.begin(), dataset.end(), Dataset::input_of_data, Dataset::output_of_data);

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
  plot << "set title 'LASSO regularization path'" << std::endl;
  plot << "set parametric" << std::endl;
  plot << "set trange [-1000:1000]" << std::endl;
  plot << "plot " ;
  for(unsigned int i = 0 ; i < nb_features; ++i) 
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
