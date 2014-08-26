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
#include "ControleurProxy.hpp"
#include "Controleur.hpp"

namespace vle {
namespace gvle {
namespace global {
namespace ibminsidegvle {

const char ControleurProxy::className[] = "ControleurProxy";

Lunar<ControleurProxy>::RegType ControleurProxy::methods[] = {
    LUNAR_DECLARE_METHOD(ControleurProxy, addModel),
    LUNAR_DECLARE_METHOD(ControleurProxy, addModelWithParam),
    LUNAR_DECLARE_METHOD(ControleurProxy, delModel),
    LUNAR_DECLARE_METHOD(ControleurProxy, getModelValue),
    LUNAR_DECLARE_METHOD(ControleurProxy, getNumber),
    LUNAR_DECLARE_METHOD(ControleurProxy, setModelValue),
    LUNAR_DECLARE_METHOD(ControleurProxy, getModelName),
    LUNAR_DECLARE_METHOD(ControleurProxy, getTime),
    LUNAR_DECLARE_METHOD(ControleurProxy, addEvent),
    {0,0} };
    
    
void ControleurProxy::setControleur ( Controleur* controleur) {mControleur = controleur;}

ControleurProxy::~ControleurProxy() { printf("deleted GodProxy (%p)\n", this); }

int ControleurProxy::addModel(lua_State *L) {
    int nbi = 0;
    if (lua_type(L, 1) == LUA_TNUMBER) {
        nbi = luaL_checknumber(L, 1);
    } else {
        nbi = mControleur->readNumber(luaL_checkstring(L, 1));
    }
    std::string si (luaL_checkstring(L, 2));
    mControleur->addInstruction(nbi, si);
    return 0;
}

int ControleurProxy::addModelWithParam(lua_State *L) {
    std::map<std::string, vv::Value*> variableToModify;
    int top = lua_gettop(L);
    std::string varName = "";
    
    for (int i = 3; i <= top; i++) {
        if (i%2 != 0) {
            varName = lua_tostring(L, i);
        } else {
            double d = lua_tonumber(L, i);
            vv::Value* varValue = new vv::Double(d);
            variableToModify.insert(std::pair<std::string, vle::value::Value* >(varName, varValue));
        }        
    }
    mControleur->addModelWith(luaL_checknumber(L, 1), luaL_checkstring(L, 2), variableToModify);
    return 0;
}

int ControleurProxy::delModel(lua_State *L) {

    for (int i=1; i<=lua_gettop(L); i++) {
        mControleur->delOneModel(lua_tostring(L, i));
    }
    
    return 0;
}

int ControleurProxy::getModelValue(lua_State *L) {
    std::string modelName (luaL_checkstring(L, 1));
    
    double d = 0;
    if (lua_gettop(L) == 2) {
        std::string varName (luaL_checkstring(L, 2));
        d = mControleur->getData(modelName, varName);
    } else {
        int nb = luaL_checknumber(L, 2);
        std::string varName (luaL_checkstring(L, 3));
        d = mControleur->getData(modelName, nb, varName);
    }
    lua_pushnumber(L, d);
    return 1;
}

int ControleurProxy::getNumber(lua_State *L) {
    std::string className = luaL_checkstring(L, 1);
    int d = mControleur->countModelOfClass(className);
    lua_pushnumber(L, d);
    return 1;
}

int ControleurProxy::setModelValue(lua_State *L) {
    if (lua_gettop(L) == 4) {
        std::string className (luaL_checkstring(L, 1));
        int i = luaL_checknumber(L, 2);
        std::string varName (luaL_checkstring(L, 3));
        int varValue = luaL_checknumber(L, 4);
        mControleur->setModelValue(className, i, varName, varValue);
    } else {
        std::string modelName (luaL_checkstring(L, 1));
        std::string varName (luaL_checkstring(L, 2));
        int varValue = luaL_checknumber(L, 3);
        mControleur->setModelValue(modelName, varName, varValue);
    }
    return 0;
}

int ControleurProxy::getModelName(lua_State *L) {
    std::string className (luaL_checkstring(L, 1));
    int i = luaL_checknumber(L, 2);
    std::string name = mControleur->getModelNameFromClassNb(className, i);
    lua_pushstring(L, name.c_str());
    return 1;
}

int ControleurProxy::getTime(lua_State *L) {
    lua_pushnumber(L, mControleur->getTime());
    return 1;
}

int ControleurProxy::addEvent(lua_State *L) {
    std::string type = luaL_checkstring(L, 1);
    if (type == "INIT") {
        std::string script (luaL_checkstring(L, 2));
        script += "()";
        mControleur->execInit(script);
    } else if (type == "SINGLE") {
        mControleur->addEffectAt(luaL_checknumber(L, 2), vd::infinity, luaL_checkstring(L, 3));
    } else if (type == "RECUR") {
        mControleur->addEffectAt(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checkstring(L, 4));
    }
    return 0;
}

}}}} // namespace vle gvle global ibminsidegvle
