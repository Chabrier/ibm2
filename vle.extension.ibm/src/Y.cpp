/**
  * @file Y.cpp
  * @author ...
  * ...
  *@@tagdynamic@@
  *@@tagdepends:vle.extension.differential-equation@@endtagdepends
  * @@tag Forrester@vle.forrester @@namespace:IbminsideGVLE;class:Y;421|400;c:age,47,71,90,25|X,57,147,90,25|KG,272,71,90,25|maturing,321,214,90,25|masse,59,220,90,25|;f:f,194,89,50,20,maturing,false,,,|Growth,214,174,56,20,KG * X *maturing,false,,,|Senescence,210,271,79,20,masse * KS *  ( 1 - maturing ) ,false,,,|;fa:X,Growth,5,2|Growth,masse,3,5|masse,Senescence,3,0|Senescence,X,1,5|f,age,3,5|;p:KS,334,326,30,16|;pa:KG,Growth,2,0,268,126|KS,Senescence,3,5,304,308|X,Growth,5,0,194,160|masse,Senescence,3,2,168,256|maturing,Growth,4,4,295,200|maturing,Senescence,4,4,305,249|maturing,f,0,5,294,161|;ev:;v:;@@end tag@@
  */

#include <vle/extension/differential-equation/DifferentialEquation.hpp>

//@@begin:include@@

//@@end:include@@
namespace vd = vle::devs;
namespace ve = vle::extension::differential_equation;
namespace vv = vle::value;

namespace IbminsideGVLE {

class Y : public ve::DifferentialEquation
{
public:
    Y(
       const vd::DynamicsInit& init,
       const vd::InitEventList& events)
    : ve::DifferentialEquation(init, events)
    {
        age = createVar("age");
        X = createVar("X");
        KG = createVar("KG");
        maturing = createVar("maturing");
        masse = createVar("masse");


        if (events.exist("KS"))
            KS = events.getDouble("KS");
        else
            throw vle::utils::ModellingError("Parameter KS not found");

    }

    virtual void compute(const vd::Time& t)
    {
        f = maturing();
        Growth = KG() * X() *maturing();
        Senescence = masse() * KS *  ( 1 - maturing() ) ;
        grad(age) = (f);
        grad(X) = (Senescence) - (Growth);
        grad(KG) = 0;
        grad(maturing) = 0;
        grad(masse) = (Growth) - (Senescence);
    }

    virtual ~Y()
    { }

private:
    Var age;
    Var X;
    Var KG;
    Var maturing;
    Var masse;

    double KS;
    double f;
    double Growth;
    double Senescence;
virtual vv::Value* observation(
    const vd::ObservationEvent& event) const
{
   const std::string& port = event.getPortName();
    if (port == "f" ) return new vv::Double(f);
    if (port == "Growth" ) return new vv::Double(Growth);
    if (port == "Senescence" ) return new vv::Double(Senescence);
   return ve::DifferentialEquation::observation(event);
}
};

} // namespace IbminsideGVLE

DECLARE_DYNAMICS(IbminsideGVLE::Y)


