#pragma once

/*
 *   Copyright (C) 2012,  Supelec
 *
 *   Author : Hervé Frezza-Buet
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
 *   Contact : herve.frezza-buet@supelec.fr
 *
 */

#include <gaml.hpp>
#include <libsvm/svm.h>
#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <sstream>
#include <iterator>

namespace gaml {

  namespace libsvm {

    namespace exception {
      class Parameters : public gaml::exception::Any {
      public:
	Parameters(std::string msg) 
	  : Any("Bad SVM parameters",msg) {}
      };

      class BadFile : public gaml::exception::Any {
      public:
	BadFile(std::string msg) 
	  : Any("Bad file",std::string("Some problem occured with file \"")+msg+"\"") {}
      };

      class BadPredictor : public gaml::exception::Any {
      private:
	std::string badfunction(bool model,
				bool nb,
				double n,
				double fd) {
	  std::ostringstream ostr;
	  ostr << "The following is null :";
	  if(model) ostr << " model";
	  if(nb)    ostr << " nb_nodes_of";
	  if(n)     ostr << " nodes_of";
	  if(fd)    ostr << " from_double";
	  ostr << '.';
	  return ostr.str();
	}
	
      public:
	BadPredictor(bool model,
		     bool nb,
		     double n,
		     double fd) 
	  : Any("Bad predictor",badfunction(model,nb,n,fd)) {}
      };
    }

    namespace internal {
      static void free_model(struct svm_model* m) {
	if(m != nullptr) {
	  svm_free_and_destroy_model(&m);
	}
      }

      static void free_problem(struct svm_problem* p) {
	if(p != nullptr) {
	  delete p->x;
	  delete p->y;
	  delete p;
	}
      }

      template<typename Output>
      struct FromDouble {
	Output operator()(double v) {return (Output)v;}
      };

      template<typename Output>
      struct ToDouble {
	double operator()(Output o) {return (double)o;}
      };

      template<typename Output>
      struct DummyToDouble {
	double operator()(Output o) {return 0.0;}
      };

      template<>
      struct FromDouble<bool> {
	bool operator()(double v) {return v != 0;}
      };
      template<>
      struct ToDouble<bool> {
	double operator()(bool o) {if(o) return 1; return 0;}
      };
    }

    /**
     * This fills a svn_node array representing an
     * input. The input is provided as a range of iterator. Free it
     * with delete []. coefOf cast the iterator content into a double.
     */
    template<typename InputIterator, typename CoefOf>
    void input_of(struct svm_node* res,
		  const InputIterator& begin,
		  const InputIterator& end,
		  const CoefOf& coef_of) {
      struct svm_node* niter;
      InputIterator iter = begin;
      int i;

      for(niter = res, i=1;
      	  iter != end;
      	  ++iter,++niter) {
      	niter->index = i;
      	niter->value = coef_of(*iter);
      }
      niter->index = -1;
    }

    template<typename Input, typename Output> class Learner;


    /**
     * This is the predictor function. It handles internally a svm
     * model. The svm model is set by some svm_train, and then
     * handeled as a smart-pointer when the predictor is copied.
     */
    template<typename Input, typename Output>
    class Predictor {
    private:
      
      std::shared_ptr<struct svm_model>    model;
      std::shared_ptr<struct svm_problem>  problem;
      mutable std::vector<struct svm_node> nodes;


      template<typename NbNodeOf, typename NodesOf, typename FromDouble>
      Predictor(std::shared_ptr<struct svm_model>   the_model,
		std::shared_ptr<struct svm_problem> the_problem,
		const NbNodeOf& nb_nodes_of_func,
		const NodesOf&  nodes_of_func,
		const FromDouble& from_double_func) 
	: model(the_model), problem(the_problem),
	  nb_nodes_of(nb_nodes_of_func),
	  nodes_of(nodes_of_func),
	  from_double(from_double_func) {}

      friend class gaml::libsvm::Learner<Input, Output>;

    public:

      typedef Predictor<Input,Output> self_type;

      typedef Input input_type;
      typedef Output output_type;

      
      /**
       * This function is called internally. 
       * @returns The number of svm_node(s) required for storing x 
       */
      std::function<int (const Input&)> nb_nodes_of;

      /**
       * This function is called internally. 
       * @param nodes an array of nodes having (at least) the size returned by nb_nodes_of.
       */
      std::function<void (const Input&,struct svm_node*)> nodes_of;

