#include <gaml.hpp>
#include <memory>
#include <cmath>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <random>

// #define DEBUG

/*
  This example shows how a decision tree algorithm can be implemented
  with gaml.
*/

// Let us define a decision tree to approximate a scalar function.
class Tree {
private:

  // For nodes
  double threshold;
  std::unique_ptr<Tree> lower;
  std::unique_ptr<Tree> upper;

  // For leaves
  double value;

public:
  
  Tree(double val) 
    : threshold(0), lower(), upper(), value(val) {} // This builds a leaf
  Tree(double thres,
       std::unique_ptr<Tree> low,
       std::unique_ptr<Tree> up) 
    : threshold(thres),
      lower(std::move(low)),
      upper(std::move(up)),
      value(0) {}

  double operator()(double x) {
    if(lower == nullptr) // this is a leaf
      return value; 
    // this is a node
    else
      if(x < threshold)  
	return (*lower)(x);
      else
	return (*upper)(x);
  }
};

#ifdef DEBUG
template<typename Iterator>
void display(const std::string& title, const Iterator& begin, const Iterator& end) {
  std::cout << title << " :" << std::setprecision(3);
  for(auto iter = begin; iter != end; ++iter) 
    std::cout << " (" << (*iter).first << "," << (*iter).second << ")";
  std::cout << std::endl;
}
#endif


// The idea now is to build a decision tree from some (x,y) pairs,
// gathered in a collection. Let us implement a generic function for
// this purpose.

template<typename Iterator, typename InputOf, typename OutputOf>
std::unique_ptr<Tree> build_tree(const Iterator& begin, const Iterator& end,
				 const InputOf& input_of, const OutputOf& output_of) {

  auto size = std::distance(begin,end);

#ifdef DEBUG
  std::cout << ">> build_tree : " << size << " : ";
  display("Args",begin,end);
#endif

  if(size < 10) {
#ifdef DEBUG
    std::cout << "  average = " << gaml::average(begin,end,output_of) << std::endl;
#endif
    return std::unique_ptr<Tree>(new Tree(gaml::average(begin,end,output_of))); 
  }
  else {
    auto x_values    = gaml::map(begin, end, input_of);
    auto bounds      = std::minmax_element(x_values.begin(), x_values.end());
    double threshold = .5 * (*(bounds.first) + *(bounds.second));
    auto test        = [threshold,input_of](const typename Iterator::value_type& d) -> bool {return input_of(d) < threshold;};
    auto split       = gaml::split(begin, end, test);
    auto low         = split.true_values;
    auto up          = split.false_values;

#ifdef DEBUG
    std::cout << "  split at " << threshold 
	      << " into " << std::distance(low.begin(), low.end()) 
	      << '+'      << std::distance(up.begin(), up.end()) 
	      << std::endl;
    display("  low", low.begin(), low.end());
    display("   up", up.begin(), up.end());
#endif
    
    return std::unique_ptr<Tree>(new Tree(threshold,
					  build_tree(low.begin(), low.end(), input_of, output_of),
					  build_tree(up.begin(),  up.end(),  input_of, output_of)));
  }
}


template<typename RANDOM_DEVICE>
double oracle(double x, RANDOM_DEVICE& rd) {
  return std::sin(x) + std::uniform_real_distribution<double>(-.2,.2)(rd);
}

typedef std::pair<double,double> Data;

int main(int argc, char* argv[]) {

  // random seed initialization
  std::random_device rd;
  std::mt19937 gen(rd());
  

  std::vector<Data> basis(100);
  std::uniform_real_distribution<double> uniform(0, 10);
  for(auto& xy : basis) {
    xy.first  = uniform(gen);
    xy.second = oracle(xy.first, gen);
  }

  auto tree = build_tree(basis.begin(), basis.end(),
			 [](const Data& d) -> double {return d.first; },
			 [](const Data& d) -> double {return d.second;});


  std::ofstream gnuplot("tree.plot");
  std::ofstream data("basis.dat");
  std::ofstream line("tree.dat");
  
  gnuplot << "set xrange[0:10]"     << std::endl
  	  << "set yrange[-1.5:1.5]" << std::endl
  	  << "plot 'basis.dat' using 1:2 with dots notitle, "
  	  << "'tree.dat' using 1:2 with lines notitle" << std::endl;

  for(auto& xy : basis) 
    data << xy.first << ' ' << xy.second << std::endl;

  for(double x=0; x < 10; x+=.01)
    line << x << ' ' << (*tree)(x) << std::endl;

  std::cout << std::endl
  	    << "gnuplot -p tree.plot" << std::endl
  	    << std::endl;

  
  return 0;
}
