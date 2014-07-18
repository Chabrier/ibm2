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
                    //Vérifier si words[2] est bien une classe existante, à faire
                    addInstruction(nb_clone, words[2]);
                } catch(boost::bad_lexical_cast const&) {
                    throw utils::ArgError("Error: input string was not valid");
                } catch(const char* Message) {
		            throw utils::ArgError(Message);
	            }
            } else {
                throw utils::ArgError("Directive not found");
            }
        }
    }
    
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
    
    void addInstruction(int nb_clone, std::string className) {
        for (int i=0; i<nb_clone; i++){
            std::string modelName = className + "_";
			std::stringstream ss;
			ss << i;
			modelName = modelName + ss.str();
			const vpz::BaseModel* newModel = createModelFromClass(className, modelName);
			connectionModelToExec(modelName, newModel);
			//FAIRE LA CONNECTION SORTIE DE L'EXEC AUX PORTS PERTURB
			connectionExecToModel(modelName);
		}
    }
    
    void connectionModelToExec(std::string modelName, const vpz::BaseModel* model) {
        //std::map< std::string, ModelPortList > vle::vpz::ConnectionList
        std::map<std::string, vpz::ModelPortList> portList = model->getOutputPortList();
        for(std::map<std::string, vpz::ModelPortList>::iterator it=portList.begin(); it!=portList.end(); ++it){
            std::string outputPortName = it->first;
            std::string inputPort = modelName + "_" + outputPortName;
            addInputPort(getModelName(), inputPort);
            addConnection(modelName, outputPortName, getModelName(), inputPort);
        }
    }
    
    void connectionExecToModel(std::string modelName) {
        std::string outputPortName = modelName + "_toPerturb";
        addOutputPort(getModelName(), outputPortName);
        addConnection(getModelName(), outputPortName, modelName, "perturb");
    }
};

}}}} // namespace vle gvle global ibminsidegvle

DECLARE_EXECUTIVE(vle::gvle::global::ibminsidegvle::Controleur)
