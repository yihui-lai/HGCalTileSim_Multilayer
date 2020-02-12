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
#include "G4UnionSolid.hh"
#include "G4UnitsTable.hh"
#include "G4VisAttributes.hh"
#include "G4VPhysicalVolume.hh"

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
  _tilez        = 3.8*mm;//3.0*mm;
  _tile_x1      = 0.0*mm;
  _tile_x2      = 0.0*mm;
  wrapgap       = 0.065*mm;
  wrapthickness = 0.1*mm;

  tilegap = 0.5*mm;//another probability 0.1*mm
  tile_number = 3;
  is_ESR = true;

  _absmult      = 1;
  _wrap_reflect = 0.985;

  _sipm_deadwidth  = 0.2*mm;
  _sipm_x          = 1.4*mm;
  _sipm_y          = 1.4*mm;
  _sipm_z          = 0.4*mm;
  _sipm_rimwidth   = 0.3*mm;
  _sipm_glasswidth = 0.1*mm;
  _sipm_standz     = 0.3*mm;

  // Default Dimple settings
  _dimple_type   = SPHERICAL;// 0: Normal, 1: Pyramid, 2: Parabolic
  _dimple_indent = 2*mm;//1.6*mm;
  _dimple_radius = 15*mm;//3.5*mm;// 3.4409*mm

  // Default Hole settings
  _pcb_radius       = 2.3;
  _pcb_reflectivity = 0.5;


  // Defining material list.
  fBialkali = Make_Bialkali();
  fEpoxy    = Make_Epoxy();
  fAir      = Make_Custom_Air();
  fEJ200    = Make_EJ200();
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

  // The matrial of the wrap isn't as important as the surface
  G4LogicalVolume* logicWrap = new G4LogicalVolume( solidWrap, fEpoxy,  "Wrap" );
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

  // Exposed PCB Back plane
  const G4ThreeVector pcb_offset( 0, 0
                                , 0.5*_tilez + wrapgap + 2*wrapthickness );

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

  G4VSolid* hollowedTile = new G4SubtractionSolid( "TileSolid"
                                                 , solidTile, solidTileSubtract
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


  ///////////////////////////////////////////////////////////////////////////////
  // put more tile layers and grease
  ///////////////////////////////////////////////////////////////////////////////

  G4LogicalVolume* mlogicTile= new G4LogicalVolume( solidTile
                                               , fEJ200, "mTileLogic" );
//  G4VPhysicalVolume* mphysTile[tile_number-1];
  std::vector<G4VPhysicalVolume*> mphysTileList;

  G4VSolid* msolidGrease = ConstructTrapazoidSolid( "GreaseTrap"
                                               , _tilex
                                               , _tiley
                                               , tilegap
                                               , _tile_x1
                                               , _tile_x2 );
  G4LogicalVolume* mlogicGrease= new G4LogicalVolume( msolidGrease
                                               , fBC_630_grease, "GreaseLogic" );
//  G4VPhysicalVolume* mphysGrease[tile_number-1];
  std::vector<G4VPhysicalVolume*> mphysGreaseList;

  for(int i=1; i<tile_number; i++){
/*
      mphysGrease[i-1] = new G4PVPlacement( 0
                                               , G4ThreeVector( 0, 0, -1*(_tilez+tilegap)*i+0.5*(_tilez+tilegap) )
                                               , mlogicGrease
                                               , "GreasePhysic"
                                               , logicWorld
                                               , false
                                               , 0
                                               , checkOverlaps );
*/
      mphysGreaseList.push_back( new G4PVPlacement(0, G4ThreeVector( 0, 0, -1*(_tilez+tilegap)*i+0.5*(_tilez+tilegap) ), mlogicGrease
                                               , "GreasePhysic"
                                               , logicWorld
                                               , false
                                               , 0
                                               , checkOverlaps)); 

/*
      mphysTile[i-1] = new G4PVPlacement( 0
                                               , G4ThreeVector( 0, 0, -1*(_tilez+tilegap)*i )
                                               , mlogicTile
                                               , "TilePhysic"
                                               , logicWorld
                                               , false
                                               , 0
                                               , checkOverlaps );
*/
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
                                , +0.5*_tilez - 0.5*_sipm_z - _sipm_standz );
  const G4ThreeVector ResinOffset( 0, 0
                                 , +0.5*_tilez - 0.5*_sipm_z - _sipm_standz
                                   - _sipm_glasswidth );
  const G4ThreeVector StandOffset( 0, 0, +0.5*_tilez - 0.5*_sipm_standz );

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



  ///////////////////////////////////////////////////////////////////////////////
  // Defining surfaces
  ///////////////////////////////////////////////////////////////////////////////
  G4LogicalBorderSurface* WrapAirSurface;
  if (is_ESR){
    WrapAirSurface = new G4LogicalBorderSurface( "WrapAirSurface"
                              , physWorld
                              , physWrap
                              , fESROpSurface );
  }else{
    WrapAirSurface = new G4LogicalBorderSurface( "WrapAirSurface"
                              , physWorld
                              , physWrap
                              , fTyvekSurface );
  }

  G4LogicalBorderSurface* TileSurface =
    new G4LogicalBorderSurface( "TileSurface"
                              , physTile
                              , physWorld
                              , fIdealPolishedOpSurface );
  //G4LogicalBorderSurface* mTileSurface[tile_number-1];
  std::vector<G4LogicalBorderSurface*> mTileSurfaceList;
  std::vector<G4LogicalBorderSurface*> mGreaseSurfaceList;

  for(int i=1; i<tile_number; i++){
    mTileSurfaceList.push_back( new G4LogicalBorderSurface(
                               "mTileSurface"
                              , mphysTileList.at(i-1)
                              , physWorld
                              , fIdealPolishedOpSurface) );    


    mGreaseSurfaceList.push_back( new G4LogicalBorderSurface(
                               "mGreaseSurface"
                              , mphysGreaseList.at(i-1)
                              , physWorld
                              , fIdealPolishedOpSurface) );

/*    new G4LogicalBorderSurface( "mTileSurface"
                              , mphysTile[i-1]
                              , physWorld
                              , fIdealPolishedOpSurface );
*/
  }
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

  G4VisAttributes* ResinVisAtt = new G4VisAttributes( G4Colour( 0., 1., 1. ) );
  ResinVisAtt->SetForceWireframe( true );
  ResinVisAtt->SetVisibility( true );
  logicSiPMResin->SetVisAttributes( ResinVisAtt );

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
  const double beam_r    = sqrt( x*x+y*y );
  const double big_r     = GetDimpleSizeRadius();
  const double thickness = GetTileZ();
  const int type         = GetDimpleType();

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

}

void
LYSimDetectorConstruction::SetPCBReflect( const double r )
{
  // Add entries into properties table
  _pcb_reflectivity = r;
  static const unsigned nentries = 2;
  static double phoE[nentries]   = {1.0*eV, 6.0*eV};
  double reflectivity[nentries]  = {r, r};

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
