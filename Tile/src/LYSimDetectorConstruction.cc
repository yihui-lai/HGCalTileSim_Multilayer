#ifdef CMSSW_GIT_HASH
#include "HGCalTileSim/Tile/interface/LYSimDetectorConstruction.hh"
#include "HGCalTileSim/Tile/interface/LYSimDetectorMessenger.hh"
#include "HGCalTileSim/Tile/interface/Materials.hh"
#include "HGCalTileSim/Tile/interface/SurfaceProperty.hh"
#else
#include "LYSimDetectorConstruction.hh"
#include "LYSimDetectorMessenger.hh"
#include "Materials.hh"
#include "SurfaceProperty.hh"
#endif

#include <math.h>
#include <vector>

#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4Ellipsoid.hh"
#include "G4GeometryManager.hh"
#include "G4IntersectionSolid.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4Orb.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4PVPlacement.hh"
#include "G4RegionStore.hh"
#include "G4RotationMatrix.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include "G4SolidStore.hh"
#include "G4Sphere.hh"
#include "G4SubtractionSolid.hh"
#include "G4SystemOfUnits.hh"
#include "G4Trap.hh"
#include "G4Trd.hh"
#include "G4Tubs.hh"
#include "G4MultiUnion.hh"
#include "G4UnionSolid.hh"
#include "G4UnitsTable.hh"
#include "G4VisAttributes.hh"
#include "G4VPhysicalVolume.hh"
#include "TF1.h"
#include "G4Polycone.hh"

using std::cos;
using std::sin;
using std::tan;
using std::atan;
using std::exp;
using namespace CLHEP;

LYSimPMTSD* LYSimDetectorConstruction::fPMTSD = NULL;

LYSimDetectorConstruction::LYSimDetectorConstruction()
  : G4VUserDetectorConstruction()
{
  fdetectorMessenger = new LYSimDetectorMessenger( this );

  _tilex        = 30*mm;//29.65*mm;
  _tiley        = 30*mm;//29.65*mm;
  _tilez        = 3.0*mm;//3.0*mm;
  _tile_x1      = 0.0*mm;
  _tile_x2      = 0.0*mm;
  wrapgap       = 0.065*mm;
  wrapthickness = 0.1*mm;

  tilegap = 0.1*mm;//another probability 0.5*mm
  tile_number = 1;
  is_ESR = true;
  material = 1;
  _dimplesa = 1.3;

  _absmult      = 1;
  _wrap_reflect = 0.985;

  _sipm_deadwidth  = 0.2*mm;
  _sipm_x          = 1.3*mm;
  _sipm_y          = 1.3*mm;
  _sipm_z          = 0.35*mm;
  _sipm_rimwidth   = 0.4*mm;
  _sipm_glasswidth = 0.3*mm;


  _sipm_standz     = 0.2*mm;  // used as dx
  _gap_pcb_wrap    = 0.2*mm;  // used as dy

  // Default Dimple settings
  _dimple_type   = SPHERICAL;// 0: Normal, 1: Pyramid, 2: Parabolic
  _dimple_indent = 1.6*mm;//1.6*mm;
  _dimple_radius = 2.65*mm;//3.5*mm;// 3.4409*mm

  // Default Hole settings
  _pcb_radius       = 3.175;
  _pcb_reflectivity = 0.5;


  // Defining material list.
  fBialkali = Make_Bialkali();
  fEpoxy    = Make_Epoxy();
  fAir      = Make_Custom_Air();
  if(material==1){ fEJ200    = Make_EJ200();}else{fEJ200    = Make_SCSN81();}
  //fEJ200    = Make_EJ200();
 
//  scsn81    = Make_SCSN81();
  fResin    = Make_Resin();
  fBC_630_grease = Make_BC_630_grease();
  SetTileAbsMult( _absmult );

  // Defining surface list.
  fTyvekSurface           = MakeS_TyvekCrystal();
  fESROpSurface           = MakeS_Rough();
  fPolishedOpSurface      = MakeS_Polished();
  fIdealPolishedOpSurface = MakeS_IdealPolished();
  fIdealWhiteOpSurface    = MakeS_IdealWhiteSurface();
  fSiPMSurface            = MakeS_SiPM();
  fPCBSurface             = MakeS_PCBSurface();
//  fdimpleSurface          = MakeS_dimple();
  fdimpleSurface          = MakeS_IdealPolished();
  SetWrapReflect( _wrap_reflect );

}

void
LYSimDetectorConstruction::UpdateGeometry()
{
  // clean-up previous geometry
  G4GeometryManager::GetInstance()->OpenGeometry();

  G4PhysicalVolumeStore::GetInstance()->Clean();
  G4LogicalVolumeStore::GetInstance()->Clean();
  G4SolidStore::GetInstance()->Clean();
  G4LogicalSkinSurface::CleanSurfaceTable();
  G4LogicalBorderSurface::CleanSurfaceTable();

  G4RunManager::GetRunManager()->DefineWorldVolume( Construct() );
  G4RunManager::GetRunManager()->GeometryHasBeenModified();
}

LYSimDetectorConstruction::~LYSimDetectorConstruction()
{
  if( fdetectorMessenger ){ delete fdetectorMessenger; }
}

