#include "G4NistManager.hh"

#ifdef CMSSW_GIT_HASH
#include "HGCalTileSim/Tile/interface/Materials.hh"
#else
#include "Materials.hh"
#endif

using namespace CLHEP;

// static list of common materials (cannot be const)
static G4Element* C  = G4NistManager::Instance()->FindOrBuildElement( "C" );
static G4Element* H  = G4NistManager::Instance()->FindOrBuildElement( "H" );
static G4Element* Si = G4NistManager::Instance()->FindOrBuildElement( "Si" );
static G4Element* O  = G4NistManager::Instance()->FindOrBuildElement( "O" );
static G4Element* K  = G4NistManager::Instance()->FindOrBuildElement( "K" );
static G4Element* Sb = G4NistManager::Instance()->FindOrBuildElement( "Sb" );
static G4Element* Rb = G4NistManager::Instance()->FindOrBuildElement( "Rb" );
static G4Element* Cs = G4NistManager::Instance()->FindOrBuildElement( "Cs" );
static G4Element* Lu = G4NistManager::Instance()->FindOrBuildElement( "Lu" );
static G4Element* Y  = G4NistManager::Instance()->FindOrBuildElement( "Y" );
static G4Element* Ce = G4NistManager::Instance()->FindOrBuildElement( "Ce" );
static G4Element* La = G4NistManager::Instance()->FindOrBuildElement( "La" );
static G4Element* Br = G4NistManager::Instance()->FindOrBuildElement( "Br" );
static G4Element* Na = G4NistManager::Instance()->FindOrBuildElement( "Na" );
static G4Element* I  = G4NistManager::Instance()->FindOrBuildElement( "I" );
static G4Element* Tl = G4NistManager::Instance()->FindOrBuildElement( "Tl" );
static G4Element* Al = G4NistManager::Instance()->FindOrBuildElement( "Al" );

static const unsigned flatentries = 2;
static const double minenergy     = 1.0*eV;
static const double maxenergy     = 8.0*eV;
//ev = 1243/lambda
//1.77*eV, 4.44*eV

// Bialkali definition:
// Ref: http://hypernews.slac.stanford.edu/HyperNews/geant4/get/AUX/2013/03/11/12.39-85121-chDetectorConstruction.cc
G4Material*
Make_Bialkali()
{
  G4Material* material
    = new G4Material( "Bialkali", 4.28*g/cm3, 3, kStateSolid );
  material->AddElement( K,  13.3*perCent );
  material->AddElement( Cs, 45.2*perCent );
  material->AddElement( Sb, 41.5*perCent );
  return material;
}

G4Material*
Make_Epoxy()
{
  G4Material* epoxy = new G4Material( "Epoxy", 3.75*g/cm3, 2, kStateSolid );
  epoxy->AddElement( Al, 40*perCent );
  epoxy->AddElement( O,  60*perCent );
  double photonE[flatentries]      = {minenergy,maxenergy};
  double refrac_idx[flatentries]   = {1.50, 1.50};
  double Reflectivity[flatentries] = {0.0, 0.0};
   // Will obe overwritten in surface property defintiion
  double abs_length[flatentries]   = {10*m, 10*m};

  G4MaterialPropertiesTable* table = new G4MaterialPropertiesTable();
  table->AddProperty( "RINDEX",       photonE, refrac_idx,   flatentries );
  table->AddProperty( "REFLECTIVITY", photonE, Reflectivity, flatentries );
  table->AddProperty( "ABSLENGTH",    photonE, abs_length,   flatentries );

  epoxy->SetMaterialPropertiesTable( table );
  return epoxy;
}

G4Material*
Make_Custom_Air()
{
  static double photonE[flatentries]    = {minenergy, maxenergy};
  static double refrac_idx[flatentries] = {1.0, 1.0};

  G4MaterialPropertiesTable* table = new G4MaterialPropertiesTable();
  table->AddProperty( "RINDEX", photonE, refrac_idx, flatentries );

  G4Material* material
    = G4NistManager::Instance()->FindOrBuildMaterial( "G4_AIR" );
  material->SetMaterialPropertiesTable( table );
  return material;
}

G4Material*
Make_Resin()
{
  static double photonE[flatentries] = {minenergy,maxenergy};
  static double refrac_idx[flatentries] = {1.52, 1.52};
  static double abs_length[flatentries] = {10*m, 10*m};

  G4MaterialPropertiesTable* table = new G4MaterialPropertiesTable();
  table->AddProperty( "RINDEX",    photonE, refrac_idx, flatentries );
  table->AddProperty( "ABSLENGTH", photonE, abs_length, flatentries );

  G4Material* material
    = G4NistManager::Instance()->FindOrBuildMaterial( "G4_Pyrex_Glass" );
  material->SetMaterialPropertiesTable( table );

  return material;
}

G4Material*
Make_EJ200()
{
  G4Material* material = new G4Material( "EJ200", 1.023*g/cm3, 2, kStateSolid );
  material->AddElement( C, 91.53*perCent );
  material->AddElement( H,  8.47*perCent );

  static double photonE[flatentries]    = {minenergy, maxenergy};
  static double refrac_idx[flatentries] = { 1.58, 1.58 };

  // Add entries into properties table
  G4MaterialPropertiesTable* table = new G4MaterialPropertiesTable();
  table->AddProperty( "RINDEX", photonE, refrac_idx, flatentries );
  // table->AddProperty( "ABSLENGTH", photonE, abs_length, nentries );
  table->AddConstProperty( "SCINTILLATIONYIELD",        10./keV );
  table->AddConstProperty( "RESOLUTIONSCALE",           1.0 );
  table->AddConstProperty( "FASTSCINTILLATIONRISETIME", 0.9*ns );

  // Disabled for CMSSW
  // table->AddConstProperty( "ELECTRONSCINTILLATIONYIELD", 10./keV );
  // table->AddConstProperty( "ALPHASCINTILLATIONYIELD",   100./MeV );
  // table->AddConstProperty( "FASTTIMECONSTANT",           2.1*ns );
  // table->AddProperty("FASTCOMPONENT",photonE,Scints,nentries);
  // table->AddConstProperty("YIELDRATIO",1.0);

  material->SetMaterialPropertiesTable( table );

  // FIXME: Set the Birks Constant for the EJ200 scintillator
  material->GetIonisation()->SetBirksConstant( 0.126*mm/MeV );

  Update_EJ200_AbsLength( material, 1 );

  return material;
}


