/**
  * @file M.cpp
  * @author ...
  * ...
  *@@tagdynamic@@
  *@@tagdepends:vle.extension.differential-equation@@endtagdepends
  * @@tag Forrester@vle.forrester @@namespace:IbminsideGVLE;class:M;400|400;c:C1,60,95,90,25|C3,69,260,90,25|DSAT,292,282,90,25|C2,66,172,90,25|;f:F1,238,143,50,20,C1 * p1,false,,,|F2,243,214,50,20,C2 * DSAT,false,,,|;fa:C1,F1,5,2|F1,C2,3,5|C2,F2,5,2|F2,C3,3,5|;p:p1,321,66,30,16|;pa:p1,F1,3,0,284,98|DSAT,F2,0,5,304,258|C1,F1,5,0,206,119|C2,F2,5,0,212,193|;ev:;v:;@@end tag@@
  */

#include <vle/extension/differential-equation/DifferentialEquation.hpp>

//@@begin:include@@

//@@end:include@@
namespace vd = vle::devs;
namespace ve = vle::extension::differential_equation;
namespace vv = vle::value;

namespace IbminsideGVLE {

class M : public ve::DifferentialEquation
{
public:
    M(
       const vd::DynamicsInit& init,
       const vd::InitEventList& events)
    : ve::DifferentialEquation(init, events)
    {
        C1 = createVar("C1");
        C3 = createVar("C3");
        DSAT = createVar("DSAT");
        C2 = createVar("C2");


        if (events.exist("p1"))
            p1 = events.getDouble("p1");
        else
            throw vle::utils::ModellingError("Parameter p1 not found");

    }

    virtual void compute(const vd::Time& t)
    {
        F1 = C1() * p1;
        F2 = C2() * DSAT();
        grad(C1) = 0 - (F1);
        grad(C3) = (F2);
        grad(DSAT) = 0;
        grad(C2) = (F1) - (F2);
    }

    virtual ~M()
    { }

private:
    Var C1;
    Var C3;
    Var DSAT;
    Var C2;

    double p1;
    double F1;
    double F2;
virtual vv::Value* observation(
    const vd::ObservationEvent& event) const
{
   const std::string& port = event.getPortName();
    if (port == "F1" ) return new vv::Double(F1);
    if (port == "F2" ) return new vv::Double(F2);
   return ve::DifferentialEquation::observation(event);
}
};

} // namespace IbminsideGVLE

DECLARE_DYNAMICS(IbminsideGVLE::M)

