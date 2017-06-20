// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
/*
 * Ceph - scalable distributed file system
 *
 * Copyright (C) 2016 Michael Sevilla <mikesevilla3@gmail.com>
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software
 * Foundation.  See file COPYING.
 *
 */

#include <lua.hpp>
#include <list>
#include <map>
// ADDED
#include <string>
#include <vector>

using namespace std;
//
//#include "include/types.h"
//#include "common/Clock.h"
//#include "CInode.h"
//
typedef int32_t rank_t;
typedef int32_t load_t;

class Mantle {
  protected:
    lua_State *L;
    map<rank_t, load_t>  mds_load;

  public:
    Mantle() : L(NULL) { };
    int start();
    int execute(const string &script);
    int balance(const string &script,
		rank_t whoami, 
                const vector < map<string, double> > &metrics,
                map<rank_t,double> &my_targets);
};
//
//#endif
/*
 * rank_t -> mds_rank_t
 * rank_t -> mpirank
 *
 */
