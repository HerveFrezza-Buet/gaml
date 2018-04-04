#include <gaml.hpp>
#include <gaml-xtree.hpp>

#include <fstream>
#include <vector>
#include <array>
#include <utility>
#include <cmath>
#include <random>

#define N_MIN          10
#define DIM             2
#define XMIN            0
#define XMAX           10
#define PLOT_STEP      .1
#define NB_SAMPLES   1000
#define NOISE          .2

typedef std::array<double,DIM> X;
typedef double                 Y;
typedef std::pair<X,Y>         Data;
typedef std::vector<Data>      Basis;

const X& input_of (const Data& d) {return d.first;}
const Y  output_of(const Data& d) {return d.second;}

Y oracle(const X& x) {
  return std::sin(x[0])*std::sin(x[1]);
}

template<typename RANDOM_DEVICE>
Data sample(RANDOM_DEVICE& rd) {
  std::uniform_real_distribution<double> uniform(  XMIN,  XMAX);
  std::uniform_real_distribution<double> noise  (-NOISE, NOISE);
  X x = {{uniform(rd), uniform(rd)}};
  return {x, oracle(x) + noise(rd)};
}
	      

int main(int argc, char* argv[]) {

  if(argc != 2) {
    std::cout << "Usage : " << argv[0] << " <forest-size>" << std::endl;
    return 0;
  }
  
  // random seed initialization
  std::random_device rd;
  std::mt19937 gen(rd());
  
  unsigned int forest_size = (unsigned int)(atoi(argv[1]));
  Basis basis(NB_SAMPLES);
  for(auto& xy : basis) xy = sample(gen);

  auto learner = gaml::xtree::regression::learner<X,Y,gaml::score::RelativeVarianceReduction>(N_MIN, DIM, gen);
  std::cout << "Learning a single tree... " << std::flush;
  auto predictor = learner(basis.begin(), basis.end(), input_of, output_of);
  std::cout << "done." << std::endl;

  std::ofstream ofile("xtree.pred");
  ofile << predictor;
  ofile.close();
  
  // Let us load the predictor (instead of using predictor directly).
  gaml::xtree::regression::Predictor<X,Y> tree;
  std::ifstream ifile("xtree.pred");
  ifile >> tree;
  ifile.close();

  // Now, let us set up a forest rather than a single tree.
  auto forest_learner = gaml::bag::learner(learner,gaml::Average(),gaml::bag::set::Identity(),forest_size,true);
  std::cout << "Learning a forest... " << std::flush;
  auto forest = forest_learner(basis.begin(), basis.end(), input_of, output_of);
  std::cout << "done." << std::endl;
  

  // This does the plotting


  std::cout << std::endl
  	    << "Plotting..." << std::endl
  	    << std::endl;

  std::ofstream samples("samples.data");
  for(auto& xy : basis)
    samples << xy.first[0] << ' ' << xy.first[1] << ' ' << xy.second << std::endl;
  samples.close();

  {
    std::ofstream data("xtree.data");
    X input;
    double& x0 = input[0];
    double& x1 = input[1];
    for(x0 = XMIN; x0 <= XMAX; x0 += PLOT_STEP, data << std::endl)
      for(x1 = XMIN; x1 <= XMAX; x1 += PLOT_STEP)
  	data << x0 << ' ' << x1 << ' ' << tree(input) << std::endl;
    data.close();

    std::ofstream plot("xtree.plot");
    plot << "set hidden3d" << std::endl
  	 << "set ticslevel 0" << std::endl
  	 << "splot 'xtree.data' with lines, 'samples.data' pt 7 ps .5" << std::endl;
    plot.close();
    std::cout << "gnuplot -p xtree.plot" << std::endl;
  }

  {
    std::ofstream data("forest.data");
    X input;
    double& x0 = input[0];
    double& x1 = input[1];
    for(x0 = XMIN; x0 <= XMAX; x0 += PLOT_STEP, data << std::endl)
      for(x1 = XMIN; x1 <= XMAX; x1 += PLOT_STEP)
  	data << x0 << ' ' << x1 << ' ' << forest(input) << std::endl;
    data.close();

    std::ofstream plot("forest.plot");
    plot << "set hidden3d" << std::endl
  	 << "set ticslevel 0" << std::endl
  	 << "splot 'forest.data' with lines, 'samples.data' pt 7 ps .5" << std::endl;
    plot.close();
    std::cout << "gnuplot -p forest.plot" << std::endl;
  }


  return 0;
}
