// This example uses a MLP to regress the sinc function
// It demonstrates how to make use of the ML library
// tools for finding the architecture allowing better generalization
// The perceptron is learned with UKF

#include <gaml-mlp.hpp>
#include <algorithm>
#include <functional>

#define NB_SAMPLES 1000
#define INPUT_DIM 1
#define OUTPUT_DIM 1

typedef std::array<double, INPUT_DIM> X;
typedef std::array<double, OUTPUT_DIM> Y;
typedef std::pair<X, Y> Data;
typedef std::vector<Data> Basis;

void fillInput(gaml::mlp::values_type::iterator iter, const X& x)
{
  for(auto& xi: x)
    *(iter++) = xi;
}

Y output_of_mlp(gaml::mlp::values_type::const_iterator iter)
{
  Y y;
  for(auto& vy: y)
    vy = *(iter++);
  return y;
}

const X& input_of_data (const Data& data) { return data.first; }
const Y& output_of_data(const Data& data) { return data.second;}

// Let us define a procedure for finding an architecture
// minimizing the real risk estimated with kfold cross validation

template<typename RANDOM_DEVICE>
class MetaLearner
{ 

private:
  gaml::mlp::learner::ukf::parameter ukf_params;

public:  
  typedef gaml::mlp::InputLayer<X> input_layer_type;
  typedef gaml::mlp::Layer<input_layer_type, RANDOM_DEVICE> hidden_layer_type;
  typedef gaml::mlp::Layer<hidden_layer_type, RANDOM_DEVICE> output_layer_type;
  typedef decltype(gaml::mlp::perceptron(std::declval<output_layer_type>(), output_of_mlp)) mlp_type;
  typedef gaml::mlp::Predictor<mlp_type> predictor_type; 

  bool verbosity;
  RANDOM_DEVICE& rd;
  
  MetaLearner(bool verbose, RANDOM_DEVICE& rd) :
    verbosity(verbose),
    rd(rd) {
    
    // Set up the parameters for learning the MLP with UKF
    ukf_params.alpha = 1e-1;
    ukf_params.beta = 2.0;
    ukf_params.kpa = 0.0;
    ukf_params.observation_noise = 1e-3;
    ukf_params.prior = sqrt(1e-1);
    ukf_params.evolution_noise_init = 1e-3;
    ukf_params.evolution_noise_decay = 0.99;
    ukf_params.evolution_noise_min = 1e-8;

    // The stopping criteria
    ukf_params.max_iter = 1000; 
    ukf_params.min_dparams = 1e-3;

  }

  MetaLearner(const MetaLearner& other):
    ukf_params(other.ukf_params),
    verbosity(other.verbosity),
    rd(other.rd) {}


  MetaLearner& operator=(const MetaLearner& other)
  {
    if(&other != this)
      {
	ukf_params = other.ukf_params;
	verbosity = other.verbosity;
	rd = other.rd; 
      }
    return *this;
  }

