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
#include <cerrno>

#define dout(level) (std::cout << "[src/Mantle.cc (" << __LINE__ << ")]\t" << level << ": " )
#define dendl (std::endl)

//!  A test class. 
/*!
  A more elaborate class description.
*/
int dout_wrapper(lua_State *L)
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
//!  A test class. 
/*!
  A more elaborate class description.
*/
int Mantle::start()
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

  return 0;
}

int Mantle::execute(Policy script, rank_t whoami, Metrics &metrics)
{
  if (start() != 0 || L == NULL) {
    dout(0) << "ERROR: mantle or lua was not started" << dendl;
    return -ENOENT;
  }

  /* tell the balancer which server is making the decision */
  lua_pushinteger(L, int(whoami));
  lua_setfield(L, -2, "whoami");

  /* global server metrics to hold all dictionaries */
  lua_newtable(L);

  /* push name of server (i) and its metrics onto Lua stack */
  for (unsigned i=0; i < metrics.size(); i++) {
    lua_pushinteger(L, i);
    lua_newtable(L);

    /* push values into this server's table; setfield assigns key/pops val */
    for (map<string, double>::const_iterator it = metrics[i].begin();
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

int Mantle::where(Policy script, Metrics metrics, rank_t whoami, map<rank_t,double> &my_targets)
{
  int ret = execute(script, whoami, metrics);
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
  rank_t it = rank_t(0);
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

int Mantle::when(Policy p, Metrics m, rank_t whoami, bool &decision)
{
  int ret = execute(p, whoami, m);
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

int Mantle::howmuch(Policy script, Metrics metrics, rank_t whoami, float &decision)
{
  int ret = execute(script, whoami, metrics);
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
