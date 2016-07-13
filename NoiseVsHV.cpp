#include <iostream>
#include "string.h"
#include <TMath.h>
#include "ROOT.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <map>


using namespace std;


struct hvrate {
  int hv;
  double rate;
};



map <string, vector<hvrate> > GetDataMap (string FileName){
  string Line;
  ifstream myFileReader ;
  string RollName;
  int HV;
  double rate;
  stringstream ss;
  
  map < string, vector<hvrate> > myMap;
  
  // prochitame failovete 
  myFileReader.open(FileName.c_str());
  if (myFileReader.is_open()){
    while(getline(myFileReader, Line)){
      //cout << Line << endl;
      ss.str(Line);      // Obekt koito shte razdelq liniqta na chasti (string, int, double)
      ss >> RollName >> HV >> rate; // vzima argumentite ot string Line i gi slaga v promenlivite koito shte izpolzvame
      //cout << RollName << " " << HV << " " << rate << endl; // string Line e razdelen i stoinosti sa po promenlivite
      hvrate hv_rate_pair ;
      hv_rate_pair.hv = HV;
      hv_rate_pair.rate = rate;
      
      if ( myMap.find(RollName) ==  myMap.end() ){
	vector<hvrate> my_hvrate_vector;
	myMap[RollName] = my_hvrate_vector;
      }
      
      myMap.at(RollName).push_back(hv_rate_pair);
      
      
      ss.clear();
      
    }
  }  
  
  return myMap;
  
}

map < string, double > GetWorkingPointForRollNameMap (string FileWorkingPoints) {
  
  ifstream Reader;
  string Line;
  stringstream ss;
  string RollName;
  double HVWP ;
  map <string, double > PointsMap;
  Reader.open(FileWorkingPoints.c_str()); // otvarq faila
  
  if (Reader.is_open()){
    while(getline(Reader, Line) ){  //chete faila za vseki red
      ss.str(Line); //razdelqme reda (Lina ) na dve stoinosti ( RollName i HV)
      ss >> RollName >> HVWP ;
      
      PointsMap [RollName] = HVWP;     // zapylvame kartata       
      ss.clear();
    }
  }
  
  return PointsMap;
  
} 



void StoreChamberHVRateGraphs (map <string, vector<hvrate> >  DataMap,string FileWithGraphs, string CorrelationFile){
  // tazi funkciq zapaza grafikite po failoe
  
  map< string, vector< hvrate > > ::iterator iter;
  
  TFile * rootfile = new TFile(FileWithGraphs.c_str(),"RECREATE");
  
  
  
  
  TF1 * f1 = new TF1("f1","[0]+x*[1]",8800,9800);
  TH1F * CorrelationCoeff = new TH1F ("corcoeff", "corcoeff", 110, 0, 1.1);
  for(iter= DataMap.begin() ; iter!= DataMap.end() ; iter++){
    string myKey= iter->first;
    vector<hvrate> myVector= iter->second;
    //cout << myKey << endl;
    
    // pravim histogramata
    double maxValue =-1 ;
    
    for (int y=0; y< myVector.size(); y++) {
     hvrate Rate_Hv;
     Rate_Hv = myVector.at(y);
     
     double currentValue= Rate_Hv.rate;
     
     if (currentValue > maxValue ){
	  maxValue = currentValue;
     }
     //cout << Rate_Hv.rate << endl;
    }  
    
    
    
    //TCanvas * myCanvas = new TCanvas((myKey+"Canvas").c_str(), "Canvas", 1200, 700);
    int vectorSize= myVector.size();
    double x[vectorSize], y[vectorSize];
    //myCanvas->cd();
    
       
    for (int i=0; i< vectorSize;i++){
      hvrate Hv_Rate;
      Hv_Rate = myVector.at(i);
      //cout << Hv_Rate.hv << " " << Hv_Rate.rate << endl;
      
      x[i]= Hv_Rate.hv;
      y[i]= Hv_Rate.rate;
      
      
    }
    
    TGraph * myGraph = new TGraph (vectorSize,x,y);
    
    string NewName= myKey + ".root" ;
    string Picture= "Pictures/" + myKey + ".jpg";
    
    
    
    myGraph->SetTitle(myKey.c_str());
    myGraph->SetName(myKey.c_str());
    myGraph->GetXaxis()->SetTitle("Voltage (V)");
    myGraph->GetYaxis()->SetTitle("Rate (Hz/cm^{2})");
    myGraph->SetMarkerStyle(kFullStar);
    myGraph->SetMarkerSize(myGraph->GetMarkerSize()*1.5);
    myGraph->Draw("CP");
    //myCanvas->SaveAs(Picture.c_str());    
    rootfile->cd();
    myGraph->Write();
    
    myGraph->Delete("R");
    
    //myCanvas->Delete("R");  
    
  }
  CorrelationCoeff->SaveAs(CorrelationFile.c_str());
  rootfile->Save();
  rootfile->Close("R");
  
}


