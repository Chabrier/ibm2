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
/*namespace vd = vle::devs;
namespace vv = vle::value;
namespace vp = vle::vpz;
namespace vu = vle::utils;
*/
namespace vle {
namespace gvle {
namespace global {
namespace ibminsidegvle {

const char ControleurProxy::className[] = "ControleurProxy";

Lunar<ControleurProxy>::RegType ControleurProxy::methods[] = {
    LUNAR_DECLARE_METHOD(ControleurProxy, addModel),
    LUNAR_DECLARE_METHOD(ControleurProxy, addModelWith),
    LUNAR_DECLARE_METHOD(ControleurProxy, delModel),
    LUNAR_DECLARE_METHOD(ControleurProxy, atAction),
    LUNAR_DECLARE_METHOD(ControleurProxy, get),
    {0,0} };
    
    
void ControleurProxy::setControleur ( Controleur* controleur) {mControleur = controleur;}

ControleurProxy::~ControleurProxy() { printf("deleted GodProxy (%p)\n", this); }

int ControleurProxy::addModel(lua_State *L) {
    int nbi = luaL_checknumber(L, 1);
    std::string si (luaL_checkstring(L, 2));
    std::cout << "ADD " << nbi << " " << si << " " << lua_gettop(L) << std::endl;
    std::cout << "This was just to test!!!"<< std::endl;
    mControleur->addInstruction(nbi, si);
    return 0;
}

int ControleurProxy::addModelWith(lua_State *L) {
    std::cout << "ADD WITH " << std::endl;
    std::map<std::string, vv::Value*> variableToModify;
    int top = lua_gettop(L);
    std::string varName = "";
    
    for (int i = 3; i <= top; i++) {
        int t = lua_type(L, i);
        vv::Value* varValue;
        switch (t) {
          case LUA_TSTRING: { 
            varName = lua_tostring(L, i);
            std::cout << varName << std::endl;
            break;
            }
          case LUA_TNUMBER:{
            double d = lua_tonumber(L, i);
            varValue = new vv::Double(d);
            std::cout << varValue->toDouble().value() << std::endl;
            break;
            }
          default:
            printf("%s", lua_typename(L, t));
            break;
        }
        
        if (i%2 == 0) {
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

int ControleurProxy::atAction(lua_State *L) {
    double time = luaL_checknumber(L, 1);
    std::string action (luaL_checkstring(L, 2));
    
    //mControleur->atRegister(time, action);
    return 0;
}

int ControleurProxy::get(lua_State *L) {
    std::string modelName (luaL_checkstring(L, 1));
    std::string varName (luaL_checkstring(L, 2));
    double d = mControleur->getData(modelName, varName);
    std::cout << "value : " << d << std::endl;
    return 0;
}

}}}} // namespace vle gvle global ibminsidegvle
