#include <gaml.hpp>
#include <string>
#include <iostream>
#include <istream>
#include <utility>
#include <vector>
#include <array>
#include <random>
#include <algorithm>
#include <cstdio>

// Let us consider students applying to some university. The
// university gives a mark to each student, from the school report of
// high school last year.

enum Discipline {
  Art,
  Biology,
  Chemistry,
  ForeignLanguage,
  Geography,
  History,
  Literature,
  Mathematics,
  Music,
  Physics,
  Sports
};
#define NB_DISCPIPLINES 11

std::string toString(int d) {
  switch((Discipline)d) {
  case Art:             return "Art";
  case Biology:         return "Biology";
  case Chemistry:       return "Chemistry";
  case ForeignLanguage: return "Foreign Language";
  case Geography:       return "Geography";
  case History:         return "History";
  case Literature:      return "Literature";
  case Mathematics:     return "Mathematics";
  case Music:           return "Music";
  case Physics:         return "Physics";
  case Sports:          return "Sports";
  default:              return "???";
  }
}

// Marks are given in [0,1]
typedef double                            Mark;
typedef std::array<Mark, NB_DISCPIPLINES> Report;

std::ostream& operator<<(std::ostream& os, const Report& r) {
  os << "["; 
  os << toString(0) << " = " << r[0];
  for(unsigned int i = 1; i < NB_DISCPIPLINES; ++i)
    os << ", " << toString(i) << " = " << r[i];
  os << "]";
  return os;
}

// The university gives a mark according to the average of some of the
// school report mark. If one mark is below the pass mark, the global
// score is 0 (elimination). The noise models an extra modulation by a
// panel of university professors.
#define PANEL_NOISE .1
#define PASS_MARK   .3

template<typename RANDOM_DEVICE>
Mark university_mark(const Report& report, RANDOM_DEVICE& rd) {
  Mark mark;

  if(   report[Discipline::Biology    ] < PASS_MARK
	|| report[Discipline::Chemistry  ] < PASS_MARK
	|| report[Discipline::Mathematics] < PASS_MARK
	|| report[Discipline::Physics    ] < PASS_MARK)
    mark = 0;
  else {
    mark =   report[Discipline::Biology    ]
      + report[Discipline::Chemistry  ]
      + report[Discipline::Mathematics]
      + report[Discipline::Physics    ];
    mark /= 4.0;
    mark += std::uniform_real_distribution<double>(-PANEL_NOISE, PANEL_NOISE)(rd);
  }
  return std::max(std::min(mark, 1.), 0.);
}

// Let us define supervised learning vocabulary.

typedef Report                  Sample;
typedef Mark                    Label;
typedef std::pair<Sample,Label> Data;

const Report& report_of(const Data& d) {return d.first;}
Mark          mark_of  (const Data& d) {return d.second;}

// Let us define a predictor, for a generic input for which attribute
// values can be iterated. The predictor implements an average if all
// marks are above the pass mark. It returns 0 (elimination)
// otherwise.
template<typename INPUT> class Predictor {
private:
  double pass_mark;

public:

  typedef INPUT input_type;
  typedef Mark  output_type;

  Predictor(double pm) : pass_mark(pm) {}

  output_type operator()(const input_type& input) const {
    double mark = 0;
    for(auto m : input) 
      if(m < pass_mark) return 0;
      else              mark += m;
    return mark/(Mark)input.size();
  }
};


// This is a learner, adapted to a particular variable projection (the
// INPUT parameter).
template<typename INPUT> class Learner {
public:
  typedef Predictor<INPUT> predictor_type;

  template<typename DataIterator, typename InputOf, typename OutputOf>
  predictor_type operator()(const DataIterator& begin, const DataIterator& end,
			    const InputOf& inputOf, const OutputOf& outputOf) const {
    double min_pass_mark = 0;
    double max_pass_mark = 1;

    for(auto student = begin; student != end; ++student) {
      auto score = outputOf(*student);
      auto& marks = inputOf(*student);
      auto min = std::min_element(marks.begin(),marks.end());
      if(score == 0) min_pass_mark = std::max(*min,min_pass_mark);
      else           max_pass_mark = std::min(*min,max_pass_mark);
    }
    
    return predictor_type((min_pass_mark+max_pass_mark)*.5);
  }
};

// This is the learner used in the variable selection process.
struct GenericLearner {
  template<typename Input>
  Learner<Input> make() const {return Learner<Input>();}
};

// This learner uses internally a variable selection, and produces a
// predictor. This predictor performs the computed projection and then
// the decision.

class MetaLearner {
  typedef std::vector<int> indices_type;
  typedef gaml::wrapper_traits<Sample, Label, GenericLearner> traits;

public:
  typedef typename traits::wrapping_predictor_type predictor_type;
  bool verbosity;

  MetaLearner() : verbosity(false) {}
  MetaLearner(const MetaLearner& other) : verbosity(other.verbosity) {}

