#include <gaml-datasets.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
  auto filename = gaml::datasets::download("https://archive.ics.uci.edu/ml/machine-learning-databases/iris/iris.data");

  std::cout << "File downloaded at : " << filename << std::endl;
}
