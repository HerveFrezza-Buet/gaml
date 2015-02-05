#include <gaml.hpp>
#include <cstdlib>
#include <vector>
#include <utility>
#include <set>
#include <iostream>
#include <fstream>

#define CONFUSION_PROBABILITY .3
#define BASIS_SIZE            1000

// Let us consider scalar inputs, taken in [0,10[, and a 10-class
// problem. The class of an input x is (int)x, in [0,1,2...9].

typedef double                    X;      // The input type
typedef int                       U;      // The output type
typedef std::pair<X,U>            Data;   // The sample type
typedef std::vector<Data>         Basis;


X  input_of_data(const Data& data) {return data.first;}
U output_of_data(const Data& data) {return data.second;}
U class_of_label(U label)          {return label;} 

// This is our classifier. It makes a confusion : an odd class is
// confused with some other odd class, and the same stands for even
// classes. It fits gaml::concept::Predictor.
class Classifier {
public:
  typedef X input_type;
  typedef U output_type;

  output_type operator() (const input_type &x) const {
    bool confused = gaml::random::proba(CONFUSION_PROBABILITY);
    U lbl = (int)x;
    U tmp;

    if(confused) {
      tmp = lbl;
      switch(tmp) {
      case 0:case 2:case 4:case 6:case 8:  // Even labels
	while(lbl==tmp)
	  lbl = 2*((int)(gaml::random::uniform(0,5)));
	break;
      default: // Odd labels
	while(lbl==tmp)
	  lbl = 2*((int)(gaml::random::uniform(0,5)))+1;
	break;
      };
    }
    return lbl;
  }
};


// This generates a sample basis according to our labelling rules.
Basis generate_data() {
  Basis basis;
  basis.resize(BASIS_SIZE);

  for(auto& data : basis) {
    double  tmp = gaml::random::uniform(0,10);
    data.first  = tmp;
    data.second = (int)tmp;
  }

  return basis;
}

int main(int argc, char* argv[]) {
  Basis basis = generate_data();

  try {
    // Let us set up a classifier and its confusion matrix.
    Classifier                         classifier;
    gaml::classification::Confusion<U> matrix;
    matrix.clear();
    matrix.update(classifier,
		  basis.begin(), basis.end(),
		  input_of_data, output_of_data, class_of_label);
  
    // Now, let us display the confusion matrix.
    matrix.display(std::cout);

    // Let us access to some data in the matrix.
    std::cout << "Probability of saying 7 while class is actually 5 : " << matrix.confusion(5,7)         << std::endl
	      << "Probability of saying 7                           : " << matrix.predictionFrequency(7) << std::endl
	      << "Probability of having class 5                     : " << matrix.truthFrequency(5)      << std::endl
	      << std::endl;
  
    // Let us list the classes that are actually used in the confusion
    // matrix (some classes may have never appeared in the examples nore
    // in the predictions).
    auto actual_classes = matrix.classes();
    std::cout << "Those classes are actually used :";
    for(auto& used_class : actual_classes) std::cout << ' ' << used_class;
    std::cout << std::endl << std::endl;

    // Let us save, reload and re-display the matrix.
    std::ofstream ofile;
    ofile.open("Confusion.data");
    ofile << matrix;
    ofile.close();

    std::ifstream ifile;
    gaml::classification::Confusion<U> loaded;
    ifile.open("Confusion.data");
    ifile >> loaded;
    ifile.close();
    loaded.display(std::cout);
  }
  catch(const std::exception& e) {
    std::cout << e.what() << std::endl;
  }

  return 0;
}

