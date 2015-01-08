#pragma once

/*
 *   Copyright (C) 2014,  Supelec
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


#include <vq2.h>
#include <gaml.hpp>

/**
 * @example example-001-001-kmeans.cpp
 * @example example-001-002-som.cpp
 *
 * @example disk.hpp
 */


/**
 * @mainpage
 *
 * KVQ is not really a new library. Performing vector quantization in
 * the feature space requires to have generic vector quantization
 * algorithms in the one hand and a way to handle the feature space
 * not only with dot products, in the other hand. This is available by
 * the vq2 and the gaml libraries (more precisely the gaml::span::* types and functions) respectively.
 *
 * The present package only provide some examples that brings things together.
 */
