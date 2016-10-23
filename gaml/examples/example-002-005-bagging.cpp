#include <gaml.hpp>

#include <array>
#include <vector>
#include <utility>
#include <cstdlib>
#include <ctime>

// Let us implement a bagging method from a silly predictor.

typedef std::array<double, 2> X;
typedef bool                  Y;
typedef std::pair<X, Y>       Data;
typedef std::vector<Data>     Basis;

const X& input_of (const Data& d) {return d.first;}
const Y  output_of(const Data& d) {return d.second;}

Y oracle(const X& x) {
  return x[1] < x[0];
}

#define XMIN -1
#define XMAX  1
#define NOISE .1
Data sample() {
  X x = {{gaml::random::uniform(XMIN,XMAX),
	  gaml::random::uniform(XMIN,XMAX)}};
  Y y = oracle(x);
  if(gaml::random::proba(NOISE))
    y = !y;
  return {x,y};
}

class Predictor {
private:
  X      w;
  double c;

public:
  typedef X input_type;
  typedef Y output_type;
  
  Predictor(const X& weight,
	    double offset) 
    : w(weight), c(offset) {}

  Predictor(const Predictor& other)            = default;
  Predictor& operator=(const Predictor& other) = default;
  
  // This does the prediction.
  output_type operator()(const input_type& x) const {
    return w[0] * x[0]
      +    w[1] * x[1]
      +    c > 0;
  }

  void display() {
    std::cout << w[0] << "* x + " << w[1] << "*y + " << c << " > 0" << std::endl;
  }
};


class Learner {

public:

  typedef Predictor predictor_type;

  Learner(void) {}
  Learner(const Learner& other) {}

  template<typename DataIterator, typename InputOf, typename OutputOf> 
  Predictor operator()(const DataIterator& begin, const DataIterator& end,
		       const InputOf& input_of, const OutputOf& output_of) const {
    // Let us filter 'true' from 'false' samples, the for each
    // cathegory and pick a random 'true' and a random 'false'
    // sample in each.

    unsigned int pos;
    auto trues    = gaml::filter(begin,end,output_of);
    pos           = gaml::random::uniform(std::distance(trues.begin(),trues.end()));
    auto it_true  = trues.begin(); std::advance(it_true,pos);
    X x_true      = input_of(*it_true);

    auto falses   = gaml::reject(begin,end,output_of);
    pos           = gaml::random::uniform(std::distance(falses.begin(),falses.end()));
    auto it_false = falses.begin(); std::advance(it_false,pos);
    X x_false     = input_of(*it_false);

    // Let us find w,c such as w.x+c > 0 is the 'true' side of the
    // bisection between x_false and x_true.
    X w = {{x_true[0]-x_false[0], x_true[1]-x_false[1]}};
    double c = 
      - .5*(x_true[0]+x_false[0])*w[0]
      - .5*(x_true[1]+x_false[1])*w[1];
    return Predictor(w,c);
  }
};

// For bagging, we need a template that tells how to compute the
// overall output from the output of all the predictor. Let us set up
// a silly one, that returns the output of the first predictor in the
// bag.
class SillyMerge {
  public:
  
  // This is requires.
  typedef Y output_type;

  // begin and end allows for iterating on the predictors output. Each
  // output can be retrieved by value_of.
  template<typename DataIterator,typename ValueOf> 
  Y operator()(const DataIterator& begin, const DataIterator& end, const ValueOf& value_of) const {
    return value_of(*begin);
  }
};

// We also need a way to feed each predictor from the basis. Here, let
// us feed each predictor with a random shuffle of the initial
// basis... this is also silly.
class SillyRandomizer {
  public:
  template<typename DataIterator> 
  auto operator()(const DataIterator& begin, const DataIterator& end) const {
    return gaml::shuffle(begin,end);
  }
};


#define NB_SAMPLES 1000
#define BAG_SIZE     20
int main(int argc, char* argv[]) {

  // random seed initialization.
  std::srand(std::time(0));

  // Setting up of a dataset.
  Basis basis(NB_SAMPLES);
  for(auto& data : basis) data = sample();

  // Let us set up a weak predictor from the data.
  Learner learner;
  auto weak = learner(basis.begin(), basis.end(), input_of, output_of);

  // Let us measure its empirical risk.
  auto evaluator = gaml::risk::empirical(gaml::loss::Classification<Y>());
  std::cout << "Weak risk = " 
	    << evaluator(weak,basis.begin(), basis.end(), input_of, output_of)
	    << std::endl;

  // Now, let us use bagging with many of such weak predictors. For
  // the sake illustration, let us dot this with our silly templates.
  
  auto silly_bag_learner = gaml::bag::learner(learner, 
					      SillyMerge(), 
					      SillyRandomizer(), 
					      BAG_SIZE,
					      false);
  auto silly = silly_bag_learner(basis.begin(), basis.end(), input_of, output_of);
  std::cout << "Silly risk = " 
	    << evaluator(silly,basis.begin(), basis.end(), input_of, output_of)
	    << std::endl;

  // Indeed, more clever ready-to-use templates are available in the
  // library. Their design is similar to the design of the Silly*
  // classes of this example.
  auto clever1_bag_learner = gaml::bag::learner(learner, 
						gaml::MostFrequent<Y>(),
						gaml::bag::set::Identity(), 
						BAG_SIZE,
						false);
  auto clever1 = clever1_bag_learner(basis.begin(), basis.end(), input_of, output_of);
  std::cout << "Clever #1 risk = " 
	    << evaluator(clever1,basis.begin(), basis.end(), input_of, output_of)
	    << std::endl;

  // Indeed, more clever ready-to-use templates are available in the
  // library. Their design is similar to the design of the Silly*
  // classes of this example.
#define BOOTSTRAP_SIZE 200
  auto clever2_bag_learner = gaml::bag::learner(learner, 
						gaml::MostFrequent<Y>(),
						gaml::bag::set::Bootstrap(BOOTSTRAP_SIZE), 
						BAG_SIZE,
						false);
  auto clever2 = clever2_bag_learner(basis.begin(), basis.end(), input_of, output_of);
  std::cout << "Clever #2 risk = " 
	    << evaluator(clever2,basis.begin(), basis.end(), input_of, output_of)
	    << std::endl;

  std::cout << std::endl;

  // This shows how to access to the predictors in the bag (e.g. for
  // saving and loading).
  for(auto& pred : clever1.predictors) pred.display();
  std::cout << std::endl;
  

  return 0;
}
