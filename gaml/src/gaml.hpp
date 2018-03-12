#pragma once

/*
 *   Copyright (C) 2012,  Supelec
 *
 *   Author : Hervé Frezza-Buet, Frédéric Pennerath 
 *
 *   Contributor :
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License (GPL) as published by the Free Software Foundation; either
 *   version 3 of the License, or any later version.
 *   
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   General Public License for more details.
 *   
 *   You should have received a copy of the GNU General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *   Contact : herve.frezza-buet@supelec.fr, frederic.pennerath@supelec.fr
 *
 */

#include <gamlAlgorithms.hpp>
#include <gamlBag.hpp>
#include <gamlBootstrap.hpp>
#include <gamlCache.hpp>
#include <gamlConfusion.hpp>
#include <gamlDimensions.hpp>
#include <gamlException.hpp>
#include <gamlFilter.hpp>
#include <gamlFilters.hpp>
#include <gamlGnuplot.hpp>
#include <gamlLoss.hpp>
#include <gamlMap.hpp>
#include <gamlMerge.hpp>
#include <gamlMultiClass.hpp>
#include <gamlMultiDim.hpp>
#include <gamlPartition.hpp>
#include <gamlScore.hpp>
#include <gamlSearch.hpp>
#include <gamlSpan.hpp>
#include <gamlSplit.hpp>
#include <gamlVariableSelection.hpp>
#include <gamlShuffle.hpp>
#include <gamlJSONParser.hpp>
#include <gamlStreamer.hpp>
#include <gamlTabular.hpp>
#include <gamlIndexedDataset.hpp>
#include <gamlWrapper.hpp>

/**
 * @example example-000-000-overview.cpp
 * @example example-001-001-JSON-parser.cpp
 * @example example-001-001-handcrafted-parser.cpp
 * @example example-001-002-indexed-dataset.cpp
 * @example example-001-003-set-manipulations.cpp
 * @example example-001-004-span.cpp
 * @example example-001-005-tree.cpp
 * @example example-001-006-score.cpp
 * @example example-001-007-score.cpp
 * @example example-002-001-confusion.cpp
 * @example example-002-002-roc.cpp
 * @example example-002-003-cross-validation.cpp
 * @example example-002-004-bootstrapping.cpp
 * @example example-002-005-bagging.cpp
 * @example example-002-006-scorer.cpp
 * @example example-003-001-multidimension.cpp
 * @example example-003-002-multiclass.cpp
 * @example example-004-001-variable-projection.cpp
 * @example example-004-002-filter-based-selection.cpp
 * @example example-004-003-wrapper-based-selection.cpp
 * @example example-005-students.cpp
 * @example example-customer.hpp
 * @example example-dummy.hpp
 * @example example-ratio.hpp
 * @example example-scorer.hpp
 * @example example-silly.hpp
 */