G4VPhysicalVolume*
LYSimDetectorConstruction::Construct()
{
  static const bool checkOverlaps = true;
  std::cout<<"---------------- material: "<<material<<" _dimplesa: "<<_dimplesa<<std::endl;
//  SetSurfaceDimpleSA( _dimplesa );




  ///////////////////////////////////////////////////////////////////////////////
  // World volume
  ///////////////////////////////////////////////////////////////////////////////
  G4Box* solidWorld = new G4Box( "World", WorldHalfX()
                               , WorldHalfY(), WorldHalfZ() );

  G4LogicalVolume* logicWorld = new G4LogicalVolume( solidWorld, fAir, "World" );

  G4VPhysicalVolume* physWorld = new G4PVPlacement( 0
                                                  , G4ThreeVector( 0, 0, 0 )
                                                  , logicWorld
                                                  , "World"
                                                  , 0
                                                  , false
                                                  , 0
                                                  , checkOverlaps );
  ///////////////////////////////////////////////////////////////////////////////
  // wrapping
  ///////////////////////////////////////////////////////////////////////////////
  G4VSolid* solidWrap = ConstructHollowWrapSolid();
  G4VSolid* solidWrapGap = ConstructHollowWrapGapSolid();
  // The matrial of the wrap isn't as important as the surface
  G4LogicalVolume* logicWrap = new G4LogicalVolume( solidWrap, fEpoxy,  "Wrap" );
  G4LogicalVolume* logicWrapGap = new G4LogicalVolume( solidWrapGap, fAir,  "WrapGap" );
//  G4LogicalSkinSurface* perfect_surface    = new G4LogicalSkinSurface( "perfect_surface"  , logicWrapGap, MakeS_Mirror() );

  const G4ThreeVector tile_offset( 0, 0
                                , -0.5*(_tilez*(tile_number-1) + tilegap*(tile_number-1)));

  G4VPhysicalVolume* physWrap = new G4PVPlacement( 0
                                                 , tile_offset
                                                 , logicWrap
                                                 , "Wrap"
                                                 , logicWorld
                                                 , false
                                                 , 0
                                                 , checkOverlaps );

  G4VPhysicalVolume* physWrapGap = new G4PVPlacement( 0
                                                 , tile_offset
                                                 , logicWrapGap
                                                 , "WrapGap"
                                                 , logicWorld
                                                 , false
                                                 , 0
                                                 , checkOverlaps );

  // Exposed PCB Back plane
  const G4ThreeVector pcb_offset( 0, 0
                                , 0.5*_tilez + wrapgap + 2*wrapthickness +_gap_pcb_wrap );

  G4VSolid* solidPCB = new G4Tubs( "PCBSolid"
                                 , 0, _pcb_radius + 1*mm
                                 , wrapthickness
                                 , 0, 2*pi );
  G4LogicalVolume* logicPCB = new G4LogicalVolume( solidPCB, fEpoxy, "PCB" );

  G4VPhysicalVolume* physPCB = new G4PVPlacement( 0, pcb_offset
                                                , logicPCB, "PCB"
                                                , logicWorld, false
                                                , 0, checkOverlaps  );

  ///////////////////////////////////////////////////////////////////////////////
  // Subtracted Dimple Version (dimple sub from tile, WWW = mothervolume of both)
  ///////////////////////////////////////////////////////////////////////////////
  G4VSolid* solidTile = ConstructTrapazoidSolid( "TileTrap"
                                               , _tilex
                                               , _tiley
                                               , _tilez
                                               , _tile_x1
                                               , _tile_x2 );

  G4VSolid* solidTileSubtract = new G4Sphere( "TileSub", 0, GetDimpleSizeRadius()
                                            , 0, 2* pi, 0, pi );
//  G4VSolid* solidTileSubtract = ConstructArbiDimpleSolid();

  G4VSolid* hollowedTile = new G4SubtractionSolid( "TileSolid"
                                                 , solidTile, solidTileSubtract
//                                                 , 0, G4ThreeVector( 0, 0, 0.5*_tilez ) );
                                                 , 0, CalcDimpleOffset() );

  G4LogicalVolume* logicTile = new G4LogicalVolume( hollowedTile
                                                  , fEJ200, "TileLogic" );

  G4VPhysicalVolume* physTile = new G4PVPlacement( 0
                                                 , G4ThreeVector( 0, 0, 0 )
                                                 , logicTile
                                                 , "TilePhysic"
                                                 , logicWorld
                                                 , false
                                                 , 0
                                                 , checkOverlaps );

/*
//dimple gap
//  G4VSolid* soliddimpleSubtract = new G4Sphere( "dimpleSub", GetDimpleSizeRadius()-0.01, GetDimpleSizeRadius()
//                                            , 0, 2* pi, 0, pi );
  G4VSolid* soliddimpleSubtract = new G4SubtractionSolid( "dimpleSub", ConstructArbiDimpleSolid(1),ConstructArbiDimpleSolid(0.99) , 0, G4ThreeVector( 0, 0, 0 ) );

  G4VSolid* holloweddimple = new G4IntersectionSolid( "dimpleSolid"
                                                 , solidTile, soliddimpleSubtract
                                                 , 0, G4ThreeVector( 0, 0, 0.5*_tilez ) );
//                                                 , 0, CalcDimpleOffset() );
  G4LogicalVolume* logicdimple = new G4LogicalVolume( holloweddimple
                                                  , fAir, "dimpleLogic" );
  G4VPhysicalVolume* physdimple = new G4PVPlacement( 0
                                                 , G4ThreeVector( 0, 0, 0 )
                                                 , logicdimple
                                                 , "dimplePhysic"
                                                 , logicWorld
                                                 , false
                                                 , 0
                                                 , checkOverlaps );
*/
  ///////////////////////////////////////////////////////////////////////////////
  // put more tile layers and grease
  ///////////////////////////////////////////////////////////////////////////////

  G4LogicalVolume* mlogicTile= new G4LogicalVolume( solidTile
                                               , fEJ200, "mTileLogic" );

  std::vector<G4VPhysicalVolume*> mphysTileList;

  G4VSolid* msolidGrease = ConstructTrapazoidSolid( "GreaseTrap"
                                               , _tilex
                                               , _tiley
                                               , tilegap
                                               , _tile_x1
                                               , _tile_x2 );
  G4LogicalVolume* mlogicGrease= new G4LogicalVolume( msolidGrease
                                               , fBC_630_grease, "GreaseLogic" );

  std::vector<G4VPhysicalVolume*> mphysGreaseList;

  for(int i=1; i<tile_number; i++){

      mphysGreaseList.push_back( new G4PVPlacement(0, G4ThreeVector( 0, 0, -1*(_tilez+tilegap)*i+0.5*(_tilez+tilegap) ), mlogicGrease
                                               , "GreasePhysic"
                                               , logicWorld
                                               , false
                                               , 0
                                               , checkOverlaps)); 

      mphysTileList.push_back( new G4PVPlacement(0
                                               , G4ThreeVector( 0, 0, -1*(_tilez+tilegap)*i )
                                               , mlogicTile
                                               , "mTilePhysic"
                                               , logicWorld
                                               , false
                                               , 0
                                               , checkOverlaps ));

  }





  ///////////////////////////////////////////////////////////////////////////////
  // SiPM
  ///////////////////////////////////////////////////////////////////////////////


  double extra_l = 0.53*mm;
  double thin_surrounding = 0.01*mm;


  G4Box* solidSiPMactive = new G4Box( "SiPMDead"
                                  , 0.5*_sipm_x, 0.5*_sipm_y
                                  , _sipm_z );

  G4Box* solidSiPMInnerBox = new G4Box( "SiPMInnerBox"
                                      , 0.5*_sipm_x+thin_surrounding, 0.5*_sipm_y+thin_surrounding
                                      ,  0.5*_sipm_z);


  G4VSolid* solidSiPMCase
    = new G4SubtractionSolid( "SiPMCase"
                            , solidSiPMInnerBox, solidSiPMactive
                            , 0
                            , G4ThreeVector( 0, 0, 0 ) );

  G4VSolid* solidSiPMInner
    = new G4IntersectionSolid( "SiPMInner"
                             , solidSiPMInnerBox, solidSiPMactive
                             , 0
                             , G4ThreeVector( 0, 0, 0 ) );

  G4Box* solidSiPMResinOuter
    = new G4Box( "SiPMResinOuter"
               , 0.5*_sipm_x + _sipm_rimwidth + 0.5*extra_l
               , 0.5*_sipm_y + _sipm_rimwidth
               , 0.5*_sipm_z + 0.5*_sipm_glasswidth );

  G4VSolid* solidSiPMResin
    = new G4SubtractionSolid( "SiPMResin"
                            , solidSiPMResinOuter, solidSiPMInnerBox
                            ,  0
                            , G4ThreeVector( 0.5*extra_l, 0, 0.5*_sipm_glasswidth ) );
  G4Box* solidSiPMStand
    = new G4Box( "SiPMStand"
               , 0.5*_sipm_x+_sipm_rimwidth + 0.5*extra_l
               , 0.5*_sipm_y+_sipm_rimwidth
               , 0.5*_sipm_standz );


  G4LogicalVolume* logicSiPM = new G4LogicalVolume( solidSiPMInner
                                                  , fBialkali,  "SiPM" );

  G4LogicalVolume* logicSiPMCase = new G4LogicalVolume( solidSiPMCase
                                                      , fEpoxy, "SiPMBack" );

  G4LogicalVolume* logicSiPMResin = new G4LogicalVolume( solidSiPMResin
                                                       , fResin, "SiPMResin" );

  G4LogicalVolume* logicSiPMStand = new G4LogicalVolume( solidSiPMStand
                                                       , fEpoxy, "SiPMStand" );


double xy_shift = 1.7*mm;
double xd=xy_shift/sqrt(2);
double yd=xy_shift/sqrt(2);

xd=0;
yd=0;

  const G4ThreeVector SiPMOffset( xd, yd
                                , +0.5*_tilez - 0.5*_sipm_z - _sipm_standz +wrapgap+wrapthickness +_gap_pcb_wrap);
  const G4ThreeVector ResinOffset( -0.5*extra_l+xd, yd
                                 , +0.5*_tilez - 0.5*_sipm_z - _sipm_standz
                                   - 0.5*_sipm_glasswidth +wrapgap+wrapthickness +_gap_pcb_wrap);
  const G4ThreeVector StandOffset( -0.5*extra_l+xd, yd, +0.5*_tilez - 0.5*_sipm_standz+wrapgap+wrapthickness +_gap_pcb_wrap);

  G4VPhysicalVolume* physSiPMStand = new G4PVPlacement( 0
                                                      , StandOffset
                                                      , logicSiPMStand
                                                      , "SiPMStand"
                                                      , logicWorld
                                                      , false
                                                      , 0
                                                      , checkOverlaps );

  G4VPhysicalVolume* physSiPMCase = new G4PVPlacement( 0
                                                     , SiPMOffset
                                                     , logicSiPMCase
                                                     , "Case"
                                                     , logicWorld
                                                     , false
                                                     , 0
                                                     , checkOverlaps );

  G4VPhysicalVolume* physSiPMResin = new G4PVPlacement( 0
                                                      , ResinOffset
                                                      , logicSiPMResin
                                                      , "SiPMResin"
                                                      , logicWorld
                                                      , false
                                                      , 0
                                                      , checkOverlaps  );

  G4VPhysicalVolume* physSiPM = new G4PVPlacement( 0
                                                 , SiPMOffset
                                                 , logicSiPM
                                                 , "SiPM"
                                                 , logicWorld
                                                 , false
                                                 , 0
                                                 , checkOverlaps );





/*
  G4Box* solidSiPMDead = new G4Box( "SiPMDead"
                                  , 0.5*_sipm_deadwidth, 0.5*_sipm_deadwidth
                                  , _sipm_z );

  G4Box* solidSiPMInnerBox = new G4Box( "SiPMInnerBox"
                                      , 0.5*_sipm_x, 0.5*_sipm_y,  0.8*_sipm_z );

  G4Box* solidSiPMOuter = new G4Box( "SiPMOuter"
                                   , 0.5*_sipm_x + _sipm_rimwidth
                                   , 0.5*_sipm_y + _sipm_rimwidth
                                   , 0.5*_sipm_z );
  G4Box* solidSiPMStand
    = new G4Box( "SiPMStand"
               , 0.5*_sipm_x+_sipm_rimwidth + _sipm_glasswidth
               , 0.5*_sipm_y+_sipm_rimwidth + _sipm_glasswidth
               , 0.5*_sipm_standz );

  G4Box* solidSiPMResinOuter
    = new G4Box( "SiPMResinOuter"
               , 0.5*_sipm_x + _sipm_rimwidth + _sipm_glasswidth
               , 0.5*_sipm_y + _sipm_rimwidth + _sipm_glasswidth
               , 0.5*_sipm_z + _sipm_glasswidth );

  G4VSolid* solidSiPMSubtract
    = new G4SubtractionSolid( "SiPMSubtract"
                            ,  solidSiPMInnerBox, solidSiPMDead
                            ,   0, G4ThreeVector( 0, 0, 0 ) );
  G4VSolid* solidSiPMCase
    = new G4SubtractionSolid( "SiPMCase"
                            , solidSiPMOuter, solidSiPMSubtract
                            , 0
                            , G4ThreeVector( 0, 0, -0.65 * _sipm_z ) );

  G4VSolid* solidSiPMInner
    = new G4IntersectionSolid( "SiPMInner"
                             , solidSiPMOuter, solidSiPMSubtract
                             , 0
                             , G4ThreeVector( 0, 0, -0.65*_sipm_z ) );

  G4VSolid* solidSiPMResin
    = new G4SubtractionSolid( "SiPMResin"
                            , solidSiPMResinOuter, solidSiPMOuter
                            ,  0
                            , G4ThreeVector( 0, 0, _sipm_glasswidth ) );


  G4LogicalVolume* logicSiPM = new G4LogicalVolume( solidSiPMInner
                                                  , fBialkali,  "SiPM" );

  G4LogicalVolume* logicSiPMCase = new G4LogicalVolume( solidSiPMCase
                                                      , fEpoxy, "SiPMBack" );

  G4LogicalVolume* logicSiPMResin = new G4LogicalVolume( solidSiPMResin
                                                       , fResin, "SiPMResin" );

  G4LogicalVolume* logicSiPMStand = new G4LogicalVolume( solidSiPMStand
                                                       , fEpoxy, "SiPMStand" );

  const G4ThreeVector SiPMOffset( 0, 0
                                , +0.5*_tilez - 0.5*_sipm_z - _sipm_standz +wrapgap+wrapthickness);
  const G4ThreeVector ResinOffset( 0, 0
                                 , +0.5*_tilez - 0.5*_sipm_z - _sipm_standz
                                   - _sipm_glasswidth +wrapgap+wrapthickness);
  const G4ThreeVector StandOffset( 0, 0, +0.5*_tilez - 0.5*_sipm_standz +wrapgap+wrapthickness);

  G4VPhysicalVolume* physSiPMStand = new G4PVPlacement( 0
                                                      , StandOffset
                                                      , logicSiPMStand
                                                      , "SiPMStand"
                                                      , logicWorld
                                                      , false
                                                      , 0
                                                      , checkOverlaps );

  G4VPhysicalVolume* physSiPMCase = new G4PVPlacement( 0
                                                     , SiPMOffset
                                                     , logicSiPMCase
                                                     , "Case"
                                                     , logicWorld
                                                     , false
                                                     , 0
                                                     , checkOverlaps );

  G4VPhysicalVolume* physSiPMResin = new G4PVPlacement( 0
                                                      , ResinOffset
                                                      , logicSiPMResin
                                                      , "SiPMResin"
                                                      , logicWorld
                                                      , false
                                                      , 0
                                                      , checkOverlaps  );

  G4VPhysicalVolume* physSiPM = new G4PVPlacement( 0
                                                 , SiPMOffset
                                                 , logicSiPM
                                                 , "SiPM"
                                                 , logicWorld
                                                 , false
                                                 , 0
                                                 , checkOverlaps );

*/

  ///////////////////////////////////////////////////////////////////////////////
  // Defining surfaces
  ///////////////////////////////////////////////////////////////////////////////
  //G4LogicalBorderSurface* WrapAirSurface;
  if (is_ESR){
  G4LogicalBorderSurface*  WrapAirSurface = new G4LogicalBorderSurface( "WrapAirSurface"
                              , physWrapGap
                              , physWrap
                              , fESROpSurface );
  }else{
  G4LogicalBorderSurface*  WrapAirSurface = new G4LogicalBorderSurface( "WrapAirSurface"
                              , physWrapGap
                              , physWrap
                              , fTyvekSurface );
  }

  G4LogicalBorderSurface* TileSurface =
    new G4LogicalBorderSurface( "TileSurface"
                              , physTile
                              , physWrapGap
//                              , physWorld
                              , fIdealPolishedOpSurface );
  std::vector<G4LogicalBorderSurface*> mTileSurfaceList;
  std::vector<G4LogicalBorderSurface*> mGreaseSurfaceList;

  for(int i=1; i<tile_number; i++){
    mTileSurfaceList.push_back( new G4LogicalBorderSurface(
                               "mTileSurface"
                              , mphysTileList.at(i-1)
//                              , physWorld
                              , physWrapGap
                              , fIdealPolishedOpSurface) );    


    mGreaseSurfaceList.push_back( new G4LogicalBorderSurface(
                               "mGreaseSurface"
                              , mphysGreaseList.at(i-1)
//                              , physWorld
                              , physWrapGap
                              , fIdealPolishedOpSurface) );
  }



//-------------------
// dimple surface
//-------------------
/*
  G4LogicalBorderSurface* dimpleSurface =
    new G4LogicalBorderSurface( "dimpleSurface"
                              , physTile
                              , physdimple
                              , fdimpleSurface );
*/


  G4LogicalSkinSurface* CaseSurface
    = new G4LogicalSkinSurface( "SiPMCaseSurface"
                              , logicSiPMCase
                              , fIdealWhiteOpSurface );
  G4LogicalSkinSurface* StandSurface
    = new G4LogicalSkinSurface( "SiPMStandSurface"
                              , logicSiPMStand
                              , fIdealWhiteOpSurface );
  G4LogicalSkinSurface* SiPMSurface
    = new G4LogicalSkinSurface( "SiPMSurface", logicSiPM, fSiPMSurface );
  G4LogicalSkinSurface* PCBSurface
    = new G4LogicalSkinSurface( "PCBSurface", logicPCB, fPCBSurface );

  // Setting the sensitive detector
  if( !fPMTSD ){
    fPMTSD = new LYSimPMTSD( "/LYSimPMT" );
    G4SDManager* sdman = G4SDManager::GetSDMpointer();
    sdman->AddNewDetector( fPMTSD );
  }
  logicSiPM->SetSensitiveDetector( fPMTSD );

  // Visual attributes
  logicWorld->SetVisAttributes( G4VisAttributes::Invisible );

  G4VisAttributes* SiPMVisAtt = new G4VisAttributes( G4Colour( 0, 0, 0 ) );
  SiPMVisAtt->SetForceSolid( true );
  SiPMVisAtt->SetVisibility( true );
  logicSiPM->SetVisAttributes( SiPMVisAtt );

  G4VisAttributes* CaseVisAtt = new G4VisAttributes( G4Colour( 0.8, 0.8, 0.8 ) );
  CaseVisAtt->SetForceSolid( true );
  CaseVisAtt->SetVisibility( true );
  logicSiPMCase->SetVisAttributes( CaseVisAtt );
  logicSiPMStand->SetVisAttributes( CaseVisAtt );

//  G4VisAttributes* ResinVisAtt = new G4VisAttributes( G4Colour( 0., 1., 1. ) );
//  ResinVisAtt->SetForceWireframe( true );
//  ResinVisAtt->SetVisibility( true );
//  logicSiPMResin->SetVisAttributes( ResinVisAtt );

  G4VisAttributes* TileVisAtt = new G4VisAttributes( G4Colour( 1., 1., 0. ) );
  TileVisAtt->SetForceWireframe( true );
  TileVisAtt->SetVisibility( true );
  logicTile->SetVisAttributes( TileVisAtt );

  mlogicTile->SetVisAttributes( TileVisAtt );

  G4VisAttributes* GreaseVisAtt = new G4VisAttributes( G4Colour( 1.0, 1.0, 1.0 ) );
  //GreaseVisAtt->SetForceWireframe( true );
  GreaseVisAtt->SetVisibility( true );
  mlogicGrease->SetVisAttributes( GreaseVisAtt );

  G4VisAttributes* WrapVisAtt = new G4VisAttributes( G4Colour( 0.5, 1., 0.5 ) );
  WrapVisAtt->SetForceWireframe( true );
  WrapVisAtt->SetVisibility( true );
  logicWrap->SetVisAttributes( WrapVisAtt );

  G4VisAttributes* PCBVisAtt = new G4VisAttributes( G4Colour( 0.0, 0.4, 0.1 ) );
  PCBVisAtt->SetForceSolid( true );
  PCBVisAtt->SetVisibility( true );
  logicPCB->SetVisAttributes( PCBVisAtt );

  return physWorld;
}

