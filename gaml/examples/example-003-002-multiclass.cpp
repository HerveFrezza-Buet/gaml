#include <gaml.hpp>
#include <vector>
#include <iterator>
#include <algorithm>
#include <tuple>
#include <limits>

/* In this example, let us build a decision stamp classifier, and make
   a multi-class classifier from it thanks to gaml. The one-versus-one
   scheme is used for that purpose here. */


// Here are the types
typedef double            X;     
typedef unsigned int      Y;     
typedef std::pair<X,Y>    Data;
typedef std::vector<Data> Dataset;

// Let us define a decision stump predictor, fitting  gaml::concepts::Predictor

class Predictor {
private:

  X threshold;   // The threshold for setting the label
  Y below_label; // The label asociated to input values below the threshold. 
  Y above_label; // The label asociated to input values above the threshold. 

public:

  using input_type  = X;
  using output_type = Y;

  Predictor(input_type threshold, output_type below_label, output_type above_label)
    : threshold(threshold), below_label(below_label), above_label(above_label) {}

  Predictor(const Predictor&) = default;
  Predictor& operator=(const Predictor&) = default;

  output_type operator()(input_type x) const {
    if(x < threshold) return below_label;
    else              return above_label;
  }
};

// Let us define the learning algorithm that builds up decision stump
// predictors from data. It fits gaml::concepts::Learner.

class Learner {

public:

  using predictor_type = Predictor;

  Learner() = default;
  Learner (const Learner &other) = default;
  Learner& operator=(const Learner &other) = default;
         
  template<typename DataIterator , typename InputOf , typename OutputOf >
  predictor_type  operator() (const DataIterator& begin, const DataIterator& end, const InputOf& input_of, const OutputOf& output_of) const {
    // Let us get the two labels actually used to tag the dataset. This may raise an exception.
    auto [l1, l2] = gaml::classification::find_two_classes<Y>()(begin, end, output_of);

    // Let us copy the input/ouput pairs in order to sort the samples according to the input.
    std::vector<std::pair<X, Y>> data;
    auto out = std::back_inserter(data);
    for(auto it = begin; it != end; ++it) *(out++) = {input_of(*it), output_of(*it)};
    std::sort(data.begin(), data.end(), [](auto& d1, auto& d2){return d1.first < d2.first;});

    // Now, let us find the best threshold.

    X threshold   = 0;
    Y below_label = 0;
    Y above_label = 0;
    
    std::size_t nb_errors   = std::numeric_limits<std::size_t>::max(); 
    std::size_t nb_l1_below = 0;
    std::size_t nb_l1_above = std::count_if(data.begin(), data.end(), [l=l1](auto& d) {return d.second == l;});
    std::size_t nb_l2_below = 0;
    std::size_t nb_l2_above = data.size() - nb_l1_above;

    // The data dataset has at least 2 elements of different classes
    // (otherwise find_two_classes would have raised an exception)
    auto previous = data.begin();
    auto current  = previous + 1;
    while(current != data.end()) {
      auto thresh = .5*(previous->first + current->first);
      if(previous->second == l1) {
	++nb_l1_below;
	--nb_l1_above;
      }
      else {
	++nb_l2_below;
	--nb_l2_above;
      }
	    
      unsigned int nb_err_l1_below = nb_l2_below + nb_l1_above; // nb of errors if l1 is set to data below thresh
      unsigned int nb_err_l2_below = nb_l1_below + nb_l2_above; // nb of errors if l2 is set to data below thresh
      unsigned int nb_err          = std::min(nb_err_l1_below, nb_err_l2_below);

      // Let us check if this threshold is better than the one found previously.
      if(nb_err < nb_errors) {
	nb_errors = nb_err;
	threshold = thresh;
	if(nb_err_l1_below < nb_err_l2_below) {below_label = l1; above_label = l2;}
	else                                  {below_label = l2; above_label = l1;}
      }

      previous = current++;
    }
	  
    return Predictor(threshold, below_label, above_label);
  }
};


#define PLOT_NAME "stump-multiclass.plot"

int main(int argc, char* argv[]) {

  if(argc != 3) {
    std::cerr << "Usage : " << argv[0] << " <nb_classes> <data size>" << std::endl;
    return 1;
  }

  std::size_t nb_classes = std::stoul(argv[1]);
  std::size_t data_size  = std::stoul(argv[2]);
  
  std::random_device rd;
  std::mt19937 gen(rd());

  Dataset data;
  auto out = std::back_inserter(data);

  // Each class c is anormal distribution of inputs centered at c.
  std::vector<std::normal_distribution<double>> dists;
  for(unsigned int c=0; c < nb_classes; ++c)
    dists.emplace_back(c,1);

  // Let us generate the data
  auto toss_class = std::uniform_int_distribution<std::size_t>(0, nb_classes-1);
  for(unsigned int i = 0; i < data_size; ++i) {
    auto c = toss_class(gen);
    *(out++) = {dists[c](gen), c};
  }

  // Let us build learners and predictors. The following three lines
  // could be gathered into a single expression.
  auto biclass_learner     = Learner();
  auto multi_class_learner = gaml::multiclass::one_vs_one::learner(biclass_learner);
  auto predictor           = multi_class_learner(data.begin(), data.end(), [](auto& d) {return d.first;}, [](auto& d) {return d.second;});

  // And that's it. Let us plot the results using gnuplot.
  
  std::ofstream file(PLOT_NAME);
  std::sort(data.begin(), data.end(), [](auto& d1, auto& d2) {return d1.first < d2.first;});
  file << "set yrange [-1:" << nb_classes << ']' << std::endl
       << "plot '-' w p ls 1 title 'data samples', '-' w l title 'prediction'" << std::endl;
  for(auto& d : data)
    file << d.first << ' ' << d.second << std::endl;
  file << 'e' << std::endl;
  for(auto& d : data)
    file << d.first << ' ' << predictor(d.first) << std::endl;
  file << 'e' << std::endl;
  
  std::cout << std::endl
	    << "run : gnuplot -p " << PLOT_NAME << std::endl;
  
  return 0;
}
