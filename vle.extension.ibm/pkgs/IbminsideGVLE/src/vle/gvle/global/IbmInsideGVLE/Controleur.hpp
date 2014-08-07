/*
 * @file Controleur.cpp
 *
 */
 
#ifndef CONTROLEUR_HPP
#define CONTROLEUR_HPP

#include "ControleurProxy.hpp"
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
class Controleur : public vd::Executive
{    
public:
    Controleur(const vd::ExecutiveInit& mdl,
               const vd::InitEventList& events);

    virtual ~Controleur();

    virtual vd::Time init(const vd::Time& t);
    
    virtual void internalTransition(const devs::Time& t);

    void externalTransition(const vd::ExternalEventList& events,
                            const vd::Time& t);

    virtual vd::Time timeAdvance() const;

    virtual void output(const vd::Time& /* time */,
                        vd::ExternalEventList& output) const;
    
    vv::Value* observation(const vd::ObservationEvent& event) const;

    void addInstruction(int nb_clone, std::string className);

    void addModelWith(int nb_clone, std::string className, std::map <std::string, vv::Value*> variableToModify);
    
    void delOneModel(std::string modelName);
    
    double getData(std::string modelName, std::string varName);
    double getData(std::string className, int n, std::string varName);
    
    std::string getModelNameFromClassNb(std::string className, int i);
    
    int countModelOfClass(std::string className);
    
    void setModelValue(std::string className, int n, std::string varName, double varValue);
    
    /**
     * @brief Find the number associated to the variable
     *
     * @param std::string nb
     *
     * @return int 
     */
    int readNumber(std::string nb);
    
private:
    const vd::InitEventList& mEvents;
	std::string mScript;
	std::string mScriptExec;
	std::map <std::string, std::map <std::string, vle::value::Value*> > mData;
	bool mInternTransition;
	double mValueStart;
	double mValueC1;
	double mValueC2;
	std::string toEmptyName;
	std::string toFillName;
	std::map <std::string, int> mNameNumber;
	std::vector <std::pair<int, std::string> > mInstructionsComing;
	std::map <std::string, vle::value::Value*> mVariables;
	double ta;
	std::vector <vd::ExternalEvent*> mNextExternalEvent;
	
	lua_State *L;
    ControleurProxy mCP;
    
    void addVar(std::string varName, vle::value::Value* varValue);
    
    /**
     * @brief Parse the script
     */
	void parseScript();
    
    void parseOneLine(std::string line);
    
    void atRegister(double time, std::string instruction);
    
    std::string addOneModel(std::string className);
    
    std::map <std::string, vv::Value*> parseWITH(std::string line);
    
    /**
     * @brief Find a name starting with className_ and a number doesn't used
     *
     * @param std::string className
     *
     * @return std::string
     */
    std::string findModelName(std::string className);
    
    /**
     * @brief Connect the model to the executive
     *
     * @param std::string modelName
     * @param const vpz::BaseModel* model
     */
    void connectionModelToExec(std::string modelName, const vpz::BaseModel* model);
    
    /**
     * @brief Connect the executive with the model
     *
     * @param std::string the model name
     */
    void connectionExecToModel(std::string modelName);
    /**
     * @brief Delete models
     *
     * @param std::vector<std::string> list of the model name to remove
     */
    void delInstruction(std::vector<std::string> words);
    
    
    
    /**
     * @brief Remove the input port of the executive associated to the model
     *
     * @param std::string model name
     */
    void removeInputPortExec(std::string modelName);
    
    /**
     * @brief Remove the output port of the executive associated to pertub port of modelName
     *
     * @param std::string
     */
    void removeOutputPortExec(std::string modelName);
    
    void putInStructure(std::string modelName, std::string variable, vle::value::Value* value);
    
    void showData();
    
    void showDataLua();
    
    void showInstructionListWaiting();
    
    std::string getModelNameFromPort(std::string s);
    
    void updateData(const vd::ExternalEventList& events);
    
    std::map <std::string, vv::Value*> modifyParameter(std::string className, std::map <std::string, vv::Value*> variableToModify);
    
    bool compareModelClass(std::string modelName, std::string className);
    
    std::map <std::string, std::map <std::string, vle::value::Value*> >::iterator getItFromData(std::string className, int n);
    
    struct CompareTime {
       bool operator()( const std::pair<int, std::string> a, const std::pair<int, std::string>  b ) const {
          return a.first > b.first;
       }
    };
};

}}}} // namespace vle gvle global ibminsidegvle
#endif