      /**
       * This function is called internally. It converts a double to the real output type.
       */
      std::function<Output (double)> from_double;
      
      Predictor(void) : nb_nodes_of(0), nodes_of(0), from_double(0) {}

      template<typename NbNodeOf, typename NodesOf, typename FromDouble>
      Predictor(const NbNodeOf& nb_nodes_of_func,
		const NodesOf&  nodes_of_func,
		const FromDouble& from_double_func) 
	: nb_nodes_of(nb_nodes_of_func),
	  nodes_of(nodes_of_func),
	  from_double(from_double_func) {}

      template<typename NbNodeOf, typename NodesOf>
      Predictor(const NbNodeOf& nb_nodes_of_func,
		const NodesOf&  nodes_of_func) 
	: nb_nodes_of(nb_nodes_of_func),
	  nodes_of(nodes_of_func),
	  from_double(internal::FromDouble<Output>()) {}

      ~Predictor(void) {
      }

      Predictor(const self_type& copy) 
      : model(copy.model),
	problem(copy.problem),
	nb_nodes_of(copy.nb_nodes_of),
	nodes_of(copy.nodes_of),
	from_double(copy.from_double) {
      }


      self_type& operator=(const self_type& copy) {
	if(this != &copy) {
	  model       = copy.model;
	  problem     = copy.problem;
	  nb_nodes_of = copy.nb_nodes_of;
	  nodes_of    = copy.nodes_of;
	  from_double = copy.from_double;
	}

	return *this;
      }
      
      bool operator!(void) const {
	return model ==  0 || nb_nodes_of == nullptr || nodes_of == nullptr || from_double == nullptr;
      }
      
      /**
       * This call is not thread-safe.
       */
      output_type operator()(const input_type& x) const {
	if(!(*this)) {
	  throw gaml::libsvm::exception::BadPredictor(model ==  0,
						      nb_nodes_of == nullptr,
						      nodes_of == nullptr,
						      from_double == nullptr);
	}
	else {
	  nodes.resize(nb_nodes_of(x));
	  struct svm_node* xx = &(*(nodes.begin()));;
	  nodes_of(x,xx);
	  return from_double(predict(xx));
	}
      }

      /** This encapsulates svm_predict of libsvm. */
      double predict(const struct svm_node* x) const {return svm_predict(model.get(),x);}
      /** This encapsulates svm_get_svm_type of libsvm. */
      int get_svm_type(void) const {return svm_get_svm_type(model.get());}
      /** This encapsulates svm_get_nr_class of libsvm. */
      int get_nr_class(void) const {return svm_get_nr_class(model.get());}
      /** This encapsulates svm_get_labels of libsvm. */
      int get_labels(int* labels) const {return svm_get_labels(model.get(),labels);}
      /** This encapsulates svm_get_sv_indices of libsvm. */
      void get_sv_indices(int* sv_indices) const {svm_get_sv_indices(model.get(),sv_indices);}
      /** This encapsulates svm_get_nr_sv of libsvm. */
      int get_nr_sv(void) const {return svm_get_nr_sv(model.get());}
      /** This encapsulates svm_get_svr_probability of libsvm. */
      double get_svr_probability(void) const {return svm_get_svr_probability(model.get());}
      /** This encapsulates svm_predict_values of libsvm. */
      double predict_values(const struct svm_node* x, double* dec_values) const {return svm_predict_values(model.get(),x,dec_values);}
      /** This encapsulates svm_predict_probability of libsvm. */
      double predict_probability(const struct svm_node* x, double* prab_estimates) const {return svm_predict_probability(model.get(),x,prab_estimates);}
      /** This encapsulates svm_get_nr_class of libsvm. */
      int check_probability_model(void) const {return svm_check_probability_model(model.get());}
      /** This encapsulates svm_save_model of libsvm. */
      int save_model(const std::string& model_file_name) const {return svm_save_model(model_file_name.c_str(),model.get());}
      /** This encapsulates svm_save_model of libsvm. Be sure that
	  function pointers nodes_of and nb_nodes_of are set in order
	  to use the loaded predictor.*/
      void load_model(const std::string& model_file_name) {
	std::shared_ptr<struct svm_model> the_model(svm_load_model(model_file_name.c_str()),
						    internal::free_model);
	if(the_model != 0) {
	  model = the_model;
	  problem = 0;
	}
	else
	  throw gaml::libsvm::exception::BadFile(model_file_name);
      }

