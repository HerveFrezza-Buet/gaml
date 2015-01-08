#include <gaml-mlp.hpp>

// This example shows how to create a 2-10-2 MLP
// The transfer functions being:
// input layer : identity (you don't have the choice)
// hidden layer : sigmoid
// output layer : identity
// This example also shows how to interface custom data structure with the MLP


// This is the data we want to feed the MLP with
class Complex {
public:
  double re, im;
};

// This is the data we want to get from the MLP
class Attributes {
public:
  double norm, arg;
};

// We write an helper which will be used by the input layer 
// to convert the Complex data to the internal datastructure of the MLP
void fillInput(gaml::mlp::values_type::iterator iter, const Complex& c)
{
  *(iter++) = c.re;
  *(iter++) = c.im;
}

// We write an helper which is used by the MLP
// to convert the internal datastructure of the MLP into Complex data to the internal datastructure of the MLP
Attributes output_of(gaml::mlp::values_type::const_iterator iter)
{
  Attributes a;
  a.norm = (*iter++);
  a.arg = (*iter++);
  return a;
}

int main(int argc, char* argv[])
{
  srand(time(NULL));
  
  //* We first define the architecture of the MLP
  // Create the input layer
  // When giving the size of the layers, you don't need to add an extra neuron for the bias 
  auto input = gaml::mlp::input<Complex>(2, fillInput);
  // The hidden layer on top of the input layer
  auto l1 = gaml::mlp::layer(input, 10, gaml::mlp::mlp_sigmoid(), gaml::mlp::mlp_dsigmoid());
  // The output layer
  auto output = gaml::mlp::layer(l1, 2, gaml::mlp::mlp_identity(), gaml::mlp::mlp_didentity());
  //*                                            *//

  // The mlp is defined from the last layer and the helper to create 
  // a result of type Attributes
  auto mlp = gaml::mlp::perceptron(output, output_of);

  // Display some informations about the perceptron
  std::cout << "The perceptron has a total of " << mlp.size() << " units "
	    << "and " << mlp.psize() << " parameters " << std::endl;

  std::cout << mlp << std::endl;

  // Create the parameter vector
  auto params = mlp.params();
  // Randomly initialize the parameters
  mlp.init_params(params);

  // Define an example input
  Complex c;
  c.re = 1.0;
  c.im = 5.0;

  // Evaluate the output
  auto res = mlp(c, params);

  std::cout << "Input : " << std::endl;
  std::cout << " re : " << c.re << std::endl;
  std::cout << " im : " << c.im << std::endl;
  std::cout << std::endl;

  std::cout << "Attributes : " << std::endl;
  std::cout << " Norm: " << res.norm << std::endl;
  std::cout << " Arg : " << res.arg << std::endl;
  std::cout << std::endl;

  // We can iterate over all the values of the MLP:
  std::cout << "Activities within the MLP:" << std::endl;;
  for(auto& v: mlp)
    std::cout << v << " ";
  std::cout << std::endl;
  std::cout << std::endl;

  // Or iterate over the values of some selected layers
  std::cout << "We now iterate on each layer individually" << std::endl;
  for(unsigned int i = 0 ; i < mlp.depth() ; ++i)
    {
      std::cout << "Layer " << i << std::endl;
      for(auto& v: mlp.layer_values(i))
	std::cout << v << " ";
      std::cout << std::endl;
    }  
}
