// This example uses a MLP to regress the sinc function
// It shows how to use UKF learner to train the MLP
// and makes use of the ml library tools to compute the 
// empirical risk and estimate the real risk

#include "gaml-mlp.hpp"
#include <algorithm>

#define NB_SAMPLES 100
#define INPUT_DIM 1
#define HIDDEN_LAYER_SIZE 5
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

Y output_of(gaml::mlp::values_type::const_iterator iter)
{
  return {{ *iter }}; // as we use a std::array for Y, we directly build the result
}

const X& input_of_data (const Data& data) { return data.first; }
const Y& output_of_data(const Data& data) { return data.second;}

Y oracle(X x)
{
  Y y;

  if(x[0] != 0)
    y[0] = sin(x[0])/x[0];
  else
    y[0] = 1.0;

  return y;
}

Y noisy_oracle(X x)
{
  Y y = oracle(x);
  y[0] += gaml::random::uniform(-0.1, 0.1);
  return y;
}

int main(int argc, char * argv[])
{
  srand(time(NULL));

  // Let us define a 3 layer perceptron architecture
  auto input = gaml::mlp::input<X>(INPUT_DIM, fillInput);
  auto l1 = gaml::mlp::layer(input, HIDDEN_LAYER_SIZE, gaml::mlp::mlp_sigmoid(), gaml::mlp::mlp_dsigmoid());
  auto output = gaml::mlp::layer(l1, OUTPUT_DIM, gaml::mlp::mlp_identity(), gaml::mlp::mlp_didentity());
  auto mlp = gaml::mlp::perceptron(output, output_of);

  // Create a training base
  // Let us try to fit a noisy sinc function
  Basis basis;
  basis.resize(NB_SAMPLES);
  for(auto& d: basis)
    {
      d.first = {{ -10.0 + 20.0 * gaml::random::uniform(0.0, 1.0) }} ;
      d.second = noisy_oracle(d.first);
    }

  // Set up the parameters for learning the MLP with UKF
  gaml::mlp::learner::ukf::parameter ukf_params;
  ukf_params.alpha = 1e-2;
  ukf_params.beta = 2.0;
  ukf_params.kpa = 0.0;
  ukf_params.observation_noise = 1e-3;
  ukf_params.prior = sqrt(1e-1);
  ukf_params.evolution_noise_init = 1e-3;
  ukf_params.evolution_noise_decay = 0.99;
  ukf_params.evolution_noise_min = 1e-8;

  // The stopping criteria
  ukf_params.max_iter = 1000; 
  ukf_params.min_dparams = 1e-4;

  // Create the learner
  auto learning_algorithm = gaml::mlp::learner::ukf::algorithm(mlp, ukf_params);
  
  // Call the learner on the basis and get the learned predictor
  auto predictor = learning_algorithm(basis.begin(),
				      basis.end(),
				      input_of_data,
				      output_of_data);
  
  // Print out the structure of the perceptron
  std::cout << predictor << std::endl;

  // Dump the results
  std::ofstream outfile("example-002-samples.data");
  for(auto& b: basis)
    outfile << b.first[0] << " "
	    << b.second[0] << " " 
	    << std::endl;
  outfile.close();

  outfile.open("example-002-regression.data");
  X x;
  for(x[0] = -10; x[0] < 10 ; x[0] += 0.1)
    {
      auto output = predictor(x);
      outfile << x[0]         << " "
	      << oracle(x)[0] << " "
	      << output[0]    << std::endl;
    }
  outfile.close();

  
  std::cout << "You can plot the results using gnuplot :" << std::endl;
  std::cout << "gnuplot " << ML_MLP_SHAREDIR << "/plot-example-002.gplot" << std::endl;
  std::cout << "This will produce example-002.ps" << std::endl;

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

  double kfold_risk = kfold_evaluator(learning_algorithm,
				      basis.begin(),basis.end(),
				      input_of_data,output_of_data);

  std::cout << "Estimation of the real risk (6-fold): "
	    << kfold_risk << std::endl;

  return 0;
}
