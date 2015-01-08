// This example uses a MLP for classification of the extended XOR
// The output of the MLP is a probability distribution over the classes
#include <gaml-mlp.hpp>

#define INPUT_DIM 2
#define HIDDEN_LAYER_SIZE 5
#define K         2  // The number of classes
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
      g = b = d/(sqrt(2.0)/2.0) * 255;
    }
  else
    {
      b = 255;
      double d = sqrt((1 - y[0])*(1 - y[0]) + (0 - y[1])*(0 - y[1]));
      if(d >= sqrt(2.0)/2.0)
	d = sqrt(2.0)/2.0;
      r = g = d/(sqrt(2.0)/2.0) * 255;
    }
}


void fillInput(gaml::mlp::values_type::iterator iter, const X& x)
{
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

const X& input_of_data (const Data& data) { return data.first; }
const Y& output_of_data(const Data& data) { return data.second;}

int main(int argc, char * argv[])
{
  
  srand(time(NULL));

  // Let us define a 3 layer perceptron architecture
  auto input = gaml::mlp::input<X>(INPUT_DIM, fillInput);
  auto l1 = gaml::mlp::layer(input, HIDDEN_LAYER_SIZE, gaml::mlp::mlp_sigmoid(), gaml::mlp::mlp_dsigmoid());
  auto output = gaml::mlp::layer(l1, K, gaml::mlp::mlp_softmax(), gaml::mlp::mlp_dsoftmax());
  auto mlp = gaml::mlp::perceptron(output, output_of);

  // Create a training base with 2 classes
  // The first class is the domain  [0, 0.5] x [0, 1]
  // The second class it the domain [0.5, 1] x [0, 1]
  // We add some jitter on this position in [-0.1, 0.1]
  Basis basis;
  for(unsigned int i = 0 ; i < NB_SAMPLES_PER_CLASS; ++i)
    {
      Data d;
      d.first = {{ gaml::random::uniform(0, 0.5) + gaml::random::uniform(-0.1, 0.1), gaml::random::uniform(0, 1.0) }};
      d.second = {{ 0.0, 1.0 }};
      basis.push_back(d);
    }
  for(unsigned int i = 0 ; i < NB_SAMPLES_PER_CLASS; ++i)
    {
      Data d;
      d.first = {{ gaml::random::uniform(0.5, 1.0) + gaml::random::uniform(-0.1, 0.1), gaml::random::uniform(0, 1.0) }};
      d.second = {{ 1.0, 0.0 }};
      basis.push_back(d);
    }

  // Set up the parameters for learning the MLP with UKF
  gaml::mlp::learner::ukf::parameter ukf_params;
  ukf_params.alpha = 1e-3;
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

  // Create the learner
  auto learning_algorithm = gaml::mlp::learner::ukf::algorithm(mlp, ukf_params);
  
  // Call the learner on the basis and get the learned predictor
  auto predictor = learning_algorithm(basis.begin(),
				      basis.end(),
				      input_of_data,
				      output_of_data);

  std::ofstream outfile("example-004-samples.data");
  for(auto& b: basis)
    outfile << b.first[0] << " "
	    << b.first[1] << " "
	    << b.second[0] << " "  // for this specific example, we use output[0] as the label :)
	    << std::endl;
  outfile.close();
  std::cout << "The samples used for learning are saved in example-004-samples.data" << std::endl;

  /*
    outfile.open("example-004-regression.data");
    X x;
    for(x[0] = 0; x[0] < 1 ; x[0] += 0.05)
    for(x[1] = 0 ; x[1] < 1 ; x[1] += 0.05)
    {
    auto output = predictor(x);
    outfile << x[0]         << " "
    << x[1]         << " "
    << output[0]    << std::endl;
    }
    outfile.close();
  */

  int nb_samples = 100;
  outfile.open("example-004.ppm");
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
  std::cout << "The classification results are saved in example-004.ppm" << std::endl;

}