G4VSolid*
LYSimDetectorConstruction::ConstructTrapazoidSolid(
  const G4String& name,
  double          x,
  double          y,
  double          z,
  double          indent_x1,
  double          indent_x2 ) const
{
  const G4ThreeVector corners[8] = {
    G4ThreeVector( -x/2,           -y/2, -z/2 ),
    G4ThreeVector( x/2,            -y/2, -z/2 ),
    G4ThreeVector( -x/2+indent_x2, y/2,  -z/2 ),
    G4ThreeVector( x/2-indent_x1,  y/2,  -z/2 ),
    G4ThreeVector( -x/2,           -y/2, z/2 ),
    G4ThreeVector( x/2,            -y/2, z/2 ),
    G4ThreeVector( -x/2+indent_x2, y/2,  z/2 ),
    G4ThreeVector( x/2-indent_x1,  y/2,  z/2 )
  };

  return new G4Trap( name, corners );
}

G4VSolid*
LYSimDetectorConstruction::ConstructHollowWrapSolid() const
{
  G4VSolid* wrapOuter
    = ConstructTrapazoidSolid( "WrapOuter"
                             , _tilex + 2*wrapgap + 2*wrapthickness
                             , _tiley + 2*wrapgap + 2*wrapthickness
                             , _tilez*tile_number + tilegap*(tile_number-1) + 2*wrapgap + 2*wrapthickness
                             , 0, 0 );
  G4VSolid* wrapInner
    = ConstructTrapazoidSolid( "WrapInner"
                             , _tilex + 2*wrapgap
                             , _tiley + 2*wrapgap
                             , _tilez*tile_number + tilegap*(tile_number-1) + 2*wrapgap
                             , 0, 0 );
  G4VSolid* wrapbox = new G4SubtractionSolid( "WrapBox"
                                            , wrapOuter, wrapInner );
  G4VSolid* wraphole = new G4Tubs( "WrapHole"
                                 , 0, _pcb_radius
                                 , 2*wrapthickness
                                 , 0, 2*pi );

  const G4ThreeVector offset( 0, 0, 0.5*(_tilez*tile_number + tilegap*(tile_number-1)) + wrapgap + 0.5*wrapthickness );

  return new G4SubtractionSolid( "WrapSolid"
                               , wrapbox, wraphole
                               , NULL, offset );
}

