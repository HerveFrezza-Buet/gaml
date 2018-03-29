#include <gaml-datasets.hpp>
#include <iostream>

template<typename DATASET>
void print_dataset(const DATASET& dataset,
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
    std::cout << std::string(100, '*') << "Iris Dataset" << std::endl;
    auto iris = gaml::make_iris_dataset();
    print_dataset(iris, 2);
  }
  
  {
    std::cout << std::string(100, '*') << "Diabetes Dataset" << std::endl;
    auto diabetes = gaml::make_diabetes_dataset();
    print_dataset(diabetes, 2);
  }
}
