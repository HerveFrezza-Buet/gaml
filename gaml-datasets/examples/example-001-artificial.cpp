#include <gaml-datasets.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
  auto sampler = gaml::datasets::make_circles(100, 0.1, 0.8);

  int idx = 0;
  for(auto s: sampler) {
    std::cout << "Sample " << idx++ << " : ("
	      << s.first[0] << "; " << s.first[1]
	      << ")"
	      << " -> " << s.second
	      << std::endl;
  }
}
