#include <gaml-datasets.hpp>
#include <iostream>

std::ostream& operator<<(std::ostream& os,
			 const std::pair<std::array<double, 2>, int>& s) {
  os << s.first[0] << " " << s.first[1] << " " << s.second;
  return os;
}

template<typename SAMPLER>
void dump_samples(std::string filename, const SAMPLER& sampler) {
  std::ofstream outfile(filename);
  for(const auto& s: sampler)
    outfile << s << std::endl;
  outfile.close();
  std::cout << "Samples saved in '" << filename << "'" << std::endl;
}

int main(int argc, char* argv[]) {

  std::random_device rd{};
  std::mt19937 gen{rd()};
  
  {
    auto sampler = gaml::datasets::make_circles(10, 0.1, 0.8, gen);
    // The sampler provides iterators to
    // go through the samples :
    std::cout << "Samples : " << std::endl;
    for(auto it = sampler.begin(); it != sampler.end(); ++it)
      std::cout << (*it) << std::endl;
    std::cout << std::endl;
    
    // Or more simply :
    std::cout << "Samples : " << std::endl;
    for(auto& s: sampler)
      std::cout << s << std::endl;
    std::cout << std::endl;
  }

  {
    auto sampler = gaml::datasets::make_circles(500, 0.05, 0.8, gen);
    dump_samples("circles.data", sampler);
  }
  
  {
    auto sampler = gaml::datasets::make_moons(500, 0.05, gen);
    dump_samples("moons.data", sampler);
  }


  std::cout << std::endl;
  std::cout << "To plot the generated datasets in gnuplot :" << std::endl;
  std::cout << "set palette defined (0 \"red\", 1 \"blue\")" << std::endl;
  std::cout << "set xrange [-1.5:1.5]" << std::endl;
  std::cout << "set yrange [-1.5:1.5]" << std::endl;
  std::cout << "plot 'dataset.data' using 1:2:3 w p lc palette" << std::endl;
    
  return 0;
  
}
