#include <gaml-datasets.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
  auto sampler = gaml::datasets::make_circles(0.1, 0.8);

  auto it = sampler.begin();
  for(unsigned int i = 0 ; i < 100; ++i, ++it)
    std::cout << "Sample " << i << " : ("
	      << it->first[0] << "; " << it->first[1]
	      << ")"
	      << " -> " << it->second
	      << std::endl;
}
