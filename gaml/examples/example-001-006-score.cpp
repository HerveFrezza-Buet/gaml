#include <gaml.hpp>
#include <iostream>
#include <vector>
#include <utility>
#include <random>

// Let us deal with 1D data, that consists of two classes A and B. In
// each class, the data are tossed from a uniform range of
// values. Ranges for classes A and B slightly overlap. Each value is
// associated with label, that is also tossed uniformly in a range
// around some central value. The central value differs for A and B
// classes.

typedef double            X;
typedef double            Y;
typedef std::pair<X,Y>    Data;
typedef std::vector<Data> Basis;

X input_of(const Data& d)  {return d.first;}
Y output_of(const Data& d) {return d.second;}

#define PROBA_A      0.3
#define A_MIN        0.0
#define A_MAX        0.6
#define B_MIN        0.4
#define B_MAX        1.0
#define A_OUT_CENTER 0.0
#define B_OUT_CENTER 1.0
#define OUT_RADIUS   1.0

template<typename RANDOM_DEVICE>
Data sample(RANDOM_DEVICE& rd) {
  X xmin,xmax;
  Y ymin,ymax;

  if(std::bernoulli_distribution(PROBA_A)(rd)) {
    xmin = A_MIN;
    xmax = A_MAX;
    ymin = A_OUT_CENTER - OUT_RADIUS;
    ymax = A_OUT_CENTER + OUT_RADIUS;
  }
  else {
    xmin = B_MIN;
    xmax = B_MAX;
    ymin = B_OUT_CENTER - OUT_RADIUS;
    ymax = B_OUT_CENTER + OUT_RADIUS;
  }

  return {std::uniform_real_distribution<double>(xmin,xmax)(rd),
      std::uniform_real_distribution<double>(ymin,ymax)(rd)};
}

// Let us use define a class which uses a score method as
// parameter. gaml provide such classes, the purpose here is to
// introduce the way they are coded. The kind of score is a template
// template... ok, let us read the example. A score is a template of
// the form Score<ITER,TEST,VALUE_OF>, it has three type
// variables. The following Evaluator class is parametrized by the
// kind of score we intend to use for evaluation.

template <template<typename,typename,typename> class SCORE>
class Evaluator {
public:
  template<typename DataIterator, typename Test, typename Valueof> 
  double compute_score(const DataIterator& begin, const DataIterator& end, const Test& test, const Valueof& value_of) {
    SCORE<DataIterator,Test,Valueof> score;
    return score(begin,end,test,value_of);
  }
};

int main(int argc, char* argv[]) {
  
  // random seed initialization
  std::random_device rd;
  std::mt19937 gen(rd());
  
  Basis basis(100000);
  for(auto& data : basis) data = sample(gen);

  // Let us use the relative variance reduction score. It scores a
  // splitting of the data basis, according to a test. Here, the
  // thread is a threshold according to x.
  auto test = [](const Data& d) -> bool {return d.first < .5*(A_MAX+B_MIN);};

  // Let us set up an evaluator with the score class provided by gaml.
  Evaluator<gaml::score::RelativeVarianceReduction> evaluator;

  // Now, we can compute...
  std::cout << std::endl
	    << "mean                            = " << gaml::average (basis.begin(),basis.end(),output_of) << std::endl
	    << "variance                        = " << gaml::variance(basis.begin(),basis.end(),output_of) << std::endl
	    << "relative variance reduction (1) = " << evaluator.compute_score(basis.begin(),basis.end(),test,output_of) << std::endl
	    << "relative variance reduction (2) = " << gaml::score::relative_variance_reduction(basis.begin(),basis.end(),test,output_of) << std::endl // the same as previous...
	    << std::endl;

  return 0;
}
