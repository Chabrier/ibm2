/*
 * @file Controleur.cpp
 *
 */

#include <vle/devs/Executive.hpp>
#include <vle/vpz/Conditions.hpp>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream> //Pour dump
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <vle/vpz/ModelPortList.hpp>

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
    : vd::Executive(mdl, events), mEvents(events)
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

    virtual vd::Time init(const vd::Time& /* time */)
    {
        return 0.0;
    }
    
    /*virtual void internalTransition(const devs::Time& time) const
    {
        std::cout << "Doing dump" << std::endl;
        std::ofstream file("/home/gcicera/outputTestHahaha.vpz");
        dump(file, "dumpHahaha");
    }*/


    //void externalTransition(const vd::ExternalEventList& events,
      //                      const vd::Time& /* time */)
    //{
    //}

    virtual vd::Time timeAdvance() const
    {
        return vd::infinity;
    }

    //virtual void output(const vd::Time& /* time */,
     //                   vd::ExternalEventList& output) const
    //{
    //}

	//virtual vv::Value* observation(
	//	const vd::ObservationEvent& event) const
	//{
	//		return new vv::Double(0);
	//}


private:
    const vd::InitEventList& mEvents;
	std::string mScript;
    
    /**
     * @brief Parse the script
     */
	void parseScript() {
        std::vector<std::string> lines;
        boost::split(lines, mScript, boost::is_any_of("\n"));
        
        for (unsigned int i=0; i<lines.size(); i++) {
            std::cout << "enum " << lines[i] << std::endl;
            std::vector<std::string> words;
            boost::split(words, lines[i], boost::is_any_of(" "));
            boost::to_upper(words[0]);
            if (words[0] == "ADD")
            {
                try {
                    int nb_clone = readNumber(words[1]);
                    addInstruction(nb_clone, words[2]);
                } catch(boost::bad_lexical_cast const&) {
                    throw utils::ArgError("Error: input string was not valid");
                } catch(utils::DevsGraphError) {
                    throw utils::ArgError("The model doesn't exist");
                } catch(const char* Message) {
		            throw utils::ArgError(Message);
	            }
            } else if (words[0] == "DEL") {
                delInstruction(words);
            } else if (words[0] != ""){
                throw utils::ArgError("Directive not found");
            }
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
                throw "Variable not found";
        } else {
            nb_clone = boost::lexical_cast<int>(nb);
            if (nb_clone < 0) throw "No positive number";
        }
        return nb_clone;
    }
    
    /**
     * @brief Create nb_clone modelwith the className Class
     * 
     * @param int nb_clone
     * @param std::string className
     */
    void addInstruction(int nb_clone, std::string className) {
        for (int i=0; i<nb_clone; i++){
            std::string modelName = findModelName(className);
			const vpz::BaseModel* newModel = createModelFromClass(className, modelName);
			connectionModelToExec(modelName, newModel);
			connectionExecToModel(modelName);
		}
    }
    
    /**
     * @brief Find a name starting with className_ and a number doesn't used
     *
     * @param std::string className
     *
     * @return std::string
     */
    std::string findModelName(std::string className) {
        int i=0;
        std::string modelName = className + "_";
        std::string name = ""; 
        do {
		    std::stringstream ss;
		    ss << i;
		    name = modelName + ss.str();
		    i++;
		} while(coupledmodel().exist(name));
		return name;
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
            try {
                removeInputPortExec(words[i]);
                removeOutputPortExec(words[i]);
                delModel(words[i]);
            } catch (utils::DevsGraphError) {
                throw utils::ArgError("Model to remove not found");
            }
        }
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
};

}}}} // namespace vle gvle global ibminsidegvle

DECLARE_EXECUTIVE(vle::gvle::global::ibminsidegvle::Controleur)
