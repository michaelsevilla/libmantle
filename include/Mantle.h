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

#ifndef CEPH_MANTLE_H
#define CEPH_MANTLE_H

#include <lua.hpp>
#include <list>
#include <map>
#include <string>
#include <vector>

using namespace std;

typedef int32_t rank_t;
typedef int32_t load_t;

class Mantle {
  protected:
    lua_State *L;
    map<rank_t, load_t>  server_load;
    int execute(const string &script, rank_t whoami,
                const vector < map<string, double> > &metrics);

  public:
    Mantle() : L(NULL) { };
    int start();
    int when(   const string &script, rank_t whoami, 
                const vector < map<string, double> > &metrics,
		bool &decision);
    int howmuch(const string &script, rank_t whoami, 
                const vector < map<string, double> > &metrics,
		float &decision);
    int where(  const string &script, rank_t whoami, 
                const vector < map<string, double> > &metrics,
                map<rank_t,double> &decision);
};

#endif
