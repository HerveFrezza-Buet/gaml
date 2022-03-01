#include <gaml-datasets.hpp>
#include <iomanip>
#include <iostream>

template<typename DATASET>
void print(const DATASET& dataset,
	   unsigned int nb_samples) {
  unsigned int idx = 0;
  for(const auto& s: dataset) {
    for(const auto& v: dataset.input_of_data(s))
      std::cout << v << " ";

    std::cout << " -> " << dataset.output_of_data(s) << std::endl;
    
    ++idx;
    if(idx == nb_samples)
      break;
  }
}


int main(int argc, char* argv[]) {
  
  {
    std::cout << std::endl << std::endl
	      << "***** " << std::setw(80) << std::setfill('*') << std::left << " Iris classification Dataset " << std::endl;
    auto dataset = gaml::datasets::make_iris();
    print(dataset, 2);
  }
  
  {
    std::cout << std::endl << std::endl
	      << "***** " << std::setw(80) << std::setfill('*') << std::left << " Diabetes regression Dataset " << std::endl;
    auto dataset = gaml::datasets::make_diabetes();
    print(dataset, 2);
  }
  
  {
    std::cout << std::endl << std::endl
	      << "***** " << std::setw(80) << std::setfill('*') << std::left << " Wine classification Dataset " << std::endl;
    auto dataset = gaml::datasets::make_wine();
    print(dataset, 2);
  }
  
  {
    std::cout << std::endl << std::endl
	      << "***** " << std::setw(80) << std::setfill('*') << std::left << " Boston housing regression Dataset " << std::endl;
    auto dataset = gaml::datasets::make_boston_housing();
    print(dataset, 2);
  }
}
