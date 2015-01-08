// This example performs several tests to ensure that the computation
// of the gradient of the loss function is not buggy :)

#include "gaml-mlp.hpp"
#include <algorithm>
#include <cfloat>

#define INPUT_DIM 2
#define HIDDEN_LAYER_SIZE 5
#define OUTPUT_DIM 2

typedef std::array<double, INPUT_DIM> X;
typedef std::array<double, OUTPUT_DIM> Y;
typedef std::pair<X, Y> Data;
typedef std::vector<Data> Basis;

void fillInput(gaml::mlp::values_type::iterator iter, const X& x) {
  for(auto& xi: x)
    *(iter++) = xi;
}

Y output_of(gaml::mlp::values_type::const_iterator iter)
{
  Y y;
  for(auto& vy: y)
    vy = *(iter++);
  return y;
}
void fillOutput(gaml::mlp::values_type::iterator iter, const Y& y) {
  for(auto& yi: y)
    *(iter++) = yi;
}

template<typename datatype>
void randomize_data(datatype& x, double minv=0.0, double maxv = 1.0) {
  for(auto& xi: x) 
    xi = minv + (maxv - minv) * std::rand() / double(RAND_MAX - 1);
}

int main(int argc, char* argv[]) {

  if(argc != 2) {
    std::cerr << "Usage : " << argv[0] << "<0,1>" <<std::endl;
    std::cerr << "with : " << std::endl;
    std::cerr << "0 : quadratic loss" << std::endl;
    std::cerr << "1 : cross entropy loss" << std::endl;
    return -1;
  }

  bool quadratic_loss = (atoi(argv[1]) == 0);

  srand(time(NULL));

  // We compare our computation of the gradient to 
  // a finite difference approximation
  // The loss is also involved
  std::cout << "---------------------------------" << std::endl;
  std::cout << "Comparing the analytical gradient and numerical approximation " << std::endl;
  auto input = gaml::mlp::input<X>(INPUT_DIM, fillInput);
  auto l1 = gaml::mlp::layer(input, HIDDEN_LAYER_SIZE, gaml::mlp::mlp_sigmoid(), gaml::mlp::mlp_dsigmoid());
  auto l2 = gaml::mlp::layer(l1, HIDDEN_LAYER_SIZE, gaml::mlp::mlp_identity(), gaml::mlp::mlp_didentity());
  auto l3 = gaml::mlp::layer(l2, HIDDEN_LAYER_SIZE, gaml::mlp::mlp_tanh(), gaml::mlp::mlp_dtanh());
  auto l4 = gaml::mlp::layer(l3, OUTPUT_DIM, gaml::mlp::mlp_sigmoid(), gaml::mlp::mlp_dsigmoid());
  auto mlp = gaml::mlp::perceptron(l4, output_of);

  std::cout << "We use the following architecture : " << std::endl;
  std::cout << mlp << std::endl;
  std::cout << "which has a total of " << mlp.psize() << " parameters"<< std::endl;

  gaml::mlp::parameters_type params(mlp.psize());
  gaml::mlp::parameters_type paramsph(mlp.psize());
  gaml::mlp::values_type derivatives(mlp.psize());
  gaml::mlp::values_type forward_sweep(mlp.size());
  X x;

  auto loss_ce = gaml::mlp::loss::CrossEntropy();
  auto loss_quadratic = gaml::mlp::loss::Quadratic();

  auto f = [&mlp, &params] (const typename decltype(mlp)::input_type& x) -> gaml::mlp::values_type {
    auto output = mlp(x, params);
    gaml::mlp::values_type voutput(mlp.output_size());
    fillOutput(voutput.begin(), output);
    return voutput;
  };
  auto df = [&mlp, &forward_sweep, &params] (const typename decltype(mlp)::input_type& x, unsigned int parameter_dim) -> gaml::mlp::values_type {
    return mlp.deriv(x, params, forward_sweep, parameter_dim);
  };

  unsigned int nbtrials = 100;
  unsigned int nbfails = 0;
  std::cout << "I will compare " << nbtrials << " times a numerical approximation and the analytical gradient we compute" << std::endl;


  for(unsigned int t = 0 ; t < nbtrials ; ++t) {
  
    randomize_data(params, -1.0, 1.0);
    randomize_data(x, -1.0, 1.0);

    // Compute the output at params
    auto output = mlp(x, params);
    gaml::mlp::values_type raw_output(OUTPUT_DIM);
    fillOutput(raw_output.begin(), output);
    gaml::mlp::values_type raw_outputph(OUTPUT_DIM);

    // For computing the loss, we need a target
    gaml::mlp::values_type raw_target(OUTPUT_DIM);
    randomize_data(raw_target);

    double norm_dh = 0.0;

    for(unsigned int i = 0 ; i < mlp.psize() ; ++i) {
      // Let us compute params + h*[0 0 0 0 0 0 1 0 0 0 0 0], the 1 at the ith position
      std::copy(params.begin(), params.end(), paramsph.begin());
      double dh = (sqrt(DBL_EPSILON) * paramsph[i]);
      paramsph[i] += dh;
      norm_dh += dh*dh;
      // Compute the output at params + h
      auto outputph = mlp(x, paramsph);
      fillOutput(raw_outputph.begin(), outputph);
      
      // We now compute the approximation of the derivative
      if(quadratic_loss)
	derivatives[i] = (loss_quadratic(raw_target, raw_outputph) - loss_quadratic(raw_target, raw_output))/dh;
      else
	derivatives[i] = (loss_ce(raw_target, raw_outputph) - loss_ce(raw_target, raw_output))/dh;
	
    }
  
    // We now compute the analytical derivatives
    mlp(x, params);
    std::copy(mlp.begin(), mlp.end(), forward_sweep.begin());

    gaml::mlp::values_type our_derivatives(mlp.psize());
    for(unsigned int i = 0 ; i < mlp.psize() ; ++i) {
      if(quadratic_loss)
	our_derivatives[i] = loss_quadratic.deriv(x, raw_target, forward_sweep, f, df, i);
      else
	our_derivatives[i] = loss_ce.deriv(x, raw_target, forward_sweep, f, df, i);
	
    }
  
    // We finally compute the norm of the difference
    double error = 0.0;
    auto diter = derivatives.begin();
    for(auto& ourdi : our_derivatives) {
      error = (ourdi - *diter) * (ourdi - *diter);
      diter++;
    }
    error = sqrt(error);
    std::cout << "Error between the analytical and numerical gradients " << error << " with a step size of " << sqrt(norm_dh) << " in norm" << std::endl;
    if(error > 1e-7) 
      ++nbfails;

    /*
    std::cout << "numerical " << std::endl;
    for(auto & di : derivatives)
      std::cout << di << " ";
    std::cout << std::endl;
    std::cout << "our :" << std::endl;
    for(auto& di : our_derivatives)
      std::cout << di << " ";
    std::cout << std::endl;
    */

  }

  std::cout << nbfails << " / " << nbtrials << " with an error higher than 1e-7" << std::endl;
}
