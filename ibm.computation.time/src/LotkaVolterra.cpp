/**
  * @file LotkaVolterra.cpp
  * @author ...
  * ...
  *@@tagdynamic@@
  *@@tagdepends:vle.extension.differential-equation@@endtagdepends
  * @@tag Forrester@vle.forrester @@namespace:ibmComputationTime;class:LotkaVolterra;406|400;c:X,153,144,90,25|Y,164,330,90,25|;f:XGrowth,23,170,63,20,alpha * X,false,,,|XDeath,309,171,57,20,beta * X * Y,false,,,|YDeath,339,357,57,20,delta * Y,false,,,|YGrowth,20,359,63,20,gamma * X * Y,false,,,|;fa:X,XDeath,5,0|XGrowth,X,0,4|Y,YDeath,5,0|YGrowth,Y,0,4|;p:gamma,246,260,30,16|beta,161,260,30,16|delta,309,261,30,16|alpha,84,262,30,16|;pa:alpha,XGrowth,1,6,59,224|alpha,XDeath,1,2,165,196|beta,XDeath,1,3,177,220|X,XGrowth,0,1,93,127|X,XDeath,1,1,296,126|delta,YDeath,0,1,322,314|Y,YDeath,3,3,291,378|Y,YGrowth,2,5,140,380|gamma,YGrowth,0,1,182,302|Y,XDeath,3,6,423,479|X,YGrowth,0,1,51,86|;ev:;v:;@@end tag@@
  */

#include <vle/extension/differential-equation/DifferentialEquation.hpp>

//@@begin:include@@

//@@end:include@@
namespace vd = vle::devs;
namespace ve = vle::extension::differential_equation;
namespace vv = vle::value;

namespace ibmComputationTime {

class LotkaVolterra : public ve::DifferentialEquation
{
public:
    LotkaVolterra(
       const vd::DynamicsInit& init,
       const vd::InitEventList& events)
    : ve::DifferentialEquation(init, events)
    {
        X = createVar("X");
        Y = createVar("Y");


        if (events.exist("gamma"))
            gamma = events.getDouble("gamma");
        else
            throw vle::utils::ModellingError("Parameter gamma not found");

        if (events.exist("beta"))
            beta = events.getDouble("beta");
        else
            throw vle::utils::ModellingError("Parameter beta not found");

        if (events.exist("delta"))
            delta = events.getDouble("delta");
        else
            throw vle::utils::ModellingError("Parameter delta not found");

        if (events.exist("alpha"))
            alpha = events.getDouble("alpha");
        else
            throw vle::utils::ModellingError("Parameter alpha not found");

    }

    virtual void compute(const vd::Time& t)
    {
        XGrowth = alpha * X();
        XDeath = beta * X() * Y();
        YDeath = delta * Y();
        YGrowth = gamma * X() * Y();
        grad(X) = (XGrowth) - (XDeath);
        grad(Y) = (YGrowth) - (YDeath);
    }

    virtual ~LotkaVolterra()
    { }

private:
    Var X;
    Var Y;

    double gamma;
    double beta;
    double delta;
    double alpha;
    double XGrowth;
    double XDeath;
    double YDeath;
    double YGrowth;
virtual vv::Value* observation(
    const vd::ObservationEvent& event) const
{
   const std::string& port = event.getPortName();
    if (port == "XGrowth" ) return new vv::Double(XGrowth);
    if (port == "XDeath" ) return new vv::Double(XDeath);
    if (port == "YDeath" ) return new vv::Double(YDeath);
    if (port == "YGrowth" ) return new vv::Double(YGrowth);
   return ve::DifferentialEquation::observation(event);
}
};

} // namespace ibmComputationTime

DECLARE_DYNAMICS(ibmComputationTime::LotkaVolterra)

