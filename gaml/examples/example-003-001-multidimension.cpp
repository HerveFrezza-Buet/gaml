#include <gaml.hpp>
#include <utility>
#include <array>


// This example shows how to deal with multi-dimensional output when
// regressors are mono-dimensional.

// Read this file. It provides mono-dimensional regressors.
#include <example-silly.hpp>


// These are useful type definitions.
class Point {
public:
  double x;
  double y;
  double z;
};

typedef int               X;       // The input;
typedef Point             Y;       // The multi-dimensional output.
typedef std::pair<X,Y>    Data;    // We learn from sample/label pairs
typedef std::vector<Data> DataSet; // We decide to use a vector for the data set.

// We need functions to extract input and output from the data.
int input_of_data (const Data& data) {return data.first; }
Y   output_of_data(const Data& data) {return data.second;}

// This function converts an array of 3 values to an actual Y value.
Y output_of_array(const std::array<double,3>& values) {
  Y res;
  res.x = values[0];
  res.y = values[1];
  res.z = values[2];
  return res;
}

// This function extracts the scalar values from an output.
std::array<double,3> array_of_output(const Y& output) {
  return {{output.x,output.y,output.z}};
}

// We need to define a loss for Y. It has to fit gaml::concept::Loss.
class Loss {
public:
  typedef Y output_type;
  double operator()(const output_type& l1, const output_type& l2) const {
    gaml::loss::Quadratic<double> loss;
    return loss(l1.x, l2.x) + loss(l1.y, l2.y) + loss(l1.z, l2.z);
  }
};

// We need a learner that deals with 3D outputs. The silly namespace
// provides learner for scalar outputs. The idea is to use one scalar
// predictor for each of the three outputs.

class Learner {
public:
  
  // This typedef is not required by the Learner concept.
  typedef silly::Predictor scalar_predictor_type;
  
  // Let us define a 3D predictor.
  typedef gaml::multidim::Predictor<Y,scalar_predictor_type,3> predictor_type;
  
  Learner(void) {}

  Learner(const Learner& other) {}
  Learner& operator=(const Learner& other) {return *this;}

  // This does the learning, and returns a predictor from the data.
  template<typename DataIterator, typename InputOf, typename OutputOf> 
  predictor_type operator()(const DataIterator& begin, const DataIterator& end,
			    const InputOf& input_of, const OutputOf& output_of) const {
    silly::Learner scalar_algo;
    
    std::array<scalar_predictor_type,3> predictors = {{ 
	scalar_algo(begin, end, input_of, [&output_of](const Data& data) -> double {return output_of(data).x;}),
	scalar_algo(begin, end, input_of, [&output_of](const Data& data) -> double {return output_of(data).y;}),
	scalar_algo(begin, end, input_of, [&output_of](const Data& data) -> double {return output_of(data).z;}) 
      }};

    // We build our multi-dimensional predictor from a conversion function and 3 scalar predictors.
    return predictor_type(output_of_array, predictors.begin(), predictors.end());			  
  }
};

// As in the previous class, the same learner is used for all
// dimensions, you can use the predefined function
// gaml::multidim::learner to set up such multi-dimentional
// predictors. This is what the main does.

#define DATA_SIZE 100
int main(int argc, char* argv[]) {

  // random seed initialization
  std::srand(std::time(0));

  // Database initialization.
  DataSet basis(DATA_SIZE);
  int i=0;
  for(auto& data : basis) {
    data.first    = i++;
    data.second.x = 1 + gaml::random::uniform(-1,1);
    data.second.y = 2 + gaml::random::uniform(-1,1);
    data.second.z = 3 + gaml::random::uniform(-1,1);
  }

  auto kfold_evaluator = gaml::risk::cross_validation(Loss(), gaml::partition::kfold(6), true);
  silly::Learner scalar_learner;
  auto learner = gaml::multidim::learner<Y,3>(scalar_learner, array_of_output, output_of_array);

  double    risk = kfold_evaluator(learner, basis.begin(), basis.end(), input_of_data, output_of_data);
  auto predictor = learner(basis.begin(), basis.end(), input_of_data, output_of_data);
  Y output       = predictor(0);
  std::cout << "output : {" << output.x << ", " << output.y << ", " << output.z << "}" << std::endl
  	    << "risk = " << risk << std::endl;

  return 0;
}
