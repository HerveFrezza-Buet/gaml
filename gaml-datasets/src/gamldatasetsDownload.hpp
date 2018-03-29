#pragma once

/*
 *   Copyright (C) 2018,  CentraleSupelec
 *
 *   Author : Jeremy Fix
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
 *   Contact : jeremy.fix@centralesupelec.fr
 *
 */

#include <string>
#include <curl/curl.h>
#include <cstdio>

#include <experimental/filesystem>

namespace gaml {
  namespace datasets {

    inline std::string download(std::string url) {
      
      auto write_data = [] (void *ptr, size_t size, size_t nmemb, FILE *stream) -> size_t {
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
      };
      
      CURL *curl;
      FILE *fp;
      CURLcode res;

      std::string outfilename(std::tmpnam(nullptr));
      while(std::experimental::filesystem::exists(outfilename))
	outfilename = std::tmpnam(nullptr);
     
      
      curl = curl_easy_init();
      if (curl) {
	std::cout << "Downloading " << url << std::endl;
        fp = fopen(outfilename.c_str(),"wb");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	/* enable progress meter */
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        res = curl_easy_perform(curl);
	
        /* always cleanup */
        curl_easy_cleanup(curl);
        fclose(fp);

	if(res != CURLE_OK)
	  throw std::runtime_error("Failed to download " + url);
      }
      else 
	throw std::runtime_error("Failed to initialize curl");

      return outfilename;
    }
    
  }
}
