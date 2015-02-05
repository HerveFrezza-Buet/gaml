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
#include <string>
#include <sstream>
#include <exception>

namespace gaml {
  namespace exception {
    class Any : public std::exception {
    private:
      std::string message;
    public:
      Any(const std::string& kind,
	  const std::string& msg) {
	message = std::string("ML exception : ")
	  + kind + " : " + msg;
      }

      virtual ~Any(void) throw () {}

      virtual const char * what(void) const throw ()
      {
        return message.c_str();
      }
    };
    
    class File : public Any {
    public:
      File(const std::string& filename,const std::string& msg) 
	: Any(std::string("Cannot open \"")+filename+std::string("\""),msg) {}
    };
    
    class Bootstrap : public Any {
    public:
      Bootstrap(std::string msg) 
	: Any("Cannot bootstrap",msg) {}
    };

    class Gnuplot : public Any {
    public:
      Gnuplot(const std::string& msg) 
	: Any("Gnuplot generation",msg) {}
    };

    class ConfusionMatrix  : public Any {
    public:
      ConfusionMatrix(const std::string& msg) 
	: Any("Confusion matrix",msg) {}
      ConfusionMatrix(const std::string& kind, 
		      const std::string& msg) 
	: Any("Confusion matrix",kind+std::string(" : ")+msg) {}
    };

    class EmptyConfusionMatrix : public ConfusionMatrix {
    public:
      EmptyConfusionMatrix(const std::string& msg) 
	: ConfusionMatrix("Empty matrix",msg) {}
    };

    class NoPositiveInData : public ConfusionMatrix {
    public:
      NoPositiveInData(const std::string& msg) 
	: ConfusionMatrix("No positive samples in data",msg) {}
    };

    class NoNegativeInData : public ConfusionMatrix {
    public:
      NoNegativeInData(const std::string& msg) 
	: ConfusionMatrix("No negative samples in data",msg) {}
    };


    class NoPositivePrediction : public ConfusionMatrix {
    public:
      NoPositivePrediction(const std::string& msg) 
	: ConfusionMatrix("No positive prediction from data",msg) {}
    };

    class NoNegativePrediction : public ConfusionMatrix {
    public:
      NoNegativePrediction(const std::string& msg) 
	: ConfusionMatrix("No negative prediction from data",msg) {}
    };

    
  }
}
