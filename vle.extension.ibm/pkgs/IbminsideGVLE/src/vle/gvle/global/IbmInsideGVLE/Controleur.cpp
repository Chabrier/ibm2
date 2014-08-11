/*
 * @file Controleur.cpp
 *
 */
#include "ControleurProxy.hpp"
#include "Controleur.hpp"
#include <vle/devs/Executive.hpp>
#include <vle/devs/ExecutiveDbg.hpp>
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
namespace vv = vle::value;

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
: vd::Executive(mdl, events), mEvents(events), mInternTransition(false), mValueStart(0)
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
    //if(luaL_dostring(L, mScript.c_str()))
		//ReportErrors(L);
		int errorCode = luaL_dostring(L, mScript.c_str());
		
    PrintErrorMessageOrNothing(errorCode);
    
    if (events.exist("ScriptExec"))
        mScriptExec = events.getXml("ScriptExec");
    else
        throw vle::utils::ModellingError("Text ScriptExec not found");

    std::ofstream file("/home/gcicera/outputTestHahaha.vpz");
    dump(file, "dumpHahaha");
}

Controleur::~Controleur() {}

vd::Time Controleur::init(const vd::Time& t)
{
    if (mInstructionsComing.size() > 0)
        ta = mInstructionsComing.back().first - t;
    else ta = vd::infinity;
    mInternTransition = true;
    return ta;
}

void Controleur::internalTransition(const devs::Time& t)
{
    if(mNextExternalEvent.size() != 0) {
        ta = 0.0001;
    } else {
        ta = vd::infinity;
    }
    mNextExternalEvent.clear();
    //
    //std::cout << "internal transition " << t << std::endl;
    /*if (mInstructionsComing.size() > 0) {
        std::pair<int, std::string> it = mInstructionsComing.back();
        while (it.first - t <= 0) {
            parseOneLine(it.second);
            mInstructionsComing.pop_back();
            if (mInstructionsComing.size() > 0)
                it = mInstructionsComing.back();
            else {
                ta = vd::infinity;
                break;
            }
        }
    }*/
    //mInternTransition = true;
}

void Controleur::externalTransition(const vd::ExternalEventList& events,
                        const vd::Time& t)
{
    //std::cout << "external " << mScriptExec.c_str() << std::endl;
    /*if (mInstructionsComing.size() > 0) 
        ta = mInstructionsComing.back().first - t;
    else */ta = 0.0001;//std::numeric_limits<double>::epsilon();
    //std::cout << "ext " << events[0]->getPortName() << " " << events.size() << std::endl;
    updateData(events);
    int errorCode = luaL_dostring(L, mScriptExec.c_str());
    PrintErrorMessageOrNothing(errorCode);
    //showData();
    //if(mValueStart == 0) {
        //std::cout << "NUM " << countModelOfClass("R1") << std::endl;
        //luaL_dostring(L, mScriptExec.c_str());
       // mValueStart = 1;
    /*}
    if (mNextExternalEvent.size()>0) {
        ta = 0;
    }*/
}

vd::Time Controleur::timeAdvance() const
{
    //if(mInternTransition)
    
    if (mNextExternalEvent.size()>0 && ta == vd::infinity) {
        //std::cout << "timeAdvance " << 0 << std::endl;
        return 0;
    }
    //std::cout << "timeAdvance " << ta << std::endl;
    return ta;
    //return std::numeric_limits<double>::epsilon();
}

void Controleur::output(const vd::Time& time ,
                    vd::ExternalEventList& output) const
{
    //std::cout << "on output " << time << " " << mNextExternalEvent.size() << std::endl;
    for (unsigned int i=0; i<mNextExternalEvent.size(); i++) {
        output.push_back(mNextExternalEvent[i]);
    }        
}

vv::Value* Controleur::observation(const vd::ObservationEvent& event) const {


    lua_getglobal(L, event.getPortName().c_str());
    if (!lua_isnumber(L, -1)) {
        //printf ("error\n");
        //throw utils::ArgError(fmt("Variable `%1%' is not a number") % event.getPortName());
        return 0;
    }

    double nb = lua_tonumber(L, -1);
    lua_settop(L,0);
    //std::cout << "p value " << nb << " " << lua_gettop(L) << std::endl;
    
    //std::cout << "on observable " << std::endl;
    //std::cout << "events " << event.getPortName() << std::endl;
    return new vv::Double(nb);
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
    std::cout << "DELETE " << modelName << std::endl;
    mData.erase(modelName);
}

void Controleur::addVar(std::string varName, vle::value::Value* varValue) {
    mVariables.insert(std::pair<std::string, vle::value::Value*>(varName, varValue));
}

/**
 * @brief Parse the script
 */
