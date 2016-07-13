{
  
  gStyle->SetOptStat(1100);
  gStyle->SetStatBorderSize(0);

  TPaveText * pt = new TPaveText(0.298831,0.74184,0.383139,0.869436,"NDC");

  pt->SetFillColor(0);
  pt->SetBorderSize(0);
  pt->SetTextSize(0.05);
  pt->SetTextAlign(22);
  
  pt->AddText("CMS");
  pt->AddText("Work in progress");
  //pt->AddText("Work in progress");
  
  TPaveText * pt2 = new TPaveText(0.156928,0.639466,0.239566,0.710682,"NDC");
  pt2->SetFillColor(0);
  pt2->SetBorderSize(0);
  pt2->SetTextAlign(12);
  pt2->SetTextFont();
  pt2->SetTextSize(0.0296736);
  pt2->AddText("Work in progress");
 
}