G4VSolid*
LYSimDetectorConstruction::ConstructHollowWrapGapSolid() const
{
  G4VSolid* wrapgapOuter
    = ConstructTrapazoidSolid( "WrapGapOuter"
                             , _tilex + 2*wrapgap
                             , _tiley + 2*wrapgap
                             , _tilez*tile_number + tilegap*(tile_number-1) + 2*wrapgap
                             , 0, 0 );
  G4VSolid* wrapgapInner
    = ConstructTrapazoidSolid( "WrapGapInner"
                             , _tilex 
                             , _tiley
                             , _tilez*tile_number + tilegap*(tile_number-1)
                             , 0, 0 );
  G4VSolid* wrapgapbox = new G4SubtractionSolid( "WrapGapBox"
                                            , wrapgapOuter, wrapgapInner );
  G4VSolid* wrapgaphole = new G4Tubs( "WrapGapHole"
                                 , 0, _pcb_radius
                                 , 2*wrapthickness
                                 , 0, 2*pi );

  const G4ThreeVector offset( 0, 0, 0.5*(_tilez*tile_number + tilegap*(tile_number-1)) + wrapgap + 0.5*wrapthickness );

  return new G4SubtractionSolid( "WrapGapSolid"
                               , wrapgapbox, wrapgaphole
                               , NULL, offset );
}


