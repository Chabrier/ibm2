/**
  * @file M.cpp
  * @author ...
  * ...
  *@@tagdynamic@@
  *@@tagdepends:vle.extension.differential-equation@@endtagdepends
  * @@tag Forrester@vle.forrester @@namespace:vle.extension.ibm;class:M;400|400;c:C1,180,135,90,25|;f:;fa:;p:;pa:;ev:;v:;@@end tag@@
  */

#include <vle/extension/differential-equation/DifferentialEquation.hpp>

//@@begin:include@@

//@@end:include@@
namespace vd = vle::devs;
namespace ve = vle::extension::differential_equation;
namespace vv = vle::value;

namespace vleextensionibm {

class M : public ve::DifferentialEquation
{
public:
    M(
       const vd::DynamicsInit& init,
       const vd::InitEventList& events)
    : ve::DifferentialEquation(init, events)
    {
        C1 = createVar("C1");


    }

    virtual void compute(const vd::Time& t)
    {
        grad(C1) = 0;
    }

    virtual ~M()
    { }

private:
    Var C1;

virtual vv::Value* observation(
    const vd::ObservationEvent& event) const
{
   const std::string& port = event.getPortName();
   return ve::DifferentialEquation::observation(event);
}
};

} // namespace vle.extension.ibm

DECLARE_DYNAMICS(vleextensionibm::M)

