/*
 * @file Controleur.cpp
 *
 */
#include "ControleurProxy.hpp"
#include "Controleur.hpp"
//#include <vle/devs/Executive.hpp>
//#include <vle/devs/ExecutiveDbg.hpp>
#include <vle/vpz/Conditions.hpp>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream> //Pour dump
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include <vle/vpz/ModelPortList.hpp>
#include <limits>
#include <algorithm>
#include <vector>
#include <vle/utils/Types.hpp>

namespace vd = vle::devs;
namespace vc = vle::vpz;
//namespace vv = vle::value;

namespace vle {
namespace gvle {
namespace global {
namespace ibminsidegvle {
/*
*@@tagdynamic@@
*@@tagdepends:@@endtagdepends
*/

Controleur::Controleur(const vd::ExecutiveInit& mdl,
           const vd::InitEventList& events)
: GenericAgent(mdl, events), mEvents(events)
{
    L = luaL_newstate();
    luaL_openlibs(L);

    Lunar<ControleurProxy>::Register(L);

    mCP.setControleur(this);

    lua_settop(L,0);

    Lunar<ControleurProxy>::push(L, &mCP);
    lua_setglobal(L, "ibm");

    if (events.exist("Script"))
        mScript = events.getXml("Script");
    else
        throw vle::utils::ModellingError("Text Script not found");

    if (events.exist("ScriptExec"))
        mScriptExec = events.getXml("ScriptExec");
    else
        throw vle::utils::ModellingError("Text ScriptExec not found");
        
    addEffect("doScript", boost::bind(&Controleur::doScript,this,_1));
}

Controleur::~Controleur() {}


void Controleur::agent_init() {
    int errorCode = luaL_dostring(L, mScript.c_str());	
    PrintErrorMessageOrNothing(errorCode);
    
    Effect update = doScriptEffect(mCurrentTime + 0.001, getModelName()+"toto");
    mScheduler.addEffect(update);
}

void Controleur::agent_dynamic() {
    Effect nextEffect = mScheduler.nextEffect();
    applyEffect(nextEffect.getName(),nextEffect);
}

void Controleur::agent_handleEvent(const Message& message) {
    if (std::find(mDeadModel.begin(), mDeadModel.end(), getModelNameFromPort(message.getSender())) == mDeadModel.end())
        putInStructure(getModelNameFromPort(message.getSender()), std::string(message.get("name")->toString().value()), new vv::Double(message.get("value")->toDouble()));
}

Effect Controleur::doScriptEffect(double t,const std::string& source)
{
    Effect effect(t,"doScript",source);
    return effect;
}

void Controleur::doScript(const Effect& /*e*/) {
    int errorCode = luaL_dostring(L, mScriptExec.c_str());
    PrintErrorMessageOrNothing(errorCode);
    Effect update = doScriptEffect(mCurrentTime + 0.001, getModelName()+"toto");
    mScheduler.update(update);
}

/**
 * @brief Create nb_clone modelwith the className Class
 * 
 * @param int nb_clone
 * @param std::string className
 */
void Controleur::addInstruction(int nb_clone, std::string className) {
    for (int i=0; i<nb_clone; i++){
		addOneModel(className);
	}
}

std::map <std::string, vv::Value*> Controleur::modifyParameter(std::string className, std::map <std::string, vv::Value*> variableToModify) 
{
    std::map <std::string, vv::Value*> variableModified;
    vc::Conditions& cl = conditions();
	vc::Condition& c = cl.get("cond_DTE_" + className);
	for (std::map <std::string, vv::Value*>::iterator it = variableToModify.begin(); it != variableToModify.end(); ++it) {
	    vle::value::Value* d = c.firstValue(it->first).clone();
	    variableModified.insert(std::pair<std::string, vv::Value*>(it->first, d));
	    c.setValueToPort(it->first, *(it->second));//c_str()
	}
	
	return variableModified;
}

void Controleur::addModelWith(int nb_clone, std::string className, std::map <std::string, vv::Value*> variableToModify) {
    std::map <std::string, vv::Value*> variableModified = modifyParameter(className, variableToModify);
    addInstruction(nb_clone, className);
    modifyParameter(className, variableModified);
}

void Controleur::delOneModel(std::string modelName) {
    delModel(modelName);
    removeInputPortExec(modelName);
    removeOutputPortExec(modelName);
    mData.erase(modelName);
    mDeadModel.push_back(modelName);
}

/**
 * @brief Find the number associated to the variable
 *
 * @param std::string nb
 *
 * @return int 
 */
int Controleur::readNumber(std::string nb) {
    if (mEvents.exist(nb))
        return mEvents.getInt(nb);
    
    throw utils::ArgError(fmt("Variable `%1%' not found ") % nb);
}

std::string Controleur::addOneModel(std::string className) {
    std::string modelName = findModelName(className);
	const vpz::BaseModel* newModel = createModelFromClass(className, modelName);
	connectionModelToExec(modelName, newModel);
	connectionExecToModel(modelName);
	
	return modelName;
}

/**
 * @brief Find a name starting with className_ and a number doesn't used
 *
 * @param std::string className
 *
 * @return std::string
 */
std::string Controleur::findModelName(std::string className) {
    int i = 0;
    std::string modelName = className + "_";
    std::map<std::string, int>::iterator it = mNameNumber.find(className);
    if (it == mNameNumber.end()) {
        mNameNumber.insert(std::pair<std::string, int> (className,i));
	} else {
	    i = it->second;
	}
	std::stringstream ss;
    ss << i;
    modelName += ss.str();
    i++;
    mNameNumber[className] = i;
    return modelName;
}

/**
 * @brief Connect the model to the executive
 *
 * @param std::string modelName
 * @param const vpz::BaseModel* model
 */
void Controleur::connectionModelToExec(std::string modelName, const vpz::BaseModel* model) {
    //std::map< std::string, ModelPortList > vle::vpz::ConnectionList
    std::map<std::string, vpz::ModelPortList> portList = model->getOutputPortList();
    for (std::map<std::string, vpz::ModelPortList>::iterator it=portList.begin(); it!=portList.end(); ++it){
        std::string outputPortName = it->first;
        std::string inputPort = modelName + "_" + outputPortName;
        addInputPort(getModelName(), inputPort);
        addConnection(modelName, outputPortName, getModelName(), inputPort);
    }
}

/**
 * @brief Connect the executive with the model
 *
 * @param std::string the model name
 */
void Controleur::connectionExecToModel(std::string modelName) {
    std::string outputPortName = modelName + "_toPerturb";
    addOutputPort(getModelName(), outputPortName);
    addConnection(getModelName(), outputPortName, modelName, "perturb");
}

/**
 * @brief Delete models
 *
 * @param std::vector<std::string> list of the model name to remove
 */
void Controleur::delInstruction(std::vector<std::string> words) {
    for (unsigned int i=1; i<words.size(); i++) {
        delOneModel(words[i]);
    }
}

/**
 * @brief Remove the input port of the executive associated to the model
 *
 * @param std::string model name
 */
void Controleur::removeInputPortExec(std::string modelName) {
    std::vector<std::string> toRemove;
    std::map<std::string, vpz::ModelPortList> portList = getModel().getInputPortList();
    for (std::map<std::string, vpz::ModelPortList>::iterator it=portList.begin(); it!=portList.end(); ++it){
        std::string temp = getModelNameFromPort(it->first);
        if (temp == modelName) {
            toRemove.push_back(it->first);
        }
    }
    for (unsigned int i=0; i<toRemove.size(); i++) {
        removeInputPort(getModelName(), toRemove[i]);
    }
    
}

/**
 * @brief Remove the output port of the executive associated to pertub port of modelName
 *
 * @param std::string
 */
void Controleur::removeOutputPortExec(std::string modelName) {
    removeOutputPort(getModelName(), modelName + "_toPerturb");
}

void Controleur::putInStructure(std::string modelName, std::string variable, vle::value::Value* value) {
    std::map<std::string,std::map <std::string, vle::value::Value*> >::iterator it = mData.find(modelName);
    if (it == mData.end()){
        std::map <std::string, vle::value::Value*> secondMap;
        secondMap.insert(std::pair<std::string,vle::value::Value*>(variable, value->clone()));
        mData.insert(std::pair<std::string,std::map <std::string, vle::value::Value*> >(modelName, secondMap));
        
    } else {
        std::map <std::string, vle::value::Value*>& temp = mData.find(modelName)->second;
        if (!temp.insert(std::pair<std::string,vle::value::Value*>(variable, value->clone())).second) {
            temp[variable] = value->clone();
        }
    }
    
}

void Controleur::showData() {
    for (std::map <std::string, std::map <std::string, vle::value::Value*> >::iterator it=mData.begin(); it!=mData.end(); ++it) {
        for (std::map <std::string, vle::value::Value*>::iterator it2=it->second.begin(); it2!=it->second.end(); ++it2) {
            std::cout << "clé " << it->first << " variable " << it2->first << " : " << *(it2->second) << std::endl;
        }
    }
}

void Controleur::updateData(const vd::ExternalEventList& events) {
    for (unsigned int i=0; i<events.size(); i++) {
        std::string s = events[i]->getPortName();
        std::string variable = events[i]->getAttributes().get("name")->toString().value();

        putInStructure(getModelNameFromPort(s), variable, events[i]->getAttributes().get("value"));
    }
}

double Controleur::getData(std::string modelName, std::string varName) {
    return mData.find(modelName)->second.find(varName)->second->toDouble().value();
}

double Controleur::getData(std::string className, int n, std::string varName) {
    std::map <std::string, std::map <std::string, vle::value::Value*> >::iterator it = getItFromData(className, n);
    return it->second.find(varName)->second->toDouble().value();
}

void Controleur::setModelValue(std::string className, int n, std::string varName, double varValue) {
    std::map <std::string, std::map <std::string, vle::value::Value*> >::iterator it = getItFromData(className, n);
    /*vd::ExternalEvent* e = new vd::ExternalEvent(it->first + "_toPerturb");
    e->putAttribute("name", new vv::String(varName));
    e->putAttribute("value", new vv::Double(varValue));
    mNextExternalEvent.push_back(e);*/
    Message m(getModelName(),it->first,"");
    m.add("name",vv::String::create(varName));
    m.add("value",vv::Double::create(varValue));
    sendMessage(m);
}

std::map <std::string, std::map <std::string, vle::value::Value*> >::iterator Controleur::getItFromData(std::string className, int n) {
    int i = 0; 
    for (std::map <std::string, std::map <std::string, vle::value::Value*> >::iterator it = mData.begin(); it!=mData.end(); ++it) {
        if (compareModelClass(it->first, className)) {
            i++;
            if (i == n) {
                return it;
            }
        }
    }
    throw utils::ArgError(fmt("Not enough `%1%' ") % className);
}

bool Controleur::compareModelClass(std::string modelName, std::string className) {
    if (modelName.substr(0, className.size()) == className) {
        return true;
    }
    return false;
}

std::string Controleur::getModelNameFromClassNb(std::string className, int i) {
    std::map <std::string, std::map <std::string, vle::value::Value*> >::iterator it = getItFromData(className, i);
    return it->first;
}

std::string Controleur::getModelNameFromPort(std::string s) {
    unsigned i = s.find_last_of("_");
    return s.substr(0, i);
}

int Controleur::countModelOfClass(std::string className) {
    int i = 0; 
    for (std::map <std::string, std::map <std::string, vle::value::Value*> >::iterator it = mData.begin(); it!=mData.end(); ++it) {
        if (compareModelClass(it->first, className)) {
            i++;
        }
    }
    return i;
}

double Controleur::getTime() {
    return mCurrentTime;
}

int Controleur::PrintErrorMessageOrNothing(int errorCode)
{
   if (errorCode != 0) {
		throw utils::ArgError(fmt("Lua Error Code: `%1%' ") % lua_tostring(L, -1));
		}
   return errorCode;
}

}}}} // namespace vle gvle global ibminsidegvle

DECLARE_EXECUTIVE(vle::gvle::global::ibminsidegvle::Controleur)