TGraph * GetGraphFromVector (vector<hvrate> DataVector, string GraphName){
    
  int vectorSize = DataVector.size();
  double x[vectorSize], y[vectorSize];
  
  for (int i=0; i< vectorSize;i++){
      hvrate Hv_Rate;
      Hv_Rate = DataVector.at(i);
      //cout << Hv_Rate.hv << " " << Hv_Rate.rate << endl;
      x[i]= Hv_Rate.hv;
      y[i]= Hv_Rate.rate;
      
      
    }
  TGraph * myGraph = new TGraph (vectorSize,x,y);
  //myGraph->SetPoint();
  return myGraph;
} 

map <string, double> intrinsicNoiseMap (string fileName){
  map <string, double> noiseMap;
  
  TFile * noiseFile = new TFile(fileName.c_str(), "READ");
  TKey * TheKey;
  if (! noiseFile->IsOpen()) return noiseMap; // just skip and return, else process the file
  TIter nextkey(noiseFile->GetListOfKeys());
  while(TheKey = (TKey*)nextkey()){
    TH1F * rollHisto = dynamic_cast<TH1F*>(TheKey->ReadObj());
    string rollName = rollHisto->GetName();
    double noiseValue = rollHisto->GetMean();
    noiseMap[rollName] = noiseValue;
  }
  noiseFile->Close("R");
  noiseFile->Delete();
  
  return noiseMap;
}

