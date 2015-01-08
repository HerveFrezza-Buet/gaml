#include <gaml.hpp>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <utility>

typedef unsigned int Label;
typedef std::vector<Label> Basis;

Label output_of(const Label& l) {return l;}

#define BASIS_SIZE 50
int main(int argc, char* argv[]) {
  // random seed initialization
  std::srand(std::time(0));

  Basis basis(BASIS_SIZE);
  for(auto& data : basis) data = gaml::random::uniform((unsigned int)10);
  
  std::cout << std::endl
	    << "Basis : ";
  for(auto data: basis) std::cout << data;
  std::cout << std::endl
	    << std::endl;
  
  auto frequencies = gaml::frequencies<unsigned int>(basis.begin(), basis.end(), output_of);
  std::cout << "Frequencies : " << std::endl;
  for(auto& kv : frequencies)
    std::cout << "  " << kv.first << " frequency is " << 100*kv.second << "%." << std::endl;

  std::cout << "Most frequent label : " << gaml::most_frequent<unsigned int>(basis.begin(), basis.end(), output_of) << std::endl
	    << "Classification entropy : " << gaml::classification_entropy<Label>(basis.begin(), basis.end(), output_of) << std::endl;
  
  auto test1 = [](const Label& l) -> bool {return l < 5;};
  auto test2 = [](const Label& l) -> bool {return l < 1;};
  std::cout << "Normalized information gain #1 : " << gaml::score::normalized_information_gain(basis.begin(),basis.end(),test1,output_of) << std::endl
	    << "Normalized information gain #2 : " << gaml::score::normalized_information_gain(basis.begin(),basis.end(),test2,output_of) << std::endl;

  return 0;
}
