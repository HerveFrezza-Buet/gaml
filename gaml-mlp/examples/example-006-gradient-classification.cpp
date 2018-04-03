// This example uses a MLP for classification of the extended XOR
// The output of the MLP is a probability distribution over classes

#include "gaml-mlp.hpp"
#include <algorithm>

#define INPUT_DIM 2
#define HIDDEN_LAYER_SIZE 5
#define K  2 // The number of classes
#define NB_SAMPLES_PER_CLASS 100


typedef std::array<double, INPUT_DIM> X;
typedef std::array<double, K> Y;
typedef std::pair<X, Y> Data;
typedef std::vector<Data> Basis;

void color_of(Y& y, int& r, int& g, int& b)
{
  Y yvec = {{ -1.0/sqrt(2.0), 1.0/sqrt(2.0) }};
  double s = y[0] * yvec[0] + y[1] * yvec[1];
  if(s >= 0)
    {
      r = 255; 
      double d = sqrt((0 - y[0])*(0 - y[0]) + (1 - y[1])*(1 - y[1]));
      if(d >= sqrt(2.0)/2.0)
	d = sqrt(2.0)/2.0;
      g = b = int(d/(sqrt(2.0)/2.0) * 255);
    }
  else
    {
      b = 255;
      double d = sqrt((1 - y[0])*(1 - y[0]) + (0 - y[1])*(0 - y[1]));
      if(d >= sqrt(2.0)/2.0)
	d = sqrt(2.0)/2.0;
      r = g = int(d/(sqrt(2.0)/2.0) * 255);
    }
}

void fillInput(gaml::mlp::values_type::iterator iter, const X& x)
{
  for(auto& xi: x)
    *(iter++) = xi;
}
void fillOutput(gaml::mlp::values_type::iterator iter, const Y& y){
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


int main(int argc, char * argv[])
{
  std::random_device rd;
  std::mt19937 gen(rd()); 

  // Let us define a 3 layer perceptron architecture
  auto input = gaml::mlp::input<X>(INPUT_DIM, fillInput);
  auto l1 = gaml::mlp::layer(input, HIDDEN_LAYER_SIZE, gaml::mlp::mlp_sigmoid(), gaml::mlp::mlp_dsigmoid(), gen);
  auto output = gaml::mlp::layer(l1, K, gaml::mlp::mlp_softmax(), gaml::mlp::mlp_dsoftmax(), gen);
  auto mlp = gaml::mlp::perceptron(output, output_of);

  // Create a training base with 2 classes
  // The first class is the domain  [0, 0.5] x [0, 1]
  // The second class it the domain [0.5, 1] x [0, 1]
  // We add some jitter on this position in [-0.1, 0.1]
  auto uniform_real = std::uniform_real_distribution<>(0.0, 1.0);
  auto rnd_func = [&gen, &uniform_real]() { return uniform_real(gen);};
  Basis basis;
  for(unsigned int i = 0 ; i < NB_SAMPLES_PER_CLASS; ++i)
    {
      Data d;
      d.first = {{ 0.5*rnd_func() + (-0.1 + 0.2 * rnd_func()), rnd_func() }};
      d.second = {{ 0.0, 1.0 }};
      basis.push_back(d);
    }
  for(unsigned int i = 0 ; i < NB_SAMPLES_PER_CLASS; ++i)
    {
      Data d;
      d.first = {{ 0.5 + 0.5*rnd_func() + (-0.1 + 0.2 * rnd_func()), rnd_func() }};
      d.second = {{ 1.0, 0.0 }};
      basis.push_back(d);
    }

  // Set up the parameters for learning the MLP with a gradient descent
  gaml::mlp::learner::gradient::parameter gradient_params;
  gradient_params.alpha = 1e-2;
  gradient_params.dalpha = 1e-3;

  gradient_params.verbose = true;
  // The stopping criteria
  gradient_params.max_iter = 10000;
  gradient_params.min_dparams = 1e-4;

  // Create the learner
  auto learning_algorithm = gaml::mlp::learner::gradient::algorithm(mlp, gradient_params, gaml::mlp::loss::CrossEntropy(), fillOutput, gen);

  // Call the learner on the basis and get the learned predictor
  auto predictor = learning_algorithm(basis.begin(),
				      basis.end(),
				      input_of_data,
				      output_of_data);

  // Print out the structure of the perceptron we learned
  std::cout << predictor << std::endl;

  // Dump the results
  std::ofstream outfile("example-006-samples.data");
  for(auto& b: basis)
    outfile << b.first[0] << " "
	    << b.first[1] << " "
	    << b.second[0] << " " 
	    << std::endl;
  outfile.close();
  std::cout << "The samples used for learning are saved in example-006-samples.data" << std::endl;
  
  int nb_samples = 100;
  outfile.open("example-006.ppm");
  outfile << "P3" << std::endl;
  outfile << nb_samples << " " << nb_samples << std::endl;
  outfile << "255" << std::endl;
  double step = 1.0 / (nb_samples - 1);
  int r, g, b;
  X x;
  for(x[1] = 1; x[1] > 0 ; x[1] -= step)
    {
      for(x[0] = 0 ; x[0] < 1 ; x[0] += step)
	{
	  auto output = predictor(x);
	  color_of(output, r, g, b);
	  outfile << r << " " << g << " " << b << " ";
	}
      outfile << std::endl;
    }
  outfile.close();
  std::cout << "The classification results are saved in example-006.ppm" << std::endl;


  auto evaluator = gaml::risk::empirical(gaml::mlp::loss::Quadratic());
  double risk = evaluator(predictor,
			  basis.begin(),
			  basis.end(),
			  input_of_data,
			  output_of_data);
  std::cout << "Empirical risk : " << risk << std::endl;

  /*
  for(auto& bi : basis) {
    auto y = predictor(bi.first);
    std::cout << bi.first[0] << " "
	      << bi.first[1] << " "
	      << bi.second[0] << " " 
	      << bi.second[1] << " -> "
	      << y[0] << " " << y[1] << std::endl;
  }
  */

}