G4VSolid*
LYSimDetectorConstruction::ConstructSphereDimpleSolid() const
{
  G4Sphere* solidsphere = new G4Sphere( "DimpleSphere"
                                      , 0, GetDimpleSizeRadius()
                                      , 0.,  2.*pi
                                      , pi/2, pi );

  G4Box* solidsub = new G4Box( "DimpleSphereSubBox"
                             , GetDimpleSizeRadius(), GetDimpleSizeRadius()
                             , GetDimpleSizeRadius() - GetDimpleIndent() );

  return new G4SubtractionSolid( "Dimple", solidsphere, solidsub );
}

G4VSolid*
LYSimDetectorConstruction::ConstructParabolicDimpleSolid() const
{
  const G4double SemiZ = ( 447317/288000 )*mm + _dimple_indent;
  const G4double SemiX = ( 447317/120000 )*mm;

  return new G4Ellipsoid( "DimpleParabolic",
    SemiX,
    SemiX,
    SemiZ,
    -SemiZ,// must be < SemiZ
    -SemiZ+_dimple_indent );// must be >-SemiZ
}

G4VSolid*
LYSimDetectorConstruction::ConstructPyramidDimpleSolid() const
{
  return new G4Trd( "DimplePyramid",
    0.0*mm, _dimple_radius,
    0.0*mm, _dimple_radius,
    _dimple_indent*0.5 );
}