G4Material*
Make_BC_630_grease()
{

G4Material* SiO2 = new G4Material("quartz", 2.200*g/cm3, 2);
SiO2->AddElement(Si, 1);
SiO2->AddElement(O , 2);

G4Material* silicone = new G4Material("Silicone", 0.965*g/cm3, 4);
silicone->AddElement(C, 2);
silicone->AddElement(H, 6);
silicone->AddElement(O, 1);
silicone->AddElement(Si, 1);

  G4Material* grease = new G4Material( "Grease", 1.06*g/cm3, 2, kStateSolid );
  grease->AddMaterial( SiO2, 6*perCent );
  grease->AddMaterial( silicone,  94*perCent );

  double photonE[flatentries]      = {minenergy, maxenergy};//1.77*eV, 4.44*eV
  double refrac_idx[flatentries]   = {1.465, 1.465};
  //double Reflectivity[flatentries] = {0.0, 0.0};
  //double abs_length[flatentries]   = {10*m, 10*m};

  G4MaterialPropertiesTable* table = new G4MaterialPropertiesTable();
  table->AddProperty( "RINDEX",       photonE, refrac_idx,   flatentries );
  //table->AddProperty( "REFLECTIVITY", photonE, Reflectivity, flatentries );
  //table->AddProperty( "ABSLENGTH",    photonE, abs_length,   flatentries );

  grease->SetMaterialPropertiesTable( table );
  return grease;
}

void
Update_EJ200_AbsLength( G4Material* material, const double mult )
{
  static const unsigned nentries = 57;
  // ~350 to 550nm
  static double photonE[nentries] = {
    3.542405514*eV, 3.492512479*eV, 3.444005361*eV, 3.396827205*eV,
    3.350924135*eV, 3.306245147*eV, 3.262741921*eV, 3.220368649*eV,
    3.179081872*eV, 3.170951228*eV, 3.162862066*eV, 3.154814071*eV,
    3.146806929*eV, 3.138840329*eV, 3.130913965*eV, 3.123027531*eV,
    3.115180729*eV, 3.107373258*eV, 3.099604825*eV, 3.091875137*eV,
    3.084183905*eV, 3.076530844*eV, 3.068915668*eV, 3.061338099*eV,
    3.053797857*eV, 3.046294668*eV, 3.038828260*eV, 3.031398362*eV,
    3.024004707*eV, 2.987570916*eV, 2.952004595*eV, 2.917275129*eV,
    2.883353326*eV, 2.850211333*eV, 2.817822568*eV, 2.786161640*eV,
    2.755204289*eV, 2.724927319*eV, 2.695308543*eV, 2.666326731*eV,
    2.637961553*eV, 2.610193537*eV, 2.583004021*eV, 2.556375113*eV,
    2.530289653*eV, 2.504731172*eV, 2.479683860*eV, 2.455132535*eV,
    2.431062608*eV, 2.407460058*eV, 2.384311404*eV, 2.361603676*eV,
    2.339324396*eV, 2.317461551*eV, 2.296003574*eV, 2.274939321*eV,
    2.254258055*eV
  };

  static double Normabs_length[nentries] =
  {
    17.42923195*mm, 17.50261763*mm, 17.52080780*mm, 17.54004961*mm,
    17.56848311*mm, 17.66153201*mm, 17.69242388*mm, 18.13437416*mm,
    22.14857794*mm, 23.91636107*mm, 26.19927553*mm, 29.08895000*mm,
    32.75048588*mm, 37.28149870*mm, 43.07842006*mm, 50.39849206*mm,
    59.85105799*mm, 71.98244983*mm, 87.89754636*mm, 108.7633087*mm,
    136.5727481*mm, 173.1953472*mm, 222.5973304*mm, 287.5620553*mm,
    375.9938324*mm, 489.0761462*mm, 637.3307552*mm, 815.2529798*mm,
    1039.856920*mm, 2315.937016*mm, 3211.846828*mm, 3800.000000*mm,
    4162.309734*mm, 4429.365072*mm, 4633.055771*mm, 4944.696400*mm,
    5176.400316*mm, 5396.503919*mm, 5716.638906*mm, 6168.317205*mm,
    6280.014490*mm, 6360.307136*mm, 6639.580861*mm, 6926.562350*mm,
    7323.864938*mm, 7652.045236*mm, 8104.081942*mm, 8525.979809*mm,
    8898.295533*mm, 9225.296344*mm, 9847.707211*mm, 10310.48782*mm,
    10862.63280*mm, 11237.54509*mm, 11929.82503*mm, 12944.01067*mm,
    13183.00043*mm
  };

  double abs_length[nentries];

  for( unsigned i = 0; i < nentries; ++i ){
    abs_length[i] = Normabs_length[i] * mult;
  }

  // Add entries into properties table
  G4MaterialPropertiesTable* table = material->GetMaterialPropertiesTable();
  table->RemoveProperty( "ABSLENGTH" );
  table->AddProperty( "ABSLENGTH", photonE, abs_length, nentries );
}
