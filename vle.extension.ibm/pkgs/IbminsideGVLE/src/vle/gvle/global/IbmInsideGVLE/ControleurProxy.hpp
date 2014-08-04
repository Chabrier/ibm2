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

#include <vle/gvle/DllDefines.hpp>
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
namespace vv = vle::value;
namespace vp = vle::vpz;
namespace vu = vle::utils;

namespace vle {
namespace gvle {
namespace global {
namespace ibminsidegvle {

class Controleur;

class GVLE_API ControleurProxy {
    Controleur* mControleur;

public:
    static const char className[];
    static Lunar<ControleurProxy>::RegType methods[];

    ControleurProxy(lua_State *L) {}
    ControleurProxy() {}

    void setControleur ( Controleur* controleur);

    int addModel(lua_State *L);
    int addModelWith(lua_State *L);
    int delModel(lua_State *L);
    int atAction(lua_State *L);
    int get(lua_State *L);
    /*int createSky(lua_State *L);
    int createBird(lua_State *L);
    int inANeigborhood(lua_State *L);*/

    ~ControleurProxy();
};

}}}} // namespace vle gvle global ibminsidegvle

#endif
