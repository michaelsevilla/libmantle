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

/** 
 * \mainpage Mantle
 *
 * Mantle is a API and framework for designing load balancing policiers
 *
 * \file Mantle.h
 * \brief API for the load balancing microservice
 *
 */

#ifndef CEPH_MANTLE_H
#define CEPH_MANTLE_H

#include <lua.hpp>
#include <list>
#include <map>
#include <string>
#include <vector>

// TODO: get rid of this
using namespace std;

/**
 * \brief user-defined load metric, calculated using the load callback
 **/
typedef int32_t load_t;

/**
 * \brief Lua script containing the logic
 */
typedef const string Policy;

/**
 * \brief environment meeasurements used by the policies (and populated by the service)
 */
typedef const vector < map<string, double> > Metrics;

/** 
 * \brief server number (either MPI rank or MDS name)
 */
typedef int32_t rank_t;

/**
 * The central class for the Mantle Load Balancing API
 **/
class Mantle {

  protected:
    lua_State *L;
    map<rank_t, load_t>  server_load;
    int execute(Policy p, rank_t r, Metrics &m);

  public:
    Mantle() : L(NULL) { };

    /**
     * Initializes Mantle, including instantiating the Lua VM
     * @return 0 on success, < 0 otherwise
     **/
    int start();

    /**
     * Whether the service should move load or not
     * @param decision true if the policy wants the service to migrate load, false otherwise
     * @return 0 on success, < 0 otherwise
     */

    int when   (Policy p, Metrics m, rank_t r, bool &decision);

    /**
     * How much load the service should move
     *
     * @param decision amount of load the service should move
     * @return 0 on success, < 0 otherwise
     */
    int howmuch(Policy p, Metrics m, rank_t r, float &decision);

    /**
     * Where the service should move load (which servers)
     *
     * @param decision dictionary (rank -> load) that describes which rank to send load to
     * @return 0 on success, < 0 otherwise
     */
    int where  (Policy p, Metrics m, rank_t r, map<rank_t,double> &decision);
};

#endif