G4VSolid*
LYSimDetectorConstruction::ConstructArbiDimpleSolid() const
{

    const double rr  = GetDimpleRadius();
    const double ind  = GetDimpleIndent();
    const double dimp_s  = GetDimpleSA();

    //derived from sphere LCE
    TF1 *f=new TF1("f","(TMath::Log((3-[0]-x)/[1]))/[2]/TMath::Log(2.7182818)",0,60); //55
    f->SetParameter(0,6.83141e+02*3/681.396);
    f->SetParameter(1,-4.94536e+02*3/681.396);
    f->SetParameter(2,-1.48631e-01);

    //derived from cut exponential
    TF1 *f3=new TF1("f3","(TMath::Log((3-[0]-x)/[1]))/[2]/TMath::Log(2.7182818)",0,60); //55
    f3->SetParameter(0,3);
    f3->SetParameter(1,-2.2826);
    f3->SetParameter(2,-1.34549e-01);
    //derived from cut exponential
    TF1 *f4=new TF1("f4","(TMath::Log((3-[0]-x)/[1]))/[2]",0,60); //55
    f4->SetParameter(0,3.0);
    f4->SetParameter(1,-2.35764);
    f4->SetParameter(2,-1.73561e-01);

    TF1 *f_up=new TF1("f_up","[0]+[1]*exp([2]*x)",0,60);
    f_up->SetParameter(0,3.0);
    f_up->SetParameter(1,-2.2826);
    f_up->SetParameter(2,-1.34549);

    double x_min = 3 - f_up->Eval(12); //12 f3
    double x_max = 3;
    const int x_n=601;
    G4double zLB[x_n] ;
    G4double rinLB[x_n];
    G4double routLB[x_n];
    zLB[0] = 0;
    rinLB[0] = 0;
    routLB[0] = f3->Eval(x_min) *mm;
    for(int i=1;i<=(x_n-1);i++){
        zLB[i] =  x_min*mm + ((i-1)*(x_max-x_min)*mm/(x_n-1));
        rinLB[i] = 0;
        if(f3->Eval(zLB[i])>0)
          routLB[i] = f3->Eval(zLB[i]) *mm;
        else
          routLB[i] = 0;
        zLB[i]*=-1;
    }

    return new G4Polycone("LittleBoySolid",0., 360.*deg, x_n, zLB, rinLB, routLB);
}

