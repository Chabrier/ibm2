/*
* This file is part of VLE, a framework for multi-modeling, simulation
* and analysis of complex dynamical systems.
* http://www.vle-project.org
*
* Copyright (c) 2013 INRA http://www.inra.fr
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to
* deal in the Software without restriction, including without limitation the
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
* sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*/
#ifndef CONTROLEURPROXY_HPP
#define CONTROLEURPROXY_HPP

#include <boost/lexical_cast.hpp>

#include <vle/value/Value.hpp>
#include <vle/devs/Executive.hpp>
#include <vle/value/Map.hpp>
#include <vle/utils/Rand.hpp>

#include<math.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "lunar.h"

namespace vd = vle::devs;
namespace vp = vle::vpz;
namespace vu = vle::utils;

namespace vle {
namespace gvle {
namespace global {
namespace ibminsidegvle {

class Controleur;

class ControleurProxy {
    enum functionType { INIT, SINGLE, RECUR};
    
    Controleur* mControleur;

public:
    static const char className[];
    static Lunar<ControleurProxy>::RegType methods[];

    ControleurProxy(lua_State */*L*/) {}
    ControleurProxy() {}

    void setControleur ( Controleur* controleur);

    int addModel(lua_State *L);
    int addModelWithParam(lua_State *L);
    int delModel(lua_State *L);
    int getModelValue(lua_State *L);
    int getNumber(lua_State *L);
    int setModelValue(lua_State *L);
    int addGlobalVar(lua_State *L);
    int getModelName(lua_State *L);
    int getTime(lua_State *L);
    int addEvent(lua_State *L);
    int getParam(lua_State *L);
    double getValueFromParam(lua_State *L, int i);

    ~ControleurProxy();
};

}}}} // namespace vle gvle global ibminsidegvle

#endif