      const struct svm_model& get_model(void) const {return *model;}
    };

    
    template<typename Input, typename Output>
    class Learner {
    private:

      std::function<int (const Input&)>                   nb_nodes_of;
      std::function<void (const Input&,struct svm_node*)> nodes_of;
      std::function<Output (double)>                      from_double;
      std::function<double (Output)>                      to_double;
      const struct svm_parameter* param;
      
      
      void check(const struct svm_problem& problem,
		 const struct svm_parameter& param) const {
	const char* res = svm_check_parameter(&problem,&param);
	if(res != 0)
	  throw exception::Parameters(res);
      }
      
      struct svm_model* train(const struct svm_problem& problem,
			      const struct svm_parameter& param) const {
	return svm_train(&problem,&param);
      }

    public:

      typedef Predictor<Input,Output> predictor_type;
      
      Learner(void) : nb_nodes_of(), nodes_of(), from_double(), to_double(), param(0) {}

      Learner(const Learner<Input,Output>& cpy) 
	: nb_nodes_of(cpy.nb_nodes_of), 
	  nodes_of(cpy.nodes_of),
	  from_double(cpy.from_double),
	  param(cpy.param) {}
      
      template<typename NbNodeOf, typename NodesOf, typename FromDouble, typename ToDouble>
      Learner(const struct svm_parameter& parameters,
	      const NbNodeOf& nb_nodes_of_func,
	      const NodesOf&  nodes_of_func,
	      const FromDouble& from_double_func,
	      const ToDouble& to_double_func)
	: nb_nodes_of(nb_nodes_of_func),
	  nodes_of(nodes_of_func), 
	  from_double(from_double_func), 
	  to_double(to_double_func), 
	  param(&parameters) {}

      Learner<Input,Output>& operator=(const Learner<Input,Output>& cpy) {

	nb_nodes_of = cpy.nb_nodes_of; 
	nodes_of    = cpy.nodes_of;
	from_double = cpy.from_double;
	to_double = cpy.to_double;
	param       = cpy.param;
	return *this;
      }
      
      /** Supervized learning */
      template<typename DataIterator, typename InputOf, typename OutputOf> 
      Predictor<Input,Output> operator()(const DataIterator& begin, 
					 const DataIterator& end,  
					 const InputOf& input_of, 
					 const OutputOf& label_of) const {  

	// this is manages by a smart pointer at predictor level.
	struct svm_problem* the_problem = new struct svm_problem; 
	
	the_problem->l = (int)(std::distance(begin,end));
	the_problem->y = new double[the_problem->l];
	the_problem->x = new svm_node*[the_problem->l];

	svm_node**   xiter;
	double*      yiter;
	DataIterator diter = begin; // no default contructor is required.

	for(xiter = the_problem->x, yiter = the_problem->y;
	    diter != end;
	    ++diter, ++xiter, ++yiter) {
	  *yiter = label_of(*diter); 
	  *xiter = new struct svm_node[nb_nodes_of(input_of(*diter))];
	  nodes_of(input_of(*diter),*xiter);
	}
	check(*the_problem,*param); 
	std::shared_ptr<struct svm_problem> problem_ptr(the_problem,internal::free_problem);
	std::shared_ptr<struct svm_model>   model_ptr(train(*the_problem,*param),internal::free_model);
	return Predictor<Input,Output>(model_ptr,problem_ptr,nb_nodes_of,nodes_of,from_double);
      }
      
      /** Unsupervized learning */
      template<typename DataIterator, typename InputOf> 
      Predictor<Input,Output> operator()(const DataIterator& begin, 
					 const DataIterator& end,  
					 const InputOf& input_of) const {  

	// this is manages by a smart pointer at predictor level.
	struct svm_problem* the_problem = new struct svm_problem; 
	
	the_problem->l = (int)(std::distance(begin,end));
	the_problem->y = new double[the_problem->l];
	the_problem->x = new svm_node*[the_problem->l];

	svm_node**   xiter;
	DataIterator diter = begin; // no default contructor is required.

	for(xiter = the_problem->x;
	    diter != end;
	    ++diter, ++xiter) {
	  *xiter = new struct svm_node[nb_nodes_of(input_of(*diter))];
	  nodes_of(input_of(*diter),*xiter);
	}
	check(*the_problem,*param); 
	std::shared_ptr<struct svm_problem> problem_ptr(the_problem,internal::free_problem);
	std::shared_ptr<struct svm_model>   model_ptr(train(*the_problem,*param),internal::free_model);
	return Predictor<Input,Output>(model_ptr,problem_ptr,nb_nodes_of,nodes_of,from_double);
      }
    };