  template<typename DataIterator, typename InputOf, typename OutputOf>
  predictor_type operator()(const DataIterator& begin, const DataIterator& end,
			    const InputOf& input_of, const OutputOf& label_of) const {

    // Let us apply a variable selection process.
    GenericLearner generic_learner;

    // We need a real risk estimator for evaluating the algorithms when variables are selected.
    auto real_risk_estimator = gaml::risk::cross_validation(gaml::loss::Quadratic<double>(),
							    gaml::partition::kfold(10), false);
    // Wrapping evaluator
    auto evaluator = gaml::varsel::make_wrapper_evaluator(generic_learner, real_risk_estimator,
							  begin, end, input_of, label_of);
    // Make the evaluator verbose.
    if(verbosity) evaluator.verbose();
    
    // The result of a search is a subset of variables with its risk.
    std::vector<int> variable_subset;
    double           best_risk = gaml::varsel::SFFS(evaluator, variable_subset, verbosity);

    if(verbosity) {
      std::cout << std::endl
		<< "Risk is : " << best_risk << ", with the following variable selection:" << std::endl
		<< "  {";
      for(auto attr : variable_subset) std::cout << ' ' << toString(attr);
      std::cout << " }" << std::endl << std::endl;
    }

    // Let us set up the predictor from the projection of the data.
    auto projection = gaml::project(begin, end, 
				    variable_subset.begin(), variable_subset.end(), 
				    input_of, label_of);
    return projection.teach(generic_learner); // The predictor is computed here.
  }
};

#define CACHE_PAGE_SIZE  200
#define CACHE_NB_PAGES    10

#define DATA_FILE  "reports.dat"
#define INDEX_FILE "reports.idx"

#define NB_DATA_IN_DATAFILE 10000

int main(int argc, char* argv[]) {
  if(argc != 2) {
    std::cout << "Usage : " << std::endl
	      << "  " << argv[0] << " generate  <-- makes the data file." << std::endl
	      << "  " << argv[0] << " run       <-- process the file. " << std::endl;
    return 0;
  }
  
  // random seed initialization
  std::random_device rd;
  std::mt19937 gen(rd());

  bool generate_mode = std::string(argv[1]) == "generate";

  // Let us build data parsers.
  auto parser = gaml::make_JSON_parser<Data>();

  if(generate_mode) {
    std::normal_distribution<> d(.6,.15);
 
    std::ofstream ofile(DATA_FILE);
    auto ostr = gaml::make_output_data_stream(ofile, parser);
    auto out  = gaml::make_output_iterator(ostr);
    std::cout << "Generating students in \"" <<  DATA_FILE << "\"..." << std::flush;
    for(unsigned int i=0; i< NB_DATA_IN_DATAFILE; ++i) {
      Report r;
      for(auto& mark : r) mark = std::max(std::min(d(gen),1.),0.);
      *(out++) = Data(r,university_mark(r, gen));
    }
    std::cout << " Done." << std::endl;
    std::cout << "Removing old index file." << std::endl;
    std::remove(INDEX_FILE);
    return 0;
  }

  // Here, we do not know how the university mark is given, and we
  // will try to predict it from a student report.

  // Let us use an indexed file as a learning dataset.
  std::cout << "Binding a dataset to file \"" << DATA_FILE << "\", using \"" 
	    << INDEX_FILE << "\" for indices..." << std::flush;
  auto dataset = gaml::make_indexed_dataset(parser, DATA_FILE, INDEX_FILE);
  std::cout << " Done." << std::endl;

  // In order to reduce the file accesses, let us implement a cache.
  auto cached_dataset = gaml::cache(dataset.begin(),dataset.end(), CACHE_PAGE_SIZE, CACHE_NB_PAGES); 

  // Let us use learn from the whole dataset.
  MetaLearner learner;
  learner.verbosity = true;
  auto pred = learner(cached_dataset.begin(), cached_dataset.end(), report_of, mark_of);
  Report test_report;
  std::uniform_real_distribution<double> uniform(0,1);
  for(auto& mark : test_report) mark = uniform(gen);
  std::cout << std::endl << "Report " << test_report << " will be scored " << pred(test_report) << std::endl << std::endl;

  // Let us cross-validate the variable selection algorithm
  // itself. Note that intricated cross-validations are
  // involved. Indeed, the wrapper evaluation uses a
  // cross-validation...
  
  auto evaluator = gaml::risk::cross_validation(gaml::loss::Quadratic<double>(),
						gaml::partition::kfold(10), true);
  learner.verbosity = false;

  double risk = evaluator(learner, cached_dataset.begin(), cached_dataset.end(), report_of, mark_of);
  std::cout << std::endl
   	    << "The estimated real risk of the meta-learning process is " << risk << std::endl
   	    << std::endl;
  return 0;
}
