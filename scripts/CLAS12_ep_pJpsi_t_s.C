//to run
// clas12-elSpectro --ebeam 10.6 --seed 2132 --trig 10 --misc '--nucleon=2212  --tslope=4 --flat=1 --muons' CLAS12_ep_pJpsi_t_s.C
//--trig => nevents
//--jpac => model selection, none by default, 0=> pomeron exchange , 1=> + 1P*, 2=> +2 P*, 3=>2019 3 Pentaquarks
//--muons => decay to muons not electrons (if ommitted will decay to electrons)
//--tslope => give t distribution slope
//--flat => give relative amount of flat production angle compared to t distribution
#include "amplitudes/vector_exchange.hpp"
#include "amplitudes/pseudoscalar_exchange.hpp"
#include "amplitudes/pomeron_exchange.hpp"
#include "amplitudes/amplitude_sum.hpp"
#include "amplitudes/baryon_resonance.hpp"
jpacPhoto::amplitude_sum* Amplitude(Int_t mode);
 
void CLAS12_ep_pJpsi_t_s(C12Config config) {

  config.Print();
  Float_t tslope=5.; //tslope
  Float_t flat = 0.; //relative amount of flat CM production angle
  Int_t jpacModel = -1; //default no jpac model
  Bool_t decayToMuons=kFALSE;
  
  //Do some misc string decoding
  //example string $nucleon=2212 $tslope=4 $flat=1 $muons
  auto tokens=config._misc.Tokenize("$");
  for(auto entry:*tokens) {
    TString sentry= entry->GetName();///get actual string
     if(sentry.Contains("tslope=")){ //slope
      TString tstring= sentry;
      tstring.ReplaceAll("tslope=","");
      tslope =  tstring.Atof();
    }
    if(sentry.Contains("flat=")){//relative amount of flat to t
      TString fstring= sentry;
      fstring.ReplaceAll("flat=","");
      flat =  fstring.Atof();
    }
    if(sentry.Contains("jpac=")){// use jpac amp 
      TString fstring= sentry;
      fstring.ReplaceAll("jpac=","");
      jpacModel =  fstring.Atoi();
    }
    if(sentry.Contains("muons")){//Jpsi decay to muons
      decayToMuons=kTRUE;
    }
  }
  
  auto ebeamP=config._beamP;

  //define e- beam, pdg =11 momentum = _beamP
  auto elBeam = initial(11,ebeamP);
  auto elin=elBeam->GetInteracting4Vector();

  //proton target at rest
  auto prTarget= initial(2212,0);
  auto prin=prTarget->GetInteracting4Vector();

  //produced jpsi decaying to e- e+
  Int_t leptonPDG=11;
  if(decayToMuons) leptonPDG=13;
  auto jpsi=particle(443,model(new PhaseSpaceDecay({},{leptonPDG,-leptonPDG})));
  
  jpacPhoto::amplitude_sum *jpac_amp=nullptr;
  if(jpacModel<0){
    //decay of gamma* + N  to N' + Jpsi
    //depends on s and t
    auto pGammaStarDecay = static_cast<DecayModelst*>(model(new DecayModelst{ {jpsi},{2212} }));
    mesonex( elBeam,prTarget ,
	     new DecayModelQ2W{0, pGammaStarDecay,new TwoBody_stu{flat,1,tslope,0,0}});

  }
  else{

    jpac_amp = Amplitude(jpacModel);
    auto pGammaStarDecay = new JpacModelst{jpac_amp, {jpsi},{2212} };
    flat=10;
    tslope=1;
    mesonex( elBeam,prTarget ,
	     new DecayModelQ2W{0, pGammaStarDecay,new TwoBody_stu{flat,1,tslope,0,0}});
    
  }

    

  c12process(config);

  if(jpac_amp!=nullptr)delete jpac_amp;
}
jpacPhoto::amplitude_sum* Amplitude(Int_t mode){
  using namespace jpacPhoto;

  reaction_kinematics * ptr = new reaction_kinematics(3.0969160);//Jpsi mass
  ptr->set_JP(1, -1);
  
  // ---------------------------------------------------------------------------
  // S - CHANNEL

  // Two different pentaquarks
  // masses and widths from 2015 LHCb paper [2]
  auto P_c4450 =new baryon_resonance(ptr, 3, -1, 4.45, 0.040, "P_{c}(4450)");
  P_c4450->set_params({0.01, .7071}); // 2% branching fraction and equal photocouplings

  auto P_c4380 = new baryon_resonance(ptr, 5, +1, 4.38, 0.205, "P_{c}(4380)");
  P_c4380->set_params({0.01, .7071}); // 2% branching fraction and equal photocouplings

  // ---------------------------------------------------------------------------
  // T - CHANNEL

  // Set up pomeron trajectory
  // Best fit values from [1]
   auto alpha =new linear_trajectory(+1, 0.941, 0.364, "pomeron");

  // Create amplitude with kinematics and trajectory
  auto  background = new pomeron_exchange(ptr, alpha, false, "Background");

  // normalization and t-slope
  background->set_params({0.379, 0.12});

  // ---------------------------------------------------------------------------
  // SUM
  // ---------------------------------------------------------------------------
  // Incoherent sum of the s and t channels
  if(mode==0)
    return new amplitude_sum(ptr, {background}, "5q Sum");
  else if(mode==1)
    return new amplitude_sum(ptr, {background, P_c4450}, "5q Sum");
  else if(mode==3){
    auto P_c4312 =new baryon_resonance(ptr, 1, -1, 4.312, 0.0098, "P_{c}(4312)");
    P_c4312->set_params({0.01, .7071}); // 2% branching fraction and equal photocouplings
    auto P_c4440 =new baryon_resonance(ptr, 3, -1, 4.440, 0.021, "P_{c}(4440)");
    P_c4440->set_params({0.01, .7071}); // 2% branching fraction and equal photocouplings
    auto P_c4457 =new baryon_resonance(ptr, 5, -1, 4.457, 0.0064, "P_{c}(4457)");
    P_c4457->set_params({0.01, .7071}); // 2% branching fraction and equal photocouplings

    return new amplitude_sum(ptr, {background, P_c4312, P_c4440, P_c4457}, "5q Sum");
  }
   else {
    return new amplitude_sum(ptr, {background, P_c4450, P_c4380}, "10q Sum");
  }
  
}