double
LYSimDetectorConstruction::GetDimpleSizeRadius() const
{
  return 0.5*( ( _dimple_radius*_dimple_radius )/_dimple_indent
               + _dimple_indent );
}

G4ThreeVector
LYSimDetectorConstruction::CalcDimpleOffset() const
{
  if( _dimple_type == PYRAMID ){
    return G4ThreeVector( 0, 0, 0.5*_tilez-0.5*_dimple_indent );
  } else if( _dimple_type == PARABOLIC ){
    return G4ThreeVector( 0, 0, 0.5*_tilez+( 447317/288000 )*mm );
  } else {
    return G4ThreeVector( 0, 0
                        , 0.5*_tilez+GetDimpleSizeRadius()-GetDimpleIndent() );
  }
}

G4ThreeVector
LYSimDetectorConstruction::CalcSiPMDimpleOffset() const
{
  if( _dimple_type == PYRAMID ){
    return G4ThreeVector( 0, 0, 0.5*_dimple_indent - 0.5*_sipm_z );
  } else if( _dimple_type == PARABOLIC ){
    return G4ThreeVector( 0, 0, 0 );// TO-DO!!
  } else {
    return G4ThreeVector( 0, 0,
      -GetDimpleSizeRadius() + GetDimpleIndent()- 0.5*_sipm_z );
  }
}

// Additional geometry factors
double
LYSimDetectorConstruction::WorldHalfX() const
{
  return _tilex * 1.5;
}

double
LYSimDetectorConstruction::WorldHalfY() const
{
  return _tiley *1.5;
}

double
LYSimDetectorConstruction::WorldHalfZ() const
{
  return _tilez * 10;
}

double
LYSimDetectorConstruction::LocalTileZ( const double x, const double y ) const
{
  const double dimple_r  = GetDimpleRadius();
  const double dimple_i  = GetDimpleIndent();
  const double dimple_s  = GetDimpleSA();

  const double beam_r    = sqrt( x*x+y*y );
  const double big_r     = GetDimpleSizeRadius();
  const double thickness = GetTileZ()-Gettgap()*(Gettn()-1);
  const int type         = GetDimpleType();

//**********************8
//add arbi shape 
//**********************8
/*
    //derived from sphere LCE
    TF1 *f=new TF1("f","[0]+[1]*exp([2]*x)",0,60);
    f->SetParameter(0,6.83141e+02*3/681.396);
    f->SetParameter(1,-4.94536e+02*3/681.396);
    f->SetParameter(2,-1.48631e-01);

    TF1 *f_up=new TF1("f_up","[0]+[1]*exp([2]*x)",0,60);
    f_up->SetParameter(0,3.00000e+00);
    f_up->SetParameter(1,-2.28260e+00);
    f_up->SetParameter(2,-1.34549e-01);
    // radius from 0 to 12.000408
    if(beam_r<12.000408){
      if( f_up->Eval(beam_r) >= 0) return  thickness - 3.0 + f_up->Eval(beam_r);
      else return 0;
    }else{
      return thickness;
    }

*/


  if( beam_r <= dimple_r ){
    if( type == SPHERICAL ){
      return thickness - ( sqrt( big_r*big_r - beam_r*beam_r ) -
                           ( big_r - dimple_i ) );
    } else if( type == PARABOLIC ){//  # Parabolic
      return thickness - abs( 1.35 * beam_r*beam_r - dimple_i );
    } else if( type == PYRAMID ){
      if( fabs( y ) <= fabs( x ) ){
        return thickness - ( dimple_i - ( dimple_i / dimple_r ) * fabs( x ) );
      } else {
        return thickness - ( dimple_i - ( dimple_i / dimple_r ) * fabs( y ) );
      }
    }  else {
      return thickness;
    }
  } else {
    return thickness;
  }
}


