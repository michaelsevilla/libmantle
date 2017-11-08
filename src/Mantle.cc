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

#include "Mantle.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cerrno>

#define dout(level) (std::cout << "[src/Mantle.cc (" << __LINE__ << ")]\t" << level << ": " )
#define dendl (std::endl)

/*
 * Functions called from Lua
 */
static int dout_wrapper(lua_State *L)
{
  #undef dout_prefix
  #define dout_prefix *_dout << "lua.balancer "

  /* Lua indexes the stack from the bottom up */
  int bottom = -1 * lua_gettop(L);
  if (!lua_isinteger(L, bottom) || bottom == 0) {
    dout(0) << "WARNING: BAL_LOG has no message" << dendl;
    return -EINVAL;
  }

  /* bottom of the stack is the log level */
  int level = lua_tointeger(L, bottom);

  /* rest of the stack is the message */
  string s = "";
  for (int i = bottom + 1; i < 0; i++)
    lua_isstring(L, i) ? s.append(lua_tostring(L, i)) : s.append("<empty>");

  dout(level) << s << dendl;
  return 0;
}

static int write_state(lua_State *L)
{
   /* Lua indexes the stack from the bottom up */
  int bottom = -1 * lua_gettop(L);
  if (!lua_isinteger(L, bottom) || bottom == 0) {
    dout(0) << "WARNING: WRstate called without a state" << dendl;
    return -EINVAL;
  }

  /* bottom of the stack is the log level */
  int state = lua_tointeger(L, bottom);

  /* write state to file, overwriting anything that's there */
  ofstream f("/tmp/state");
  f << state;
  f.close();
  return 0;
}

static int read_state(lua_State *L)
{
  /* read state to file */
  ifstream f("/tmp/state");
  stringstream buffer;
  buffer << f.rdbuf();
  f.close();

  lua_pushnumber(L, stoi(buffer.str()));
  return 1;
}


TimeSeriesMetrics timeseries;
static int timeseries_new(lua_State* L) {
  TimeSeriesMetrics* p = (TimeSeriesMetrics *)lua_newuserdata(L, sizeof(TimeSeriesMetrics *));
  *p = timeseries;
  luaL_getmetatable(L, "timeseries");
  lua_setmetatable(L, -2);
  return 1;
}

size_t size;
static int timeseries_size(lua_State* L) {
  lua_pushnumber(L, size);
  return 1;
}

static int timeseries_get(lua_State* L) {
  TimeSeriesMetrics* p = (TimeSeriesMetrics *)luaL_checkudata(L, 1, "timeseries");
  int index = luaL_checkinteger(L, 2);
  Timestamp ts = (Timestamp)(*p)[index-1];
  lua_pushinteger(L, ts.first);
  lua_pushinteger(L, ts.second);
  return 2;
}

/*
 * Mantle functions
 */
int Mantle::configure(Policy load, Policy when, Policy where, Policy howmuch)
{
  whoami = whoami;
  load_callback = load;
  when_callback = when;
  where_callback = where;
  howmuch_callback = howmuch; 
  return 0;
}