  template<typename DataIterator, typename InputOf, typename OutputOf> 
  predictor_type operator()(const DataIterator &begin, 
			    const DataIterator &end, 
			    const InputOf & input_of, 
			    const OutputOf & output_of)  const {

    unsigned int optimal_hidden_size = -1;
    double optimal_risk = -1;

    const unsigned int max_hidden_size = 9;

    if(verbosity) std::cout << " hidden_size | kfold risk " << std::endl;
    
    for(unsigned int hidden_size = 1; hidden_size <= max_hidden_size ; ++hidden_size) {
      // Let us define an architecture with hidden_size units
      // in the hidden layer
      auto input = input_layer_type(INPUT_DIM, fillInput);
      auto l1 = hidden_layer_type(input, hidden_size, gaml::mlp::mlp_sigmoid(), gaml::mlp::mlp_dsigmoid(), rd);
      auto output = output_layer_type(l1, OUTPUT_DIM, gaml::mlp::mlp_identity(), gaml::mlp::mlp_didentity(), rd);
      auto mlp = mlp_type(output, output_of_mlp);
      
      // Create the learner
      auto learning_algorithm = gaml::mlp::learner::ukf::algorithm(mlp, ukf_params, rd);
      
      // We will use a 6-fold cross-validation
      auto kfold_evaluator = gaml::risk::cross_validation(gaml::mlp::loss::Quadratic(),
							  gaml::partition::kfold(3),
							  false);
      double kfold_risk = kfold_evaluator(learning_algorithm,
					  begin, end,
					  input_of,output_of);

      if(verbosity) std::cout << std::setw(12) << std::setfill(' ') << hidden_size
			      << " | " 
			      << std::scientific << std::setprecision(4)  << std::setw(10) << std::setfill(' ') << kfold_risk
			      << std::endl;
      

      if(optimal_risk == -1) {
	optimal_risk = kfold_risk;
	optimal_hidden_size = hidden_size;
      }
      else if(kfold_risk < optimal_risk) {
	optimal_risk = kfold_risk;
	optimal_hidden_size = hidden_size;
      }
    }

    // Let us train the optimal architecture on the whole data basis
    auto input = input_layer_type(INPUT_DIM, fillInput);
    auto l1 = hidden_layer_type(input, optimal_hidden_size, gaml::mlp::mlp_sigmoid(), gaml::mlp::mlp_dsigmoid(), rd);
    auto output = output_layer_type(l1, OUTPUT_DIM, gaml::mlp::mlp_identity(), gaml::mlp::mlp_didentity(), rd);
    auto mlp = mlp_type(output, output_of_mlp);

    auto learning_algorithm = gaml::mlp::learner::ukf::algorithm(mlp, ukf_params, rd);
    
    return learning_algorithm(begin, end,
			      input_of,
			      output_of);
  } 
};


int main(int argc, char * argv[])
{
  std::random_device rd;
  std::mt19937 gen(rd()); 

  auto uniform_real = std::uniform_real_distribution<>(-1.0, 1.0);
  auto noise_func = [&gen, &uniform_real]() { return uniform_real(gen);};
  
  // Create a training base
  // Let us try to fit a noisy sinc function
  Basis basis;
  basis.resize(NB_SAMPLES);
  for(auto& d: basis)
    {
      d.first = {{ 10.0 * noise_func() }} ;
      d.second = {{ sin(d.first[0])/d.first[0] + 0.1 * noise_func() }};
    }
  
  // Create the learner
  MetaLearner learning_algorithm(true, gen);
  
  std::cout << "Finding the optimal perceptron for the basis and train it..." << std::endl;

  // Call the learner on the basis and get the learned predictor
  auto predictor = learning_algorithm(basis.begin(),
				      basis.end(),
				      input_of_data,
				      output_of_data);

  std::cout << "done!" << std::endl;

  // Print ou the structure of the perceptron
  std::cout << predictor << std::endl;

  // Dump the results
  std::ofstream outfile("example-003.data");
  for(auto& b: basis)
    {
      auto output = predictor(b.first);
      outfile << b.first[0] << " "
	      << b.second[0] << " "
	      << output[0] << " " 
	      << std::endl;
    }
  outfile.close();
  std::cout << "You can plot the results using gnuplot :" << std::endl;
  std::cout << "gnuplot " << ML_MLP_SHAREDIR << "/plot-example-003.gplot" << std::endl;

  // Let us compute the empirical risk.
  auto evaluator = gaml::risk::empirical(gaml::mlp::loss::Quadratic());
  double risk = evaluator(predictor,
			  basis.begin(),
			  basis.end(),
			  input_of_data,
			  output_of_data);
  std::cout << "Empirical risk = " << risk << std::endl;

  // We will use a 6-fold cross-validation to estimate the real risk.
  auto kfold_evaluator = gaml::risk::cross_validation(gaml::mlp::loss::Quadratic(),
						      gaml::partition::kfold(6),
						      true);

  learning_algorithm.verbosity = false;

  double kfold_risk = kfold_evaluator(learning_algorithm,
				      basis.begin(),basis.end(),
				      input_of_data,output_of_data);

  std::cout << "Estimation of the real risk (6-fold): "
	    << kfold_risk << std::endl;

  return 0;
}
