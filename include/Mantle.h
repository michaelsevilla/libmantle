// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
/**
 * \mainpage Mantle
 *
 * Mantle is an API and framework for designing load balancing policies.
 * 
 * \file Mantle.h
 * \brief API for the load balancing microservice
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
#include <lauxlib.h>

// TODO: get rid of this
using namespace std;

/**
 * \brief load metric, calculated using the load callback
 **/
typedef double Load;

/**
 * \brief Lua script containing the logic
 */
typedef string Policy;

/** 
 * \brief server name (e.g., MPI rank, MDS name)
 */
typedef int32_t Server;

/**
 * \brief measurements of resource utilizations used by policies (and populated by the service)
 */
typedef map<std::string, Load> ServerMetrics;

/**
 * \brief measurements of every server in the cluster
 */
typedef vector<ServerMetrics> ClusterMetrics;

/**
 * \brief access to a data structure used by policies (and populated by the service)
 */
typedef pair<int,int> Timestamp;

/**
 * \brief list of accesses to a data structure
 */
typedef Timestamp* TimeSeriesMetrics;

/** 
 * \brief describes which target server to send load to (see where callback)
 */
typedef map<Server,Load> Targets;

class Mantle {

  protected:
    lua_State *L;
    Server whoami;
    ClusterMetrics metrics;
    Policy load_callback, when_callback, howmuch_callback, where_callback;
    int execute(Policy p, Server s, ClusterMetrics m);

  public:
    /**
     * Initializes a Mantle instance
     *
     * @param s name of current server (the one running Mantle)
     */
    Mantle(Server s) : L(NULL)
    {
      whoami = s;
    };
   
    /**
     * Executes the when callback, which decides whether the service should
     * move load or not
     *
     * @param decision true if the policy wants the service to migrate load, false otherwise
     * @return 0 on success, < 0 otherwise
     */
    int when(bool &decision);

    /**
     * Executes the where callback, which decides where the service should
     * move load (which servers)
     *
     * @param decision dictionary (rank -> load) that describes which rank to
     * send load to @return 0 on success, < 0 otherwise
     */
    int where(Targets &decision);

    /**
     * Executes the howmuch callback, which decides how much load the service
     * should move
     *
     * @param decision amount of load the service should move @return 0 on
     * success, < 0 otherwise
     */
    int howmuch(Load &decision);

    /**
     * Configure load balancing policies
     *
     * @param  load callback that lets users scalarize metrics, run before all
     * other callbacks
     * @param  when callback that decides when to move load
     * @param  where callback that decides which servers to send load to
     * @param  howmuch callback that decides what fraction of load to shed
     * @return 0 on success, < 0 otherwise
     **/
    int configure(Policy load, Policy when, Policy where, Policy howmuch);
 
    /**
     * Populates cluster metrics with updated values
     *
     * @param metrics values to update metrics data structure with
     * @return 0 on success, < 0 otherwise
     */
    int update(ClusterMetrics metrics);

    /**
     * Populates time series metrics with updated values
     *
     * @param array of (timestamp, ID) accesses to expose to policy engine
     * @return 0 on success, < 0 otherwise
     */
    int update(TimeSeriesMetrics array, size_t size);

    /**
     * Executes the debug callback, which helps users inspect the environment
     *
     * @param p executable script that inspects environment
     */
    void debugenv(Policy p);
};
#endif
