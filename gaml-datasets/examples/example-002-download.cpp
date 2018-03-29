#include <gaml-datasets.hpp>
#include <iostream>

int main(int argc, char* argv[]) {

  auto dataset = gaml::make_iris_dataset();
  for(auto& sample: dataset) {
    for(auto& v: dataset.input_of_data(sample))
      std::cout << v << " ";
    std::cout << "-> " << dataset.output_of_data(sample) << std::endl;
  }

  /*
  auto diabetes = gaml::datasets::diabetes();
  unsigned int patient_idx = 0;
  for(auto& patient: diabetes) {
    std::cout << "Patient " << patient_idx << " : " << std::endl;
  }
  */
}
