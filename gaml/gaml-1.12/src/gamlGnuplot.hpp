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


#include <fstream>
#include <string>
#include <gamlException.hpp>

namespace gaml {
  namespace gnuplot {
    enum Terminal {
      termFig,
      termX11
    };

    enum Style {
      styleLine,
      styleDots
    };

    enum Verbosity {
      verboseOn,
      verboseOff
    };

    /**
     * This plots a ROC curve.
     */
    template<int GNUPLOT_TERMINAL, int GNUPLOT_STYLE, int GNUPLOT_VERBOSITY>
    void ROC(std::string plot_file_radix,
	     std::string data_file,
	     std::string title) {
      std::string plot_name(plot_file_radix+".plot");
      std::ofstream ofs;

      ofs.open(plot_name.c_str());
      if(!ofs)
	throw gaml::exception::File(plot_name,"in gaml::gnuplot::ROC");

      switch(GNUPLOT_TERMINAL) {
      case termFig:
	ofs << "set terminal fig color dashed" << std::endl
	    << "set output \"" << plot_file_radix << ".fig\"" << std::endl;
	break;
      case termX11:
	ofs << "set terminal x11 dashed" << std::endl;
	break;
      default:
	throw gaml::exception::Gnuplot("in gaml::gnuplot::ROC : Bad terminal in template arguments");
      }

      ofs << "set xrange[0:1]" << std::endl
	  << "set yrange[0:1]" << std::endl
	  << "set xlabel \"False Positive rate (fall out)\"" << std::endl
	  << "set ylabel \"True Positive rate (sensitivity)\"" << std::endl
	  << "set title \"" << title << "\"" << std::endl
	  << "plot x with lines lt 4 lc 0 notitle, \\" << std::endl
	  << "  '" << data_file << "' with lines lt 1 lc rgb \"blue\" lw 3 notitle" << std::endl;

      ofs.close();
      switch(GNUPLOT_VERBOSITY) {
      case verboseOff:
	break;
      case verboseOn:
	std::cout << "\"" << plot_name << "\" generated."<< std::endl;
	break;
      default:
	throw gaml::exception::Gnuplot("in gaml::gnuplot::ROC : Bad verbosity in template arguments");
      }
    }
  }
}