void NoiseVsHV (){
  
  ofstream RollsWithHighDifference;  
  RollsWithHighDifference.open("results/RollsWithHighDifference.txt");
  ofstream RollsWithHighDifference_fits;  
  RollsWithHighDifference_fits.open("results/RollsWithHighDifference_fromfits.txt");
  
  map <string, vector<hvrate> > map2011 = GetDataMap("data/noiseScan11.txt");
  map <string, vector<hvrate> > map2015 = GetDataMap("data/noiseScan15.txt");
  
  //StoreChamberHVRateGraphs(map2011, " results/GraphFile2011.root", "results/CorrCoef2011.root");
  //StoreChamberHVRateGraphs(map2015, "results/GraphFile2015.root", "results/CorrCoef2015.root");
  
  map <string, double> WP11 = GetWorkingPointForRollNameMap("data/WP2011_applied.txt");
  map <string, double> WP12 = GetWorkingPointForRollNameMap("data/WP2016_applied.txt");
  
  map<string, double>::iterator iter;
  
  //map <string, double> intrinsicNoiseMap_11 = intrinsicNoiseMap("intrinsic11.root");
  //map <string, double> intrinsicNoiseMap_15 = intrinsicNoiseMap("intrinsic15.root");
  
  TH1F * Histos = new TH1F ("RateDifference2015_2011", "#DeltaR = Rate(2015) - Rate(2011)                          All RPC", 100, -2,2);
  TH1F * Histo_fits = new TH1F ("RateDifference2015_2011_fits", "#DeltaR = Rate(2015) - Rate(2011)                          All RPC", 100, -2,2);
  Histos->SetStats(111111);
  
  string GroupedHisFile = "results/GroupedHisFile.root";
  string fits11 = "results/Graphs11_withFits.root";
  string fits15 = "results/Graphs15_withFits.root";
  
  TFile * rootfile = new TFile(GroupedHisFile.c_str(),"RECREATE");
  TFile * graphs11_withFits = new TFile(fits11.c_str(),"RECREATE");
  TFile * graphs15_withFits = new TFile(fits15.c_str(),"RECREATE");
  
  TH1F * His_Barrel = new TH1F ("RateDifference2011_2015_Barrel","#DeltaR = Rate(2015) - Rate(2011)                          All Barrel Rolls", 100, -2,2);
  TH1F * His_EndCap = new TH1F ("RateDifference2011_2015_EndCap","#DeltaR = Rate(2015) - Rate(2011)                          All EndCap Rolls", 100, -2,2);
  TH1F * His_ECp_R2_A = new TH1F ("RateDifference2011_2015_ECp_R2_A","#DeltaR = Rate(2015) - Rate(2011)                          Endcap+ RE*R2_A", 100, -2,2);
  TH1F * His_ECp_R2_B= new TH1F ("RateDifference2011_2015_ECp_R2_B","#DeltaR = Rate(2015) - Rate(2011)                          Endcap+ RE*R2_B", 100, -2,2);
  TH1F * His_ECp_R2_C= new TH1F ("RateDifference2011_2015_ECp_R2_C","#DeltaR = Rate(2015) - Rate(2011)                          Endcap+ RE*R2_C", 100, -2,2);
  TH1F * His_ECp_R3_A= new TH1F ("RateDifference2011_2015_ECp_R3_A","#DeltaR = Rate(2015) - Rate(2011)                          Endcap+ RE*R3_A", 100, -2,2);
  TH1F * His_ECp_R3_B= new TH1F ("RateDifference2011_2015_ECp_R3_B","#DeltaR = Rate(2015) - Rate(2011)                          Endcap+ RE*R3_B", 100, -2,2);
  TH1F * His_ECp_R3_C= new TH1F ("RateDifference2011_2015_ECp_R3_C","#DeltaR = Rate(2015) - Rate(2011)                          Endcap+ RE*R3_C", 100, -2,2);
  TH1F * His_ECm_R2_A = new TH1F ("RateDifference2011_2015_ECm_R2_A","#DeltaR = Rate(2015) - Rate(2011)                           Endcap- RE*R2_A", 100, -2,2);
  TH1F * His_ECm_R2_B= new TH1F ("RateDifference2011_2015_ECm_R2_B","#DeltaR = Rate(2015) - Rate(2011)                           Endcap- RE*R2_B", 100, -2,2);
  TH1F * His_ECm_R2_C= new TH1F ("RateDifference2011_2015_ECm_R2_C","#DeltaR = Rate(2015) - Rate(2011)                           Endcap- RE*R2_C", 100, -2,2);
  TH1F * His_ECm_R3_A= new TH1F ("RateDifference2011_2015_ECm_R3_A","#DeltaR = Rate(2015) - Rate(2011)                           Endcap- RE*R3_A", 100, -2,2);
  TH1F * His_ECm_R3_B= new TH1F ("RateDifference2011_2015_ECm_R3_B","#DeltaR = Rate(2015) - Rate(2011)                           Endcap- RE*R3_B", 100, -2,2);
  TH1F * His_ECm_R3_C= new TH1F ("RateDifference2011_2015_ECm_R3_C","#DeltaR = Rate(2015) - Rate(2011)                           Endcap- RE*R3_C", 100, -2,2);
  
  TH1F * His_RB1= new TH1F ("RateDifference2011_2015_RB1","#DeltaR = Rate(2015) - Rate(2011)                          Barrel station RB1", 100, -2,2);
  TH1F * His_RB2= new TH1F ("RateDifference2011_2015_RB2","#DeltaR = Rate(2015) - Rate(2011)                          Barrel station RB2", 100, -2,2);
  TH1F * His_RB3= new TH1F ("RateDifference2011_2015_RB3","#DeltaR = Rate(2015) - Rate(2011)                          Barrel station RB3", 100, -2,2);
  TH1F * His_RB4= new TH1F ("RateDifference2011_2015_RB4","#DeltaR = Rate(2015) - Rate(2011)                          Barrel station RB4", 100, -2,2);
  
  TH1F * WP_Distribution_Before = new TH1F("before","WP11",100,0,2);
  TH1F * WP_Distribution_Now = new TH1F("now","WP12",100,0,2);
  
  vector <TH1F*> GroupedHis;
  GroupedHis.push_back(His_Barrel);
  GroupedHis.push_back(His_EndCap);
  GroupedHis.push_back(His_ECp_R2_A);
  GroupedHis.push_back(His_ECp_R2_B);
  GroupedHis.push_back(His_ECp_R2_C);
  GroupedHis.push_back(His_ECp_R3_A);
  GroupedHis.push_back(His_ECp_R3_B);
  GroupedHis.push_back(His_ECp_R3_C);
  GroupedHis.push_back(His_ECm_R2_A);
  GroupedHis.push_back(His_ECm_R2_B);
  GroupedHis.push_back(His_ECm_R2_C);
  GroupedHis.push_back(His_ECm_R3_A);
  GroupedHis.push_back(His_ECm_R3_B);
  GroupedHis.push_back(His_ECm_R3_C);
  GroupedHis.push_back(His_RB1);
  GroupedHis.push_back(His_RB2);
  GroupedHis.push_back(His_RB3);
  GroupedHis.push_back(His_RB4);  
  
  /*
  TPaveText * pt = new TPaveText(0.156928,0.734421,0.239566,0.805638,"NDC");
  pt->SetFillColor(0);
  pt->SetBorderSize(0);
  pt->SetTextSize(0.05);
  pt->SetTextAlign(22);
  
  pt->AddText("CMS");
  //pt->AddText("Work in progress");
  
  TPaveText * pt2 = new TPaveText(0.154424,0.639466,0.267947,0.710682,"NDC");
  pt2->SetFillColor(0);
  pt2->SetBorderSize(0);
  pt2->SetTextAlign(22);
  pt2->SetTextSize(0.02);
  pt2->AddText("Work in progress");
  */
  
  for (iter = WP11.begin(); iter != WP11.end() ;iter++){
    
    string RollName = iter->first;
    cout << RollName << endl;
    double WorkingPoint11 = iter->second;
    double WorkingPoint12 = WP12.find(RollName)->second;
    
    //WorkingPoint11 = WorkingPoint11;
    //WorkingPoint12 = WorkingPoint12;
    
    vector<hvrate> VectorRate11 = map2011.find(RollName)->second;
    
    vector<hvrate> VectorRate15 = map2015.find(RollName)->second;
    
    double Rate11 = 0 ;
    double Rate15 = 0 ;
    
    double min = 10000;
    
    TGraph * graph11 = GetGraphFromVector(VectorRate11,"gr11");
    TGraph * graph15 = GetGraphFromVector(VectorRate15,"gr15");
    //graph11->SaveAs(("results/"+RollName+"_graph.root").c_str());    
    
    int min11, max11,min15,max15;    
    
    min11 = TMath::MinElement(graph11->GetN(),graph11->GetX());
    min15 = TMath::MinElement(graph15->GetN(),graph15->GetX());
    max15 = TMath::MaxElement(graph15->GetN(),graph15->GetX());
    max11 = TMath::MaxElement(graph11->GetN(),graph11->GetX());    
    
    //cout << " 11 " << min11 << " " << max11 << endl;
    //cout << " 15 " << min15 << " " << max15 << endl;
    
    TF1 * pol11 = new TF1("y11","pol2",min11,max11);
    TF1 * pol15 = new TF1("y15","pol2",min15,max15);
    
    graph11->Fit(pol11,"R");
    graph15->Fit(pol15,"R");
    graph11->SetTitle(RollName.c_str());
    graph15->SetTitle(RollName.c_str());
    graph11->SetName((RollName+"_11").c_str());
    graph15->SetName((RollName+"_15").c_str());
    
    graph11->GetXaxis()->SetTitle("Voltage (V)");
    graph15->GetXaxis()->SetTitle("Voltage (V)");
    graph11->GetYaxis()->SetTitle("Rate (Hz/cm^{2})");
    graph11->SetMarkerStyle(kFullStar);
    graph11->SetMarkerSize(graph11->GetMarkerSize()*1.5);;
    graph15->GetYaxis()->SetTitle("Rate (Hz/cm^{2})");
    graph15->SetMarkerStyle(kFullStar);
    graph15->SetMarkerSize(graph15->GetMarkerSize()*1.5);
    
    graphs11_withFits->cd();
    //pt->Draw();
    graph11->Write();
    graphs15_withFits->cd();
    //pt->Draw();
    graph15->Write();
    
    double rate_at_wp11 = pol11->Eval(WorkingPoint11);
    double rate_at_wp15 = pol15->Eval(WorkingPoint12);
    
    //double intrNoise11 = intrinsicNoiseMap_11.at(RollName);
    //double intrNoise15 = intrinsicNoiseMap_15.at(RollName);
    
    double difference = rate_at_wp15 - rate_at_wp11;
    WP_Distribution_Before->Fill(rate_at_wp11);
    WP_Distribution_Before->GetYaxis()->SetTitle("Number of Rolls");
    WP_Distribution_Before->GetXaxis()->SetTitle("#DeltaR (Hz/cm^{2})");
    WP_Distribution_Before->SetTitle("Roll rates at working point distribution - 2011");
    WP_Distribution_Now->Fill(rate_at_wp15);
    WP_Distribution_Now->GetYaxis()->SetTitle("Number of Rolls");
    WP_Distribution_Now->GetXaxis()->SetTitle("#DeltaR (Hz/cm^{2})");
    WP_Distribution_Now->SetTitle("Roll rates at working point distribution - 2015");
    
    pol11->Delete();
    pol15->Delete();
    graph11->Delete();
    graph15->Delete();    
    Histo_fits->Fill(difference);        
    
    for (int i=0; i<VectorRate11.size(); i++){
      hvrate Hv_Rate;
      Hv_Rate = VectorRate11.at(i);
      
      if ( min > abs(WorkingPoint11 - Hv_Rate.hv) ){
	min = abs(WorkingPoint11 - Hv_Rate.hv);
	Rate11 = Hv_Rate.rate;
      }
    }
    
    min = 10000;
    
    for (int i=0; i<VectorRate15.size(); i++){
      hvrate Hv_Rate;
      Hv_Rate = VectorRate15.at(i);
      
      if ( min > abs(WorkingPoint12 - Hv_Rate.hv) ){
	min = abs(WorkingPoint12 - Hv_Rate.hv);
	Rate15 = Hv_Rate.rate;
      }
    
    }
    
    if (RollName.find("W") != string::npos){
      His_Barrel->Fill(difference);
    }
    
    if (RollName.find("RE") != string::npos){
      His_EndCap->Fill(difference);
    
    
    if (RollName.find("RE+") != string::npos){
      if (RollName.find("_R2_") != string::npos){
	
	
	if (RollName.find("_A") !=string::npos){ His_ECp_R2_A->Fill(difference);}//cout << RollName << endl;}
	if (RollName.find("_B") !=string::npos){ His_ECp_R2_B->Fill(difference);}// cout << RollName << endl; }
	if (RollName.substr(12,2) == "_C"){ His_ECp_R2_C->Fill(difference);}//cout << RollName << endl;}
	
      }
      
      if (RollName.find("_R3_") != string::npos){
	if (RollName.find("_A") !=string::npos){ His_ECp_R3_A->Fill(difference);}
	if (RollName.find("_B") !=string::npos){ His_ECp_R3_B->Fill(difference);}
	if (RollName.substr(12,2) == "_C"){ His_ECp_R3_C->Fill(difference);}
      }
    }
    
    
    if (RollName.find("RE-") != string::npos){
      if (RollName.find("_R2_") != string::npos){
	
	if (RollName.find("_A") !=string::npos){ His_ECm_R2_A->Fill(difference);}
	if (RollName.find("_B") !=string::npos){ His_ECm_R2_B->Fill(difference);}
	if (RollName.substr(12,2) == "_C"){ His_ECm_R2_C->Fill(difference);}
	
      }
      
      if (RollName.find("_R3_") != string::npos){
	if (RollName.find("_A") !=string::npos){ His_ECm_R3_A->Fill(difference);}
	if (RollName.find("_B") !=string::npos){ His_ECm_R3_B->Fill(difference);}
	if (RollName.substr(12,2) == "_C"){ His_ECm_R3_C->Fill(difference);}
      }
    }
    
    }
    
    if (RollName.find("RB1") != string::npos){
      His_RB1->Fill(difference);
    }
    if (RollName.find("RB2") != string::npos){
      His_RB2->Fill(difference);
    }
    if (RollName.find("RB3") != string::npos){
      His_RB3->Fill(difference);
    }
    if (RollName.find("RB4") != string::npos){
      His_RB4->Fill(difference);
    }
        
    Histos->Fill((Rate15-Rate11));    
    
    if (abs(Rate15-Rate11) > 0.3) { RollsWithHighDifference << RollName << " " << Rate15-Rate11 << '\n'; }
    if (abs(difference) > 0.3) { RollsWithHighDifference_fits << RollName << " " << difference << '\n'; }
    
    RollsWithHighDifference.clear();
    
  }  
  
  Histos->GetXaxis()->SetTitle("#DeltaR (Hz/cm^{2})");
  Histos->GetYaxis()->SetTitle("Number of Rolls");
  Histos->SetLineWidth(3);
  Histos->SetFillColor(kBlue);
  Histos->SetFillStyle(3001);
  //pt->Draw();
  
  Histo_fits->GetXaxis()->SetTitle("#DeltaR (Hz/cm^{2})");
  Histo_fits->GetYaxis()->SetTitle("Number of Rolls");
  Histo_fits->SetLineWidth(3);
  Histo_fits->SetFillColor(kBlue);
  Histo_fits->SetFillStyle(3001);
  //pt->Draw();
    
  RollsWithHighDifference.close();
  RollsWithHighDifference_fits.close();
  
  rootfile->cd();
  
  for(int i = 0 ; i < GroupedHis.size(); i++){
    TH1F* h = GroupedHis.at(i);

    h->GetXaxis()->SetTitle("#DeltaR (Hz/cm^{2})");
    h->GetYaxis()->SetTitle("Number of Rolls");
    h->SetLineWidth(3);
    h->SetFillColor(kBlue);
    h->SetFillStyle(3001);
    //h->SetStats(kFALSE);
    //pt->Draw();
    h->Write();
    string hname = h->GetName();
    TCanvas * myCanvas = new TCanvas((hname+"Canvas").c_str(), "Canvas", 1200, 700);
    myCanvas->cd();
    h->Draw();
    //pt->Draw();
    
    myCanvas->SaveAs(("results/"+hname+".root").c_str());
  }
  
  graphs11_withFits->Save();
  graphs15_withFits->Save();
  rootfile->Save();
  Histo_fits->SaveAs("results/RateDifference2015_2011_fits.root");
  Histos->SaveAs("results/RateDifference2015_2011.root");
  WP_Distribution_Before->SaveAs("results/RateAtWP_Before.root");
  WP_Distribution_Now->SaveAs("results/RateAtWP_Now.root");
  
}  


int main (int argc, char * argv[] ){
  
  //string myDataFile = argv[1];
  //string argtwo = argv[2];
  
  //map <string, vector<hvrate> > map2011 = GetDataMap("noiseScan2011.txt");
  
  //StoreChamberHVRateGraphs(map2011, " GraphFile2011.root", "CorrCoef2011.root");
  gStyle->SetOptStat(1010);
  NoiseVsHV();
  
  
  
  
  
  return 0;
}