/**
 * @mainpage
 *
 * @section Overview
 * 
 * The gaml library has been supported by the <a
 * href="http://malis.metz.supelec.fr/spip.php?rubrique107">Methodeo
 * project</a>. It consists of a C++ library, based on generic
 * programming techniques, which offers tools for the use of machine
 * learning: real risk estimator, manimulation of data, variable
 * selection, etc... The library iself does not provide regression or
 * classification algorithms, but rather allows the user to wrap
 * around its favorite algorithms some general purpose machine
 * learning features. Nevertheless, the famous <a
 * href="http://www.csie.ntu.edu.tw/~cjlin/libsvm/">libsvm package</a>
 * by Chih-Chung Chang and Chih-Jen Lin has already been included in gaml thanks
 * to the <a
 * href="http://malis.metz.supelec.fr/spip.php?article192">gaml-libsvm</a>
 * extension.
 *
 * Last, let us insist on one major feature of the gaml lib. It relies
 * on c++ generic programming, which is strongly typed. The design of
 * the library fits the mathematics of machine learning concepts, and
 * thus the strong typing forces the user to comply to those
 * concepts. This is deliberate. The drawback is clearly that the
 * syntax error fixing can be a hard job, since a small error in
 * typing can generate quite a lot of error messages. In spite of
 * this, the benefit is that all the programming effort is
 * concentrated on that point. Indeed, when syntaxically correct, the
 * code leads to a safe and efficient execution . Very few time is 
 * spent at debugging run time memory errors then.
 *
 * @section The use of concepts in generic programming
 *
 * For those who are not familiar with generic programming, the use of
 * concept may be confusing since classical object oriented relies
 * rather on inheritence mechanisms. A concept is a syntactical
 * requirement. In the gaml library, such requirement are documented
 * through the use of <b>fake</b> classes in the gaml::concept
 * namespace. Let us take the exemple of the gaml::concept::Predictor
 * concept.
 *
 * The gaml::concept::Predictor concept says that some predictor must define two types, names input_type and output_type, and that it should provide some defaut and copy constructors, as well as a operator() method. Let us propose some predictor (dummy...).
 * @code
class Funny {
public:
  typedef char         input_type
  typedef std::string  output_type

  Funny(void) {}
  Funny (const Funny& other) {}
  Funny& operator=(const Funny& other) {}

  output_type operator()(const input_type& x) const {
    return std::string(10,x); // "xxxxxxxxxx"
  }
};
 * @endcode
 *
 * This Funny class fits the gaml::concept::Predictor concept while no inheritance is involved. If some algorithm in the documentation is such as it requires an argument whose type fits the gaml::concept::Predictor concept, this will be specified in the documentation. For example, let us suppose that the function foo is dedicated to the manipulation of some predictor. Its declaration in the gaml lib would be
 * @code
 namespace gaml {
   template<typename Predictor>
   double foo(const Predictor& pred) {....}
 }
 * @endcode
 *
 * The use of the function in some code where Funny is available would be
 * @code
 Funny funny;
 double result = gaml::foo<Funny>(funny);
 * @endcode
 *
 * This is will compile fine as long as the Funny class fits the gaml::concept::Predictor concept. Moreover, when the compiler can guess the template parameter type from the function call, the template parameters can be removed. This leads to the following codes, that gives you the flavor of the gaml function calls.
 * @code
 Funny funny;
 double result = gaml::foo(funny);
 * @endcode
 *
 *
 *
 * @section The use of iterators and functors
 *
 * This idea of the library is that data belong to collections that
 * can be accessed by iterators. Most algorithms provided in the gaml
 * library take iterators as argument when they have to consider
 * a collection of data. This is complient with the STL programming
 * style. The user is thus responsible for the way s/he stores the
 * data. Consequently s/he has to provide functions that allows to retrieve
 * elements in each single datum in the data set. Typically, data sets
 * contain input/output pairs. The gaml algorithm will be provided with
 * iterators on the dataset and it will acces to successive
 * elements. From each element, the gaml algorithm will have to extract
 * the input and the output contained in the pair. In order not to
 * impose the coding of those pairs to the user, gaml algorithms will
 * have to be given two supplementary extraction functions. Let us
 * write some typical gaml code accordingly.
 *
 * @code
typedef char                    Input;
typedef std::string             Output;
typedef std::pair<Input,Output> Data;
typedef std::vector<Data>       Samples;

const Input&  input_of (const Data& data) {return data.first;}
const Output& output_of(const Data& data) {return data.second;}

int main(...) {
  Samples basis;

  // Let us fill the basis.
  basis.resize(100);
  for(Samples::iterator iter = basis.begin(); iter != basis.end(); ++iter) {
    Data& data  =  *iter;
    data.first  = // init some input here
    data.second = // init some output here
  }

  // Let us set up a shuffled basis.
  gaml::Shuffle<Samples::iterator,nasty-functional-types> shuffled = gaml::shuffle(basis.begin(),
									       basis.end());

  // Let us compute something
  Funny funny;
  risk = gaml::some_algo(funny,
                         shuffled.begin(), shuffled.end(), // We iterate on the shuffled basis.
		         input_of,     // These are the
		         output_of);   // extraction functions.
}
 * @endcode
 *
 * The previous code benefits from the template parameter implicite
 * resolution, since gaml::some_algo is a template function, whose type
 * parameters can be ommitted, as mentioned for gaml::foo previously. It
 * can be simplified further. First, C++11 provide smarts notation for
 * interation on collections (a new for loop syntax). Second, the auto
 * keyword can be used where a type name is required, when the type can
 * be guessed by the compiler. This is the case for the
 * gaml::Shuffle<Samples::iterator,nasty-functional-types> obscure type
 * provided by gaml. This leads to rewrite the code as this.
 *
 * @code
typedef char                    Input;
typedef std::string             Output;
typedef std::pair<Input,Output> Data;
typedef std::vector<Data>       Samples;

const Input&  input_of (const Data& data) {return data.first;}
const Output& output_of(const Data& data) {return data.second;}

int main(...) {
  Samples basis;

  // Let us fill the basis.
  basis.resize(100);
  for(auto& data : basis){
    data.first  = // init some input here
    data.second = // init some output here
  }

  // Let us set up a shuffled basis.
  auto shuffled = gaml::shuffle(basis.begin(),basis.end());

  // Let us compute something
  Funny funny;
  risk = gaml::some_algo(funny,
                         shuffled.begin(), shuffled.end(), 
		         input_of,
		         output_of);
}
 * @endcode
 *
 * Moreover C++11 provides a syntax for the definition of functions on the fly in the code (lambda functions). This can be done for input_of and output_of. This leads to rewrite the code as this.
 *
 * @code
typedef char                    Input;
typedef std::string             Output;
typedef std::pair<Input,Output> Data;
typedef std::vector<Data>       Samples;

int main(...) {
  Samples basis;

  // Let us fill the basis.
  basis.resize(100);
  for(auto& data : basis){
    data.first  = // init some input here
    data.second = // init some output here
  }

  // Let us set up a shuffled basis.
  auto shuffled = gaml::shuffle(basis.begin(),basis.end());

  // Let us compute something
  Funny funny;
  risk = gaml::some_algo(funny,
                         shuffled.begin(), shuffled.end(), 
		         [](const Data& data) -> const Input&  {return data.first;},
		         [](const Data& data) -> const Output& {return data.second;});
}
 * @endcode
 *
 *
 * @section Read the documentation
 *
 * The user manual of the gaml library consists of a set of examples,
 * available in this documentation. They are ordered, and they should
 * be read in that order to get a comprehensive overview of the gaml
 * features.
 * 
 * @section Conclusion
 *
 * The use of gaml implies invoking templates that can by
 * intricated. Thanks to C++11 syntactical elements (as auto), this
 * intrication can be hidden to the user so that the code is kept
 * readible. The code expresses naturally the machine learning
 * methodological concepts, and type checking ensures that they are
 * not misused. Once compiled, as all the type checking effort is made
 * at compiling time, the executable is safe and efficient.
 *
 */



