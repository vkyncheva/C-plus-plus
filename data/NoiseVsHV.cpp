#include <iostream>
#include "string.h"
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
    cout << myKey << endl;
    
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
    
    
    
    TCanvas * myCanvas = new TCanvas((myKey+"Canvas").c_str(), "Canvas", 1200, 700);
    int vectorSize= myVector.size();
    double x[vectorSize], y[vectorSize];
    myCanvas->cd();
    
       
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
    myCanvas->SaveAs(Picture.c_str());
    
    
    
    
    
    rootfile->cd();
    myGraph->Write();
    
    myGraph->Delete("R");
    
    myCanvas->Delete("R");  
    
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
  
  return myGraph;
} 
 



void NoiseVsHV (){
  
  map <string, vector<hvrate> > map2011 = GetDataMap("noiseScan11.txt");
  map <string, vector<hvrate> > map2015 = GetDataMap("noiseScan15.txt");
  
 // StoreChamberHVRateGraphs(map2011, " GraphFile2011.root", "CorrCoef2011.root");
 // StoreChamberHVRateGraphs(map2015, "GraphFile2015.root", "CorrCoef2015.root");
  
  map <string, double> WP11 = GetWorkingPointForRollNameMap("WorkingPoints/WP2011.txt");
  map <string, double> WP12 = GetWorkingPointForRollNameMap("WorkingPoints/WP2012.txt");
  
  map<string, double>::iterator iter;
  
  TH1F * Histos = new TH1F ("RateDifference2015_2011", "RateDifference2015_2011", 100, -2,2);
  Histos->SetStats(111111);
  
  string GroupedHisFile = "GroupedHisFile.root";
  TFile * rootfile = new TFile(GroupedHisFile.c_str(),"RECREATE");
  
  
  TH1F * His_Barrel = new TH1F ("RateDifference2011_2015_Barrel","RateDifference2011_2015_Barrel", 100, -2,2);
  TH1F * His_EndCap = new TH1F ("RateDifference2011_2015_EndCap","RateDifference2011_2015_EndCap", 100, -2,2);
  TH1F * His_ECp_R2_A = new TH1F ("RateDifference2011_2015_ECp_R2_A","RateDifference2011_2015_EndCap", 100, -2,2);
  TH1F * His_ECp_R2_B= new TH1F ("RateDifference2011_2015_ECp_R2_B","RateDifference2011_2015_ECp_R2_B", 100, -2,2);
  TH1F * His_ECp_R2_C= new TH1F ("RateDifference2011_2015_ECp_R2_C","RateDifference2011_2015_ECp_R2_C", 100, -2,2);
  TH1F * His_ECp_R3_A= new TH1F ("RateDifference2011_2015_ECp_R3_A","RateDifference2011_2015_ECp_R3_A", 100, -2,2);
  TH1F * His_ECp_R3_B= new TH1F ("RateDifference2011_2015_ECp_R3_B","RateDifference2011_2015_ECp_R3_B", 100, -2,2);
  TH1F * His_ECp_R3_C= new TH1F ("RateDifference2011_2015_ECp_R3_C","RateDifference2011_2015_ECp_R3_C", 100, -2,2);
  TH1F * His_ECm_R2_A = new TH1F ("RateDifference2011_2015_ECm_R2_A","RateDifference2011_2015_ECm_R2_A", 100, -2,2);
  TH1F * His_ECm_R2_B= new TH1F ("RateDifference2011_2015_ECm_R2_B","RateDifference2011_2015_ECm_R2_B", 100, -2,2);
  TH1F * His_ECm_R2_C= new TH1F ("RateDifference2011_2015_ECm_R2_C","RateDifference2011_2015_ECm_R2_C", 100, -2,2);
  TH1F * His_ECm_R3_A= new TH1F ("RateDifference2011_2015_ECm_R3_A","RateDifference2011_2015_ECm_R3_A", 100, -2,2);
  TH1F * His_ECm_R3_B= new TH1F ("RateDifference2011_2015_ECm_R3_B","RateDifference2011_2015_ECm_R3_B", 100, -2,2);
  TH1F * His_ECm_R3_C= new TH1F ("RateDifference2011_2015_ECm_R3_C","RateDifference2011_2015_ECm_R3_C", 100, -2,2);
  
  TH1F * His_RB1= new TH1F ("RateDifference2011_2015_RB1","RateDifference2011_2015_RB1", 100, -2,2);
  TH1F * His_RB2= new TH1F ("RateDifference2011_2015_RB2","RateDifference2011_2015_RB2", 100, -2,2);
  TH1F * His_RB3= new TH1F ("RateDifference2011_2015_RB3","RateDifference2011_2015_RB3", 100, -2,2);
  TH1F * His_RB4= new TH1F ("RateDifference2011_2015_RB4","RateDifference2011_2015_RB4", 100, -2,2);
  
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
  
  
  
  
  
  for (iter = WP11.begin(); iter != WP11.end() ;iter++){
    
    string RollName = iter->first;
    double WorkingPoint11 = iter->second;
    double WorkingPoint12 = WP12.find(RollName)->second;
    
    WorkingPoint11 = WorkingPoint11*1000;
    WorkingPoint12 = WorkingPoint12*1000;
    
    vector<hvrate> VectorRate11 = map2011.find(RollName)->second;
    
    vector<hvrate> VectorRate15 = map2015.find(RollName)->second;
    
    double Rate11 = 0 ;
    double Rate15 = 0 ;
    
    double min = 10000;
    
    TGraph * graph11 = GetGraphFromVector(VectorRate11,"gr11");
    TGraph * graph15 = GetGraphFromVector(VectorRate15,"gr15");    
    
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
      His_Barrel->Fill(Rate15-Rate11);
    }
    
    if (RollName.find("RE") != string::npos){
      His_EndCap->Fill(Rate15-Rate11);
    }
    
    if (RollName.find("RE+") != string::npos){
      if (RollName.find("R2") != string::npos){
	
	if (RollName.find("_A") !=string::npos){ His_ECp_R2_A->Fill(Rate15-Rate11);}
	if (RollName.find("_B") !=string::npos){ His_ECp_R2_B->Fill(Rate15-Rate11);}
	if (RollName.find("_C") !=string::npos){ His_ECp_R2_C->Fill(Rate15-Rate11);}
	
      }
      
      if (RollName.find("R3") != string::npos){
	if (RollName.find("_A") !=string::npos){ His_ECp_R3_A->Fill(Rate15-Rate11);}
	if (RollName.find("_B") !=string::npos){ His_ECp_R3_B->Fill(Rate15-Rate11);}
	if (RollName.find("_C") !=string::npos){ His_ECp_R3_C->Fill(Rate15-Rate11);}
      }
    }
    
    
    else {
      if (RollName.find("R2") != string::npos){
	
	if (RollName.find("_A") !=string::npos){ His_ECm_R2_A->Fill(Rate15-Rate11);}
	if (RollName.find("_B") !=string::npos){ His_ECm_R2_B->Fill(Rate15-Rate11);}
	if (RollName.find("_C") !=string::npos){ His_ECm_R2_C->Fill(Rate15-Rate11);}
	
      }
      
      if (RollName.find("R3") != string::npos){
	if (RollName.find("_A") !=string::npos){ His_ECm_R3_A->Fill(Rate15-Rate11);}
	if (RollName.find("_B") !=string::npos){ His_ECm_R3_B->Fill(Rate15-Rate11);}
	if (RollName.find("_C") !=string::npos){ His_ECm_R3_C->Fill(Rate15-Rate11);}
      }
    }
    
    if (RollName.find("RB1") != string::npos){
      His_RB1->Fill(Rate15-Rate11);
    }
    if (RollName.find("RB2") != string::npos){
      His_RB2->Fill(Rate15-Rate11);
    }
    if (RollName.find("RB3") != string::npos){
      His_RB3->Fill(Rate15-Rate11);
    }
    if (RollName.find("RB4") != string::npos){
      His_RB4->Fill(Rate15-Rate11);
    }
    
    
   
    Histos->Fill((Rate15-Rate11));
    Histos->GetXaxis()->SetTitle("Rate (Hz/cm^{2}");
    Histos->GetYaxis()->SetTitle("Number of Rolls");
    Histos->SetLineWidth(3);
    Histos->SetFillColor(kBlue);
    Histos->SetFillStyle(3001);
   
  }
  
  rootfile->cd();
    
    for(int i = 0 ; i<GroupedHis.size(); i++){
      TH1F* h = GroupedHis.at(i);
    
      h->GetXaxis()->SetTitle("Rate (Hz/cm^{2}");
      h->GetYaxis()->SetTitle("Number of Rolls");
      h->SetLineWidth(3);
      h->SetFillColor(kBlue);
      h->SetFillStyle(3001);
      h->Write();
    }
  
  
  
  rootfile->Save();
  Histos->SaveAs("RateDifference2015_2011.root");
  
}  
  
  

 

int main (int argc, char * argv[] ){
  
  //string myDataFile = argv[1];
  //string argtwo = argv[2];
  
  //map <string, vector<hvrate> > map2011 = GetDataMap("noiseScan2011.txt");
  
  //StoreChamberHVRateGraphs(map2011, " GraphFile2011.root", "CorrCoef2011.root");
  NoiseVsHV();
  
  
  
  
  
  return 0;
}