#include <gaml.hpp>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <utility>
#include <numeric>

// This example is about how to use data projections over a subset of
// attributes.  The example considers the subset of the 4th, 5th and
// 8th attribute. Projections are used for variable selection (see
// next examples).

#define DATA_SIZE        10
#define ATTRIBUTE_NUMBER 10

typedef std::array<double, ATTRIBUTE_NUMBER> X;
typedef bool                                 U;
typedef std::pair<X, U>                      Data;
typedef std::vector<Data>                    DataSet;

DataSet build_dataset() {
  DataSet dataset;
  dataset.resize(DATA_SIZE);
  for (auto& d : dataset) {
    for (auto& attr : d.first)
      attr = gaml::random::uniform(0, 1);
    d.second = gaml::random::proba(.5);
  }

  return dataset;
}

template<typename DATASET>
void print_dataset(const DATASET& dataset) {
  for (auto& data : dataset) {
    for (auto& var : data.first)
      std::cout << var << ' ';
    if (data.second)
      std::cout << ": true" << std::endl;
    else
      std::cout << ": false" << std::endl;
  }
}

const X& input_of_data(const Data& data) {
  return data.first;
}

U output_of_data(const Data& data) {
  return data.second;
}

int main(int argc, char* argv[]) {

  // random seed initialization
  std::srand(std::time(0));

  // configuration of the printing for numbers.
  std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(3);

  DataSet dataset = build_dataset();

  // The set of indices of variables to select (indices start at 0)
  typedef std::array<size_t, 3> Idx;

  Idx indices = { { 3, 4, 8 } };

  // Creates the projection of a dataset
  auto projection = gaml::project(dataset.begin(),  dataset.end(),  // Range of data
				  indices.cbegin(), indices.cend(), // Range of indices
				  input_of_data, output_of_data);

  // Outputs the initial dataset
  std::cout << "Initial dataset:" << std::endl << std::endl;
  print_dataset(dataset);
  std::cout << std::endl << std::endl;

  // Outputs the projected dataset
  std::cout << "Projected dataset on variables of index {";
  for (int attr : indices) std::cout << ' ' << attr;
  std::cout << " }:" << std::endl << std::endl;

  // Making a projection implies using internal types, defined by
  // gaml. They can be manipulated by dedicated type traits, enabling
  // to retrieve the types.
  typedef gaml::projection_traits<X, U, Idx::const_iterator> Traits;

  typedef Traits::input_type           Input;          // This is X here.
  typedef Traits::output_type          Output;         // This is the initial U which is the same before and after projection
  typedef Traits::projected_input_type ProjectedInput; // This is the type of the input after projection (a collection of the projected dimensions)
  typedef Traits::projected_data_type  ProjectedData;  // This is the type of the pair ProjectedInput/Output.
  
  typedef decltype(projection)         Projection;     // We get the projection type from our 'projection' variable.

  // Let us compare the first initial data with the first projected data.
  // Making types explicit is for clarification purpose. Inference type (auto) could be used instead.
  auto& initialData = *(dataset.begin());			// Get the first data
  const Input& initialInput = input_of_data(initialData); // Retrieve the input from it.
  const Output& initialOutput = output_of_data(initialData); // Retrieve the output from it.

  std::cout << "First initial data is (";
  for (auto& val : initialInput) std::cout << ' ' << val;
  std::cout << " ), label is " << initialOutput << ", input dimension is " << initialInput.size() << std::endl << std::endl;

  const ProjectedData& projectedData   = *(projection.begin());      // Get the first projected data.
  const ProjectedInput& projectedInput  = Projection::inputOf(projectedData);  // Retrieve the input from it.
  const Output& projectedOutput = Projection::outputOf(projectedData); // Retrieve the output from it.

  std::cout << "First projected data is (";
  for (auto& val : projectedInput) std::cout << ' ' << val;
  std::cout << " ), label is " << projectedOutput << ", input dimension is " << projectedInput.size() << std::endl << std::endl;

  // Let us print the whole projected dataset.
  print_dataset(projection);

  return EXIT_SUCCESS;
}