void
LYSimDetectorConstruction::SetTileAbsMult( const double mult )
{
  _absmult = mult;
  Update_EJ200_AbsLength( fEJ200, _absmult );
}
void
LYSimDetectorConstruction::SetSurfaceDimpleSA( const double mult )
{
  _dimplesa = mult;
  Update_Dimple_SigmaAlpha( fdimpleSurface, _dimplesa );
}
void
LYSimDetectorConstruction::SetWrapReflect( const double r )
{
  // Add entries into properties table
  _wrap_reflect = r;
  static const unsigned nentries = 2;
  static double phoE[nentries]   = {1.0*eV, 6.0*eV};
  double reflectivity[nentries]  = {_wrap_reflect, _wrap_reflect};

  G4MaterialPropertiesTable* table = fESROpSurface->GetMaterialPropertiesTable();
  if( table ){
    table->RemoveProperty( "REFLECTIVITY" );
    table->AddProperty( "REFLECTIVITY", phoE, reflectivity, nentries );
  } else {
    table = new G4MaterialPropertiesTable();
    table->AddProperty( "REFLECTIVITY", phoE, reflectivity, nentries );
    fESROpSurface->SetMaterialPropertiesTable( table );
  }

  static const unsigned nentriesty = 26;
  static double phoEty[nentriesty]   = {4.943804623    *eV,
4.69950395    *eV,
4.410663534    *eV,
4.305769181    *eV,
4.178097005    *eV,
3.98967036    *eV,
3.76463744    *eV,
3.504566212    *eV,
3.297986987    *eV,
3.134959329    *eV,
2.977941044    *eV,
2.859341507    *eV,
2.724238775    *eV,
2.569705793    *eV,
2.390512946    *eV,
2.26527967    *eV,
2.154957608    *eV,
2.036186213    *eV,
1.936699241    *eV,
1.839342514    *eV,
1.770863763    *eV,
1.705769441    *eV,
1.659646089    *eV,
1.609116456    *eV,
1.565425358    *eV,
1.550120187    *eV};

 double reflectivityty[nentriesty]  = {0.823738509    *_wrap_reflect,
0.842890705    *_wrap_reflect,
0.878426966    *_wrap_reflect,
0.906639428    *_wrap_reflect,
0.913942799    *_wrap_reflect,
0.933105209    *_wrap_reflect,
0.952277835    *_wrap_reflect,
0.96784474    *_wrap_reflect,
0.98340143    *_wrap_reflect,
0.988937692    *_wrap_reflect,
0.989948927    *_wrap_reflect,
0.992747702    *_wrap_reflect,
0.991940756    *_wrap_reflect,
0.993881512    *_wrap_reflect,
0.99587334    *_wrap_reflect,
0.996905005    *_wrap_reflect,
0.997037794    *_wrap_reflect,
0.999918284    *_wrap_reflect,
0.997334014    *_wrap_reflect,
0.998396323    *_wrap_reflect,
0.998518897    *_wrap_reflect,
1    *_wrap_reflect,
0.99691522    *_wrap_reflect,
0.995209397    *_wrap_reflect,
0.999846782    *_wrap_reflect,
1    *_wrap_reflect };


  G4MaterialPropertiesTable* tablet = fTyvekSurface->GetMaterialPropertiesTable();
  if( tablet ){
    tablet->RemoveProperty( "REFLECTIVITY" );
    tablet->AddProperty( "REFLECTIVITY", phoEty, reflectivityty, nentriesty );
  } else {
    tablet = new G4MaterialPropertiesTable();
    tablet->AddProperty( "REFLECTIVITY", phoEty, reflectivityty, nentriesty );
    fTyvekSurface->SetMaterialPropertiesTable( tablet );
  }
}

void
LYSimDetectorConstruction::SetPCBReflect( const double r )
{
  // Add entries into properties table
  const int nentries = 43;
  double phoE[nentries]   = {360, 370, 380, 390, 400, 410, 420, 430, 440, 450, 460, 470, 480, 490, 500, 510, 520, 530, 540, 550, 560, 570, 580, 590, 600, 610, 620, 630, 640, 650, 660, 670, 680, 690, 700, 710, 720, 730, 740, 750, 760, 770, 780 };
  double reflectivity[nentries] = {15.11, 15.4,      19.75,   27.935,   38.715,   52.025,   59.16,   62.25,   63.19,   63.44,   63.325,   63.18,   62.705,   62.295,   62.21,   62.035,   61.695,   61.26,   60.87,   60.505,   60.035,   59.485,   59.015,   58.535,   57.89,   57.295,   57.07,   56.525,   56.09,   55.68,   55.07,   54.65,   54.18,   53.72,   53.25,   52.83,   52.45,   52.195,   52.89,   54.16,   55.78,   57.59,   58.47};
  _pcb_reflectivity = r;
  for(int i=0;i<nentries;i++){
    phoE[i] = 1239.84193*eV/phoE[i];
    reflectivity[i] = reflectivity[i]*r/100.0;
  } 

  G4MaterialPropertiesTable* table = fPCBSurface->GetMaterialPropertiesTable();
  if( table ){
    table->RemoveProperty( "REFLECTIVITY" );
    table->AddProperty( "REFLECTIVITY", phoE, reflectivity, nentries );
  } else {
    table = new G4MaterialPropertiesTable();
    table->AddProperty( "REFLECTIVITY", phoE, reflectivity, nentries );
    fPCBSurface->SetMaterialPropertiesTable( table );
  }
}




