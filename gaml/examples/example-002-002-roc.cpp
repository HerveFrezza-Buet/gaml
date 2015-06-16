#include <gaml.hpp>
#include <array>
#include <utility>
#include <fstream>
#include <iostream>

// Let us determine here wether a student is good not,
// according to the result of their math test (in [0..100]).

#define NB_GOOD_STUDENTS      600
#define NB_WEAK_STUDENTS      400
#define BASIS_SIZE            (NB_GOOD_STUDENTS + NB_WEAK_STUDENTS)
#define MIN_WEAK_MARK         0
#define MAX_WEAK_MARK         60
#define MIN_GOOD_MARK         40
#define MAX_GOOD_MARK         100

typedef double                           Mark;
typedef enum {studentGood,studentWeak}   Skill;
typedef std::pair<Mark,Skill>            Data;  
typedef std::array<Data,BASIS_SIZE>      Basis;

Mark   input_of_data(const Data& data) {return data.first;}
Skill output_of_data(const Data& data) {return data.second;}
Skill class_of_label(Skill label)      {return label;} 

// Let us define a classifier based on a straightforward
// threshold decision.
class Classifier {
public:
  typedef Mark input_type;
  typedef Skill output_type;

  double threshold;

  output_type operator() (const input_type &x) const {
    if(x >= threshold) return studentGood;
    else               return studentWeak;
  }
};

Data make_good_student(void) {
  return Data(gaml::random::uniform(MIN_GOOD_MARK,MAX_GOOD_MARK),
	      studentGood);
}

Data make_weak_student(void) {
  return Data(gaml::random::uniform(MIN_WEAK_MARK,MAX_WEAK_MARK),
	      studentWeak);
}

int main(int argc, char* argv[]) {

  Basis                                  basis;
  gaml::classification::Confusion<Skill> matrix;
  std::ofstream                          data_file;
  unsigned int                           i;
  Classifier                             classifier;

  try {

    for(i = 0; i < NB_GOOD_STUDENTS; ++i) basis[i] = make_good_student();
    for(     ; i < BASIS_SIZE;       ++i) basis[i] = make_weak_student();

    // Let us generate a ROC curve.

    data_file.open("roc.data");
    for(classifier.threshold = 0; 
	classifier.threshold <= 100; 
	++classifier.threshold) {
      matrix.clear();
      matrix.update(classifier,
		    basis.begin(), basis.end(),
		    input_of_data, output_of_data, class_of_label);
      data_file << matrix.fallOut    (studentGood, studentWeak) << ' ' 
		<< matrix.sensitivity(studentGood, studentWeak) << std::endl;
    }
    data_file.close();

    // Let us draw it in some gnuplot files

    gaml::gnuplot::ROC<gaml::gnuplot::termX11,
		       gaml::gnuplot::styleLine,
		       gaml::gnuplot::verboseOn>("roc-curve",     "roc.data", "ml example 001-002"); 
    gaml::gnuplot::ROC<gaml::gnuplot::termFig,
		       gaml::gnuplot::styleLine,
		       gaml::gnuplot::verboseOn>("roc-curve-fig", "roc.data", "ml example 001-002");
  }
  catch(const std::exception& e) {
    std::cout << e.what() << std::endl;
  }

  return 0;
}