int Mantle::execute(Policy policy, Server whoami, ClusterMetrics metrics)
{
  /* build lua vm state */
  L = luaL_newstate();
  if (!L) {
    dout(0) << "WARNING: mantle could not load Lua state" << dendl;
    return -ENOEXEC;
  }

  /* balancer policies can use basic Lua functions */
  luaopen_base(L);

  /* setup debugging */
  lua_register(L, "BAL_LOG", dout_wrapper);
  lua_register(L, "WRstate", write_state);
  lua_register(L, "RDstate", read_state);

  if (L == NULL) {
    dout(0) << "ERROR: mantle or lua was not started" << dendl;
    return -ENOENT;
  }

  /* tell the balancer which server is making the decision */
  lua_pushinteger(L, whoami);
  lua_setfield(L, -2, "whoami");

  /* global server metrics to hold all dictionaries */
  lua_newtable(L);

  /* push name of server (i) and its metrics onto Lua stack */
  for (unsigned i=0; i < metrics.size(); i++) {
    lua_pushinteger(L, i);
    lua_newtable(L);

    /* push values into this server's table; setfield assigns key/pops val */
    for (ServerMetrics::const_iterator it = metrics[i].begin();
         it != metrics[i].end();
         it++) {
      lua_pushnumber(L, it->second);
      lua_setfield(L, -2, it->first.c_str());
    }

    /* in global server table at stack[-3], set k=stack[-1] to v=stack[-2] */
    lua_rawset(L, -3);
  }

  /* set the name of the global server table */
  lua_setglobal(L, "server");

  /* functions we can call on a timeseries object in Lua*/
  static const struct luaL_Reg timeseries[] = {
     { "get", timeseries_get },
     { "size", timeseries_size },
     {NULL, NULL}
  };

  /* expose timeseries from C++ to Lua as a metatable*/
  lua_register(L, "timeseries", timeseries_new);
  luaL_newmetatable(L, "timeseries");

  /* redirect __index to metatable, which sits at the top of the stack */
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaL_setfuncs(L, timeseries, 0);

  /* append policy to load callback, so we calculate load before execution */
  Policy script = load_callback + "\n" + policy;

  /* load the balancer */
  if (luaL_loadstring(L, script.c_str())) {
    dout(0) << "WARNING: mantle could not load balancer: "
            << lua_tostring(L, -1) << dendl;
    return -EINVAL;
  }

  /* compile/execute balancer */
  int ret = lua_pcall(L, 0, LUA_MULTRET, 0);
  if (ret) {
    dout(0) << "WARNING: mantle could not execute script: "
            << lua_tostring(L, -1) << dendl;
    return -EINVAL;
  }

  return 0;
}

int Mantle::where(Targets &my_targets)
{
  int ret = execute(where_callback, whoami, metrics);
  if (ret != 0) {
    lua_close(L);
    return ret;
  }

  /* parse response by iterating over Lua stack */
  if (lua_istable(L, -1) == 0) {
    dout(0) << "WARNING: mantle script returned a malformed response" << dendl;
    lua_close(L);
    return -EINVAL;
  }

  /* fill in return value */
  Server it = Server(0);
  lua_pushnil(L);
  while (lua_next(L, -2) != 0) {
    if (!lua_isnumber(L, -1)) {
      dout(0) << "WARNING: mantle script returned a malformed response" << dendl;
      lua_close(L);
      return -EINVAL;
    }
    my_targets[it] = (lua_tonumber(L, -1));
    lua_pop(L, 1);
    it++;
  }

  lua_close(L);
  return 0;
}

int Mantle::when(bool &decision)
{
  int ret = execute(when_callback, whoami, metrics);
  if (ret != 0) {
    lua_close(L);
    return ret;
  }

  /* parse response by iterating over Lua stack */
  if (lua_isboolean(L, -1) == 0) {
    dout(0) << "WARNING: mantle script returned a malformed response" << dendl;
    lua_close(L);
    return -EINVAL;
  }

  /* fill in return value */
  decision = lua_toboolean(L, -1);
  lua_pop(L, 1);
  lua_close(L);
  return 0;
}

int Mantle::howmuch(Load &decision)
{
  int ret = execute(howmuch_callback, whoami, metrics);
  if (ret != 0) {
    lua_close(L);
    return ret;
  }

  /* parse response by iterating over Lua stack */
  if (lua_isnumber(L, -1) == 0) {
    dout(0) << "WARNING: mantle script returned a malformed response" << dendl;
    lua_close(L);
    return -EINVAL;
  }

  /* fill in return value */
  decision = lua_tonumber(L, -1);
  lua_pop(L, 1);
  lua_close(L);
  return 0;
}


int Mantle::update(ClusterMetrics m)
{
  metrics = m;
  return 0;
}

int Mantle::update(TimeSeriesMetrics ts, size_t s) {
  timeseries = ts;
  size = s;
  return 0;
}

void Mantle::debugenv(Policy p)
{
  execute(p, whoami, metrics);
}
