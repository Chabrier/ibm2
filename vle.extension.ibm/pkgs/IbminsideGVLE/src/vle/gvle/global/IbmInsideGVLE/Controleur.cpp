/*
 * @file Controleur.cpp
 *
 */

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
               const vd::InitEventList& events)
    : vd::Executive(mdl, events), mEvents(events), mInternTransition(false), mValueStart(0)
    {
        if (events.exist("Script"))
            mScript = events.getString("Script");
        else
            throw vle::utils::ModellingError("Text Script not found");
        parseScript();
        std::ofstream file("/home/gcicera/outputTestHahaha.vpz");
        dump(file, "dumpHahaha");
    }

    virtual ~Controleur()
    {
    }

    virtual vd::Time init(const vd::Time& t)
    {
        mValueC2 = 0;
        mValueC1 = 0;
        toEmptyName = "R1_0";
        toFillName = "R2_0";
        if (mInstructionsComing.size() > 0)
            ta = mInstructionsComing.back().first - t;
        else ta = vd::infinity;
        mInternTransition = true;
        return ta;
    }
    
    virtual void internalTransition(const devs::Time& t)
    {
        std::cout << "internal transition " << t << std::endl;
        if (mInstructionsComing.size() > 0) {
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
        }
        mInternTransition = true;
        std::ofstream file("/home/gcicera/outputTestHahaha.vpz");
        dump(file, "dumpHahaha");
    }

    void externalTransition(const vd::ExternalEventList& /*events*/,
                            const vd::Time& t)
    {
        //std::cout << "external" << std::endl;
        if (mInstructionsComing.size() > 0) 
            ta = mInstructionsComing.back().first - t;
        else ta = vd::infinity;
        //std::cout << "ext " << events[0]->getPortName() << " " << events.size() << std::endl;
        //updateData(events);
    }

    virtual vd::Time timeAdvance() const
    {
        //if(mInternTransition)
        //std::cout << "timeAdvance " << ta << std::endl;
        return ta;
        //return std::numeric_limits<double>::epsilon();
    }

    virtual void output(const vd::Time& /* time */,
                        vd::ExternalEventList& /*output*/) const
    {/*
        vd::ExternalEvent* e1 = new vd::ExternalEvent(toEmptyName + "_toPerturb");
        e1->putAttribute("name", new vv::String("C1"));
        e1->putAttribute("value", new vv::Double(mValueC1));
        vd::ExternalEvent* e2 = new vd::ExternalEvent(toFillName + "_toPerturb");
        e2->putAttribute("name", new vv::String("C1"));
        e2->putAttribute("value", new vv::Double(mValueC2));
        output.push_back(e1);
        output.push_back(e2);*/
    }

	//virtual vv::Value* observation(
	//	const vd::ObservationEvent& event) const
	//{
	//		return new vv::Double(0);
	//}