    namespace supervized {
      template<typename Input,typename NbNodeOf, typename NodesOf, typename FromDouble, typename ToDouble>
      auto learner(const struct svm_parameter& parameters,
		   const NbNodeOf& nb_nodes_of_func,
		   const NodesOf&  nodes_of_func,
		   const FromDouble& from_double_func,
		   const ToDouble& to_double_func) -> Learner<Input,decltype(from_double_func(0.0))> {
	return Learner<Input,decltype(from_double_func(0.0))>(parameters,nb_nodes_of_func,nodes_of_func,from_double_func,to_double_func);
      }
      
      template<typename Input, typename Output, typename NbNodeOf, typename NodesOf>
      auto learner(const struct svm_parameter& parameters,
		   const NbNodeOf& nb_nodes_of_func,
		   const NodesOf&  nodes_of_func) -> Learner<Input,Output> {
	return Learner<Input,Output>(parameters,
				     nb_nodes_of_func,nodes_of_func,
				     internal::FromDouble<Output>(),
				     internal::ToDouble<Output>());
      }
    }

    namespace unsupervized {
      template<typename Input,typename NbNodeOf, typename NodesOf, typename FromDouble>
      auto learner(const struct svm_parameter& parameters,
		   const NbNodeOf& nb_nodes_of_func,
		   const NodesOf&  nodes_of_func,
		   const FromDouble& from_double_func) -> Learner<Input,decltype(from_double_func(0.0))> {
	return Learner<Input,decltype(from_double_func(0.0))>(parameters,nb_nodes_of_func,nodes_of_func,from_double_func,
							      internal::DummyToDouble<decltype(from_double_func(0.0))>()); 
      }
      
      template<typename Input, typename Output, typename NbNodeOf, typename NodesOf>
      auto learner(const struct svm_parameter& parameters,
		   const NbNodeOf& nb_nodes_of_func,
		   const NodesOf&  nodes_of_func) -> Learner<Input,Output> {
	return Learner<Input,Output>(parameters,
				     nb_nodes_of_func,nodes_of_func,
				     internal::FromDouble<Output>(),
				     internal::DummyToDouble<Output>());
      }
    }


    /**
     * For internal use
     */
    inline void noprint(const char*) {}

    /**
     * This makes libsvm quiet.
     */
    inline void quiet(void) {
      svm_set_print_string_function(noprint);
    }
    
    /**
     * This performs a parameter init.
     */
    inline void init(struct svm_parameter& param) {
      param.svm_type = C_SVC;
      param.kernel_type = RBF;
      param.degree = 3;
      param.gamma = 0;	
      param.coef0 = 0;
      param.nu = 0.5;
      param.cache_size = 100;
      param.C = 1;
      param.eps = 1e-3;
      param.p = 0.1;
      param.shrinking = 1;
      param.probability = 0;
      param.nr_weight = 0;
      param.weight_label = NULL;
      param.weight = NULL;
    }
  }
}

/**
 * @example example-001-basics.cpp
 * @example example-002-unsupervized.cpp
 * @example example-003-3D.cpp
 * @example example-004-grid-search.cpp
 */


/**
 * @mainpage
 *
 *
 * The gaml-libsvm library has been supported by the <a
 * href="http://malis.metz.supelec.fr/spip.php?rubrique107">Methodeo
 * project</a>. It brings the famous <a
 * href="http://www.csie.ntu.edu.tw/~cjlin/libsvm/">libsvm package</a>
 * by Chih-Chung Chang and Chih-Jen Lin into the <a
 * href="http://malis.metz.supelec.fr/spip.php?rubrique109">gaml</a>
 * project.
 *
 *
 * You have to be familiar with gaml in order to use
 * gaml-libsvm. If so, just read the examples in the suggested order,
 * they cover all the features. You may also need to read the libsvm
 * documentation for the svm parameters.
 */
