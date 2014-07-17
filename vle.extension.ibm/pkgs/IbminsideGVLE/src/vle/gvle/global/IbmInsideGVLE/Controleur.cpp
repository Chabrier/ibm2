/*
 * @file Controleur.cpp
 *
 */

#include <vle/devs/Executive.hpp>
#include <vle/vpz/Conditions.hpp>
#include <map>
#include <sstream>
#include <iostream>

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
    : vd::Executive(mdl, events)
    {
        mScript = "";

        if (events.exist("Script"))
            mScript = events.getString("Script");
        else
            throw vle::utils::ModellingError("Text mProgram not found");
    }

    virtual ~Controleur()
    {
    }

    virtual vd::Time init(const vd::Time& /* time */)
    {
        return vd::infinity;
    }

    //void externalTransition(const vd::ExternalEventList& events,
         //                   const vd::Time& /* time */)
    /*{
    }*/

    virtual vd::Time timeAdvance() const
    {
        return vd::infinity;
    }

    //virtual void output(const vd::Time& /* time */,
      //                  vd::ExternalEventList& output) const
    //{
    //}

	//virtual vv::Value* observation(
	//	const vd::ObservationEvent& event) const
	//{
	//		return new vv::Double(0);
	//}


private:
	std::string mScript;
};

}}}} // namespace vle gvle global ibminsidegvle

DECLARE_EXECUTIVE(vle::gvle::global::ibminsidegvle::Controleur)
