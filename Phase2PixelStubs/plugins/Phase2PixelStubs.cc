// -*- C++ -*-
//
// Package:    Phase2PixelSim/Phase2PixelStubs
// Class:      Phase2PixelStubs
// 
/**\class Phase2PixelStubs Phase2PixelStubs.cc Phase2PixelSim/Phase2PixelStubs/plugins/Phase2PixelStubs.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  andres abreu nazario
//         Created:  Thu, 06 Jul 2017 21:24:55 GMT



// system include files
#include <memory>
#include <iostream>

// user include files
#include "CondFormats/HcalObjects/interface/HcalPedestals.h"

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Framework/interface/EDFilter.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Utilities/interface/typelookup.h"

#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"

#include "DataFormats/L1TrackTrigger/interface/TTStub.h"
#include "DataFormats/L1TrackTrigger/interface/TTTypes.h"
#include "DataFormats/L1TrackTrigger/interface/TTTrack.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Common/interface/View.h"
#include "DataFormats/SiStripDetId/interface/StripSubdetector.h"
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "SimDataFormats/TrackingAnalysis/interface/TrackingParticle.h"
#include "SimDataFormats/TrackingAnalysis/interface/TrackingVertex.h"

#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/CommonDetUnit/interface/GeomDet.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/TrackerGeometryBuilder/interface/StripGeomDetUnit.h"
#include "Geometry/Records/interface/StackedTrackerGeometryRecord.h"
//#include "Geometry/TrackerGeometryBuilder/interface/StackedTrackerGeometry.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "SimTracker/TrackTriggerAssociation/interface/TTStubAssociationMap.h"

#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

//root
#include "TLorentzVector.h"
#include "TH1D.h"
#include "TCanvas.h"
#include <TTree.h>

//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<> and also remove the line from
// constructor "usesResource("TFileService");"
// This will improve performance in multithreaded jobs.

typedef edm::Ref< edm::DetSetVector< Phase2TrackerDigi >, Phase2TrackerDigi > Ref_Phase2TrackerDigi_;

class Phase2PixelStubs : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
   public:
      explicit Phase2PixelStubs(const edm::ParameterSet&);
      ~Phase2PixelStubs();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


   private:
      virtual void beginJob() override;
      virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
      virtual void endJob() override;

  // ----------member data ---------------------------
  edm::InputTag stubSrc_;
  edm::EDGetTokenT<edmNew::DetSetVector< TTStub< Ref_Phase2TrackerDigi_ > > >  StubTok_;

  edm::InputTag TrackingParticleSrc_;
  edm::EDGetTokenT< std::vector< TrackingParticle > > TrackingParticleTok_;

  edm::InputTag TrackingVertexSrc_;
  edm::EDGetTokenT< std::vector< TrackingVertex > > TrackingVertexTok_;

  edm::InputTag generalTracksSrc_;
  edm::EDGetTokenT< std::vector< reco::Track > > GeneralTracksTok_;

  // ----------Tree and branches for mini-tuple ---------------------------
  TTree* eventTree;

  std::vector<float>* stub_pt;        // pt
  std::vector<float>* stub_eta;       // eta
  std::vector<float>* stub_phi;       // phi
  std::vector<float>* trig_bend;
  std::vector<int>*   nstub; //number of stubs per event
  std::vector<float>* gentracks_eta;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//-------------------------------------------------------------------------------------------------
// constructors and destructor
//------------------------------------------------------------------------------------------------- 
Phase2PixelStubs::Phase2PixelStubs(const edm::ParameterSet& iConfig)
{
  stubSrc_ = iConfig.getParameter<edm::InputTag>("TTStubs");
  StubTok_ = consumes<edmNew::DetSetVector< TTStub< Ref_Phase2TrackerDigi_ > > >(stubSrc_);

  TrackingParticleSrc_ = iConfig.getParameter<edm::InputTag>("TrackingParticleInputTag");
  TrackingParticleTok_ = consumes< std::vector< TrackingParticle > >(TrackingParticleSrc_);

  TrackingVertexSrc_ = iConfig.getParameter<edm::InputTag>("TrackingVertexInputTag");
  TrackingVertexTok_ = consumes< std::vector< TrackingVertex > >(TrackingVertexSrc_);

  generalTracksSrc_ = iConfig.getParameter<edm::InputTag>("GeneralTracksInputTag");
  GeneralTracksTok_ = consumes< std::vector< reco::Track > >(generalTracksSrc_);
}

Phase2PixelStubs::~Phase2PixelStubs()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called for each event  ------------
void
Phase2PixelStubs::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{

  stub_pt->clear();
  stub_eta->clear();
  stub_phi->clear();
  trig_bend->clear();
  nstub->clear();

  edm::Handle< edmNew::DetSetVector< TTStub< Ref_Phase2TrackerDigi_ > > > Phase2TrackerDigiTTStubHandle;
  iEvent.getByToken(StubTok_, Phase2TrackerDigiTTStubHandle);
  edm::Handle< TTStubAssociationMap< Ref_Phase2TrackerDigi_ > > MCTruthTTStubHandle;

  //general tracks
  edm::Handle< std::vector< reco::Track > > GeneralTracksHandle;
  iEvent.getByToken(GeneralTracksTok_, GeneralTracksHandle);

  //tracking particles 
  edm::Handle< std::vector< TrackingParticle > > TrackingParticleHandle;
  edm::Handle< std::vector< TrackingVertex > > TrackingVertexHandle;
  iEvent.getByToken(TrackingParticleTok_, TrackingParticleHandle);
  iEvent.getByToken(TrackingVertexTok_, TrackingVertexHandle);

  edm::ESHandle< TrackerGeometry > tGeomHandle;
  iSetup.get< TrackerDigiGeometryRecord >().get(tGeomHandle);
  const TrackerGeometry* const theTrackerGeometry = tGeomHandle.product();

  /// Loop over input Stubs
  typename edmNew::DetSetVector< TTStub< Ref_Phase2TrackerDigi_ > >::const_iterator inputIter;
  typename edmNew::DetSet< TTStub< Ref_Phase2TrackerDigi_ > >::const_iterator contentIter;
  //Adding protection
  if ( !Phase2TrackerDigiTTStubHandle.isValid() )  return;

  TH1* h1 = new TH1I("h1", "Number of Stubs", 11, 0.0, 11.0);
  TH1* h2 = new TH1I("h2", "Number of Stubs", 3, 0.0, 3.0);

  int temp, temp1 = 0;//, counter = 0;
  std::vector<int> stubPerEvent, Nstubs;

  // ----------------------------------------------------------------------------------------------
  // loop over L1 stubs
  // ----------------------------------------------------------------------------------------------

  for ( inputIter = Phase2TrackerDigiTTStubHandle->begin();
        inputIter != Phase2TrackerDigiTTStubHandle->end();
        ++inputIter )
    {  
      temp = 0;
      for ( contentIter = inputIter->begin(); contentIter != inputIter->end(); ++contentIter )
      {
        /// Make reference stub
	edm::Ref< edmNew::DetSetVector< TTStub< Ref_Phase2TrackerDigi_ > >, TTStub< Ref_Phase2TrackerDigi_ > > tempStubRef = edmNew::makeRefTo( Phase2TrackerDigiTTStubHandle, contentIter );

	/// Get det ID (place of the stub)
	//  tempStubRef->getDetId() gives the stackDetId, not rawId
	DetId detIdStub = theTrackerGeometry->idToDet( (tempStubRef->getClusterRef(0))->getDetId() )->geographicalId();
	
	/// Define position stub by position inner cluster
	MeasurementPoint mp = (tempStubRef->getClusterRef(0))->findAverageLocalCoordinates();
	const GeomDet* theGeomDet = theTrackerGeometry->idToDet(detIdStub);
	Global3DPoint posStub = theGeomDet->surface().toGlobal( theGeomDet->topology().localPosition(mp) );
	  
	double eta = posStub.eta();
	double phi = posStub.phi();

	float trigBend = tempStubRef->getTriggerBend();

	stub_phi->push_back(phi);		  
	stub_eta->push_back(eta);
	trig_bend->push_back(trigBend);

	temp++;

	temp1 = 0;
	temp1++;
	Nstubs.push_back(temp1); //Actual number of stubs per event
      }
      stubPerEvent.push_back(temp); //more research needed
    }

  // ----------------------------------------------------------------------------------------------                     
  // loop over general tracks                                                                                             
  // ----------------------------------------------------------------------------------------------  

  
  int this_gtrk = 0;
  std::vector< reco::Track >::const_iterator iterGTrk;

  for (iterGTrk = GeneralTracksHandle->begin(); iterGTrk != GeneralTracksHandle->end(); ++iterGTrk) {
    
    //edm::Ptr< reco::Track > gtrk_ptr(GeneralTracksHandle, this_gtrk);                                                  
    //this_gtrk++;

    float tmp_gtrk_eta = iterGTrk->outerEta();
    gentracks_eta->push_back(tmp_gtrk_eta);
  }
  /*
  for(unsigned int i = 0; i < GeneralTracksHandle->size(); ++i) {
    
    //float tmp_gtrk_eta = GeneralTracksHandle->at(i).Eta();                                                                                  
    //gentracks_eta->push_back(tmp_gtrk_eta); 
  }
  */
  // ----------------------------------------------------------------------------------------------
  // loop over tracking particles
  // ----------------------------------------------------------------------------------------------
  /*
  int this_tp = 0;
  std::vector< TrackingParticle >::const_iterator iterTP;

  for (iterTP = TrackingParticleHandle->begin(); iterTP != TrackingParticleHandle->end(); ++iterTP) {
 
    edm::Ptr< TrackingParticle > tp_ptr(TrackingParticleHandle, this_tp);
    this_tp++;

    float track_pt = iterTP->pt();
    //if (track_pt < 2.0) continue;

    stub_pt->push_back(track_pt);
  }
  */

  //int vecSize = stubPerEvent.size();
  int vecSize2 = Nstubs.size();
  
  //For loops for local histogram creation (not on TTree)
  int i = 0;
  for ( inputIter = Phase2TrackerDigiTTStubHandle->begin();
        inputIter != Phase2TrackerDigiTTStubHandle->end();
        ++inputIter )
    {
      for (int j = 1; j < 10; j++) {
      if (stubPerEvent[i] == j)                                                                       
        h1->Fill(stubPerEvent[i],j);
      } 
      i++;
      //nstub->push_back(vecSize);
    }

  for (int k = 0; k <= vecSize2; k++)
    h2->Fill(Nstubs[k],1);
  
  nstub->push_back(vecSize2); //fill stub per event in TTree
  //std::string intstr = std::to_string(vecSize);
  std::string intstr = std::to_string(vecSize2);
  TString name = intstr + "NStubs.pdf";
  TCanvas* hcanvas = new TCanvas("hcanvas","Canvas 1",100,100,800,800);
  h2->Draw("HIST");
  //gPad->SetLogy();
  hcanvas->SaveAs("../plugins/plots/"+name);
  delete hcanvas;

  eventTree->Fill();
}



// ------------ method called once each job just before starting event loop  ------------
void 
Phase2PixelStubs::beginJob()
{
  edm::Service<TFileService> fs;

  stub_pt = new std::vector<float>;
  stub_eta = new std::vector<float>;    
  stub_phi = new std::vector<float>;    
  trig_bend = new std::vector<float>;
  gentracks_eta = new std::vector<float>;
  nstub = new std::vector<int>;

  // ntuple
  eventTree = fs->make<TTree>("eventTree", "Event tree");

  eventTree->Branch("stub_pt",     &stub_pt);
  eventTree->Branch("stub_eta",    &stub_eta);
  eventTree->Branch("stub_phi",    &stub_phi);
  eventTree->Branch("trig_bend",   &trig_bend);
  eventTree->Branch("nstub",     &nstub);
  eventTree->Branch("gentracks_eta",     &gentracks_eta);
}

// ------------ method called once each job just after ending the event loop  ------------
void 
Phase2PixelStubs::endJob() 
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
Phase2PixelStubs::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(Phase2PixelStubs);