private:
    const vd::InitEventList& mEvents;
	std::string mScript;
	std::map <std::string, std::map <std::string, vle::value::Value*> > mData;
	bool mInternTransition;
	double mValueStart;
	double mValueC1;
	double mValueC2;
	std::string toEmptyName;
	std::string toFillName;
	std::map <std::string, int> mNameNumber;
	std::vector <std::pair<int, std::string> > mInstructionsComing;
	//std::map <std::string, vle::value::Value*> mVariables;
	double ta;
    
    /**
     * @brief Parse the script
     */
	void parseScript() {
        std::vector<std::string> lines;
        boost::split(lines, mScript, boost::is_any_of("\n"));
        
        for (unsigned int i = 0; i<lines.size(); i++) {
            parseOneLine(lines[i]);
        }
        showData();
        showInstructionListWaiting();
    }
    
    void parseOneLine(std::string line) {
        std::cout << "enum " << line << std::endl;
        std::vector<std::string> words;
        boost::split(words, line, boost::is_any_of(" "));
        boost::to_upper(words[0]);
        if (words[0] == "ADD")
        {
            try {
                int nb_clone = readNumber(words[1]);
                std::map <std::string, std::string> variableToModify;
                if (boost::contains(line, "WITH"))
                    variableToModify = parseWITH(line);
                addInstruction(nb_clone, words[2], variableToModify);
            } catch(const char* Message) {
	            throw utils::ArgError(Message);
            }
        } else if (words[0] == "DEL") {
            delInstruction(words);
        } else if (words[0].substr(0,1) == "@") {
            std::string time = words[0];
            std::string instruction = line.substr(words[0].size() + 1);
            int time_dec = atoi(time.substr(1).c_str());
            mInstructionsComing.push_back(std::pair<int, std::string> (time_dec, instruction));
            CompareTime compTime;
            std::sort(mInstructionsComing.begin(), mInstructionsComing.end(), compTime);
        } else if (words[0] != ""){
            throw utils::ArgError(fmt("Directive `%1%' not found") % words[0]);
        }
    }
    
    /**
     * @brief Convert nb to int or find the number associated to the variable
     *
     * @param std::string nb
     *
     * @return int 
     */
    int readNumber(std::string nb) {
        int nb_clone;
        if (nb.substr(0,1) == "$") {
            std::string variableName = nb.substr(1);
            if (mEvents.exist(variableName))
                nb_clone = mEvents.getInt(variableName);
            else
                throw utils::ArgError(fmt("Variable `%1%' not found ") % variableName);
        } else {
            nb_clone = boost::lexical_cast<int>(nb);
            if (nb_clone < 0) 
                throw utils::ArgError(fmt("No positive number `%1%'") % nb);
        }
        return nb_clone;
    }
    
    /**
     * @brief Create nb_clone modelwith the className Class
     * 
     * @param int nb_clone
     * @param std::string className
     */
    void addInstruction(int nb_clone, std::string className, std::map <std::string, std::string> variableToModify) {
        for (int i=0; i<nb_clone; i++){
			addOneModel(className, variableToModify);
		}
    }
    
    std::string addOneModel(std::string className, std::map <std::string, std::string> variableToModify) {
        modifyParameter(className, variableToModify);
        std::string modelName = findModelName(className);
		const vpz::BaseModel* newModel = createModelFromClass(className, modelName);
		connectionModelToExec(modelName, newModel);
		connectionExecToModel(modelName);
		
		return modelName;
    }
    
    void modifyParameter(std::string className, std::map <std::string, std::string> variableToModify) 
    {
        vc::Conditions& cl = conditions();
		vc::Condition& c = cl.get("cond_DTE_" + className);
		vv::Value* valueToPut;
		for (std::map <std::string, std::string>::iterator it = variableToModify.begin(); it != variableToModify.end(); ++it) {
		    vv::Value::type typeValue = c.firstValue(it->first).getType();
		    switch (typeValue) {
		        case vv::Value::DOUBLE : {
		            double d = boost::lexical_cast<double>(it->second.c_str());
		            valueToPut = new vv::Double(d);
		            std::cout << "doble " << it->first << " " << it->second << std::endl;
		            break; }
		        case vv::Value::INTEGER :{
		            int i = boost::lexical_cast<int>(it->second.c_str());
		            valueToPut = new vv::Integer(i);
		            std::cout << "integer " << it->first << " " << it->second << std::endl;
		            break;}
		        default :{
		            valueToPut = NULL;
		            std::cout << "default switch" << std::endl;
		            break;}
		    }
		    c.setValueToPort(it->first, *(valueToPut));
		}
    }
    
    std::map <std::string, std::string> parseWITH(std::string line) {
        std::map <std::string, std::string> nameAndValue;
        boost::to_upper(line);
        std::string part = line.substr(line.find("WITH") + 5);
        std::vector<std::string> variablesEqualValue;
        boost::split(variablesEqualValue, part, boost::is_any_of(","));
        for (unsigned int i=0; i<variablesEqualValue.size(); i++) {
            std::vector<std::string> variablesAndValue;
            boost::split(variablesAndValue, variablesEqualValue[i], boost::is_any_of("="));
            std::cout << "pk??? " << variablesAndValue.size() << " " << variablesAndValue[0] << std::endl;
            boost::replace_all(variablesAndValue[0], " ", "");
            boost::replace_all(variablesAndValue[1], " ", "");
            nameAndValue.insert(std::pair<std::string, std::string> (variablesAndValue[0], variablesAndValue[1]));
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
    std::string findModelName(std::string className) {
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
    void connectionModelToExec(std::string modelName, const vpz::BaseModel* model) {
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
    void connectionExecToModel(std::string modelName) {
        std::string outputPortName = modelName + "_toPerturb";
        addOutputPort(getModelName(), outputPortName);
        addConnection(getModelName(), outputPortName, modelName, "perturb");
    }
    
    /**
     * @brief Delete models
     *
     * @param std::vector<std::string> list of the model name to remove
     */
    void delInstruction(std::vector<std::string> words) {
        for (unsigned int i=1; i<words.size(); i++) {
            delOneModel(words[i]);
        }
    }
    
    void delOneModel(std::string modelName) {
        std::cout << "del " << modelName << std::endl;
        delModel(modelName);
        removeInputPortExec(modelName);
        removeOutputPortExec(modelName);
        std::cout << "DELETE " << modelName << std::endl;
        mData.erase(modelName);
    }
    
    /**
     * @brief Remove the input port of the executive associated to the model
     *
     * @param std::string model name
     */
    void removeInputPortExec(std::string modelName) {
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
    void removeOutputPortExec(std::string modelName) {
        removeOutputPort(getModelName(), modelName + "_toPerturb");
    }
    
    void putInStructure(std::string modelName, std::string variable, vle::value::Value* value) {
    /*std::cout << "in struct " << value->toDouble().value() << std::endl;
    std::map <std::string, vle::value::Value*> secondMap;
    std::pair<std::string,std::map <iterator, bool> = mData.insert(std::pair<std::string,std::map <std::string, vle::value::Value*> >(modelName, secondMap));*/
//    if (!bool) {
//    }w
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
    
    void showData() {
        for (std::map <std::string, std::map <std::string, vle::value::Value*> >::iterator it=mData.begin(); it!=mData.end(); ++it) {
            for (std::map <std::string, vle::value::Value*>::iterator it2=it->second.begin(); it2!=it->second.end(); ++it2) {
                std::cout << "clé " << it->first << " variable " << it2->first << " : " << *(it2->second) << std::endl;
            }
        }
    }
    
    void showInstructionListWaiting() {
        for (unsigned int i = 0; i<mInstructionsComing.size(); i++) {
            std::cout << "Instruction waiting " << mInstructionsComing[i].first << " " << mInstructionsComing[i].second << std::endl;
        }
    }
    
    std::string getModelNameFromPort(std::string s) {
        unsigned i = s.find_last_of("_");
        return s.substr(0, i);
    }
    
    void updateData(const vd::ExternalEventList& events) {
        for (unsigned int i=0; i<events.size(); i++) {
            std::string s = events[i]->getPortName();
            std::string variable = events[i]->getAttributes().get("name")->toString().value();
            putInStructure(getModelNameFromPort(s), variable, events[i]->getAttributes().get("value"));
        }
        showData();
    }
    
    struct CompareTime {
       bool operator()( const std::pair<int, std::string> a, const std::pair<int, std::string>  b ) const {
          return a.first > b.first;
       }
    };
};

}}}} // namespace vle gvle global ibminsidegvle

DECLARE_EXECUTIVE_DBG(vle::gvle::global::ibminsidegvle::Controleur)