void Controleur::parseScript() {
    std::vector<std::string> lines;
    boost::split(lines, mScript, boost::is_any_of("\n"));
    
    for (unsigned int i = 0; i<lines.size(); i++) {
        parseOneLine(lines[i]);
    }
    showData();
    showInstructionListWaiting();
}

void Controleur::parseOneLine(std::string line) {
    std::cout << "enum " << line << std::endl;
    std::vector<std::string> words;
    boost::split(words, line, boost::is_any_of(" "));
    boost::to_upper(words[0]);
    if (words[0] == "ADD")
    {
        try {
            int nb_clone = readNumber(words[1]);
            std::map <std::string, vv::Value*> variableToModify;
            if (boost::contains(line, "WITH")) {
                variableToModify = parseWITH(line);
                modifyParameter(words[2], variableToModify);
             }
            addInstruction(nb_clone, words[2]);
        } catch(const char* Message) {
            throw utils::ArgError(Message);
        }
    } else if (words[0] == "DEL") {
        delInstruction(words);
    } else if (words[0].substr(0,1) == "@") {
        std::string time = words[0];
        std::string instruction = line.substr(words[0].size() + 1);
        int time_dec = atoi(time.substr(1).c_str());
        atRegister(time_dec, instruction);
    } else if (words[0] != ""){
        throw utils::ArgError(fmt("Directive `%1%' not found") % words[0]);
    }
}

void Controleur::atRegister(double time, std::string instruction) {
    mInstructionsComing.push_back(std::pair<int, std::string> (time, instruction));
    CompareTime compTime;
    std::sort(mInstructionsComing.begin(), mInstructionsComing.end(), compTime);
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

std::map <std::string, vv::Value*> Controleur::parseWITH(std::string line) {
    std::map <std::string, vv::Value*> nameAndValue;
    boost::to_upper(line);
    std::string part = line.substr(line.find("WITH") + 5);
    std::vector<std::string> variablesEqualValue;
    boost::split(variablesEqualValue, part, boost::is_any_of(","));
    for (unsigned int i=0; i<variablesEqualValue.size(); i++) {
        std::vector<std::string> variablesAndValue;
        boost::split(variablesAndValue, variablesEqualValue[i], boost::is_any_of("="));
        boost::replace_all(variablesAndValue[0], " ", "");
        boost::replace_all(variablesAndValue[1], " ", "");
        //nameAndValue.insert(std::pair<std::string, vv::Double> (variablesAndValue[0], variablesAndValue[1])); Changer le string en double
    }
    return nameAndValue;
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
    //it->second = i;
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
        std::string temp = it->first.substr(0, modelName.size());
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

void Controleur::showDataLua() {
    double d = 0;
    lua_getglobal(L, "R1_C1");
    d = (double)lua_tonumber(L, -1);
    std::cout << "youhou !!! " << d << std::endl;
}

void Controleur::showInstructionListWaiting() {
    for (unsigned int i = 0; i<mInstructionsComing.size(); i++) {
        std::cout << "Instruction waiting " << mInstructionsComing[i].first << " " << mInstructionsComing[i].second << std::endl;
    }
}

std::string Controleur::getModelNameFromPort(std::string s) {
    unsigned i = s.find_last_of("_");
    return s.substr(0, i);
}

void Controleur::updateData(const vd::ExternalEventList& events) {
    for (unsigned int i=0; i<events.size(); i++) {
        std::string s = events[i]->getPortName();
        std::string variable = events[i]->getAttributes().get("name")->toString().value();

        putInStructure(getModelNameFromPort(s), variable, events[i]->getAttributes().get("value"));
    }
    //showDataLua();
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
    std::cout << "set " << it->first << " " << varName << " " << varValue << std::endl;
    vd::ExternalEvent* e = new vd::ExternalEvent(it->first + "_toPerturb");
    e->putAttribute("name", new vv::String(varName));
    e->putAttribute("value", new vv::Double(varValue));
    mNextExternalEvent.push_back(e);
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

int Controleur::countModelOfClass(std::string className) {
    int i = 0; 
    for (std::map <std::string, std::map <std::string, vle::value::Value*> >::iterator it = mData.begin(); it!=mData.end(); ++it) {
        if (compareModelClass(it->first, className)) {
            i++;
        }
    }
    return i;
}

int Controleur::PrintErrorMessageOrNothing(int errorCode)
{
   if (errorCode != 0) {
		throw utils::ArgError(fmt("Lua Error Code: `%1%' ") % lua_tostring(L, -1));
		//printf("%s\n", lua_tostring(L, -1));
		}
   return errorCode;
}

}}}} // namespace vle gvle global ibminsidegvle

DECLARE_EXECUTIVE_DBG(vle::gvle::global::ibminsidegvle::Controleur)
