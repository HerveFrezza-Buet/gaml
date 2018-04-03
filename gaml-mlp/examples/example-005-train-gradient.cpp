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


void fillInput(gaml::mlp::values_type::iterator iter, const X& x) {
  for(auto& xi: x)
    *(iter++) = xi;
}

void fillOutput(gaml::mlp::values_type::iterator iter, const Y& y) {
  for(auto& yi: y)
    *(iter++) = yi;
}

Y output_of(gaml::mlp::values_type::const_iterator iter)
{
  Y y;
  for(auto& vy: y)
    vy = *(iter++);
  return y;
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

Y noisy_oracle(X x, double noise_value)
{
  Y y = oracle(x);
  y[0] += noise_value;
  return y;
}

int main(int argc, char * argv[])
{
  std::random_device rd;
  std::mt19937 gen(rd());

  // Let us define a 3 layer perceptron architecture
  auto input = gaml::mlp::input<X>(INPUT_DIM, fillInput);
  auto l1 = gaml::mlp::layer(input, HIDDEN_LAYER_SIZE, gaml::mlp::mlp_sigmoid(), gaml::mlp::mlp_dsigmoid(), gen);
  auto l2 = gaml::mlp::layer(l1, HIDDEN_LAYER_SIZE, gaml::mlp::mlp_sigmoid(), gaml::mlp::mlp_dsigmoid(), gen);
  auto output = gaml::mlp::layer(l2, OUTPUT_DIM, gaml::mlp::mlp_identity(), gaml::mlp::mlp_didentity(), gen);
  auto mlp = gaml::mlp::perceptron(output, output_of);

  // Create a training base
  // Let us try to fit a noisy sinc function
  auto uniform_real = std::uniform_real_distribution<>(-1.0, 1.0);
  auto rnd_func = [&gen, &uniform_real]() { return uniform_real(gen);};
  Basis basis;
  basis.resize(NB_SAMPLES);
  for(auto& d: basis)
    {
      d.first = {{ 10.0*rnd_func() }} ;
      d.second = noisy_oracle(d.first, 0.1 * rnd_func());
    }

  // Set up the parameters for learning the MLP with a gradient descent
  gaml::mlp::learner::gradient::parameter gradient_params;
  gradient_params.alpha = 1e-2;
  gradient_params.dalpha = 1e-3;

  gradient_params.verbose = true;
  // The stopping criteria
  gradient_params.max_iter = 10000;
  gradient_params.min_dparams = 1e-7;

  // Create the learner
  auto learning_algorithm = gaml::mlp::learner::gradient::algorithm(mlp, gradient_params, gaml::mlp::loss::Quadratic(), fillOutput, gen);

  // Call the learner on the basis and get the learned predictor
  auto predictor = learning_algorithm(basis.begin(),
				      basis.end(),
				      input_of_data,
				      output_of_data);

  // Print out the structure of the perceptron we learned
  std::cout << predictor << std::endl;

  // Dump the results
  std::ofstream outfile("example-005-samples.data");
  for(auto& b: basis)
    outfile << b.first[0] << " "
	    << b.second[0] << " " 
	    << std::endl;
  outfile.close();

  outfile.open("example-005-regression.data");
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
  std::cout << "gnuplot " << ML_MLP_SHAREDIR << "/plot-example-005.gplot" << std::endl;
  std::cout << "This will produce example-005.ps" << std::endl;


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


}
