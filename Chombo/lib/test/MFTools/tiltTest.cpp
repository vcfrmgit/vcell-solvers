#ifdef CH_LANG_CC
/*
 *      _______              __
 *     / ___/ /  ___  __ _  / /  ___
 *    / /__/ _ \/ _ \/  V \/ _ \/ _ \
 *    \___/_//_/\___/_/_/_/_.__/\___/
 *    Please refer to Copyright.txt, in Chombo's root directory.
 */
#endif

#include "TiltedCylinderIF.H"
#include "MFIndexSpace.H"

#include "EBISLayout.H"
#include "ParmParse.H"
#include "GeometryShop.H"
#include "PolyGeom.H"
#include "DebugDump.H"
#include "BRMeshRefine.H"
#include "LoadBalance.H"
#include "UsingNamespace.H"

int readGeometryInfo(Box& a_domain,
                     Real& a_dx,
                     RealVect& a_origin,
                     RealVect& a_direct,
                     RealVect& a_center,
                     Real& a_radius)
{

  // parse input file
  ParmParse pp;

  Vector<int> n_cell(SpaceDim);
  pp.getarr("n_cell",n_cell,0,SpaceDim);

  CH_assert(n_cell.size() == SpaceDim);

  IntVect lo = IntVect::Zero;
  IntVect hi;

  for (int ivec = 0; ivec < SpaceDim; ivec++)
    {
      if (n_cell[ivec] <= 0)
        {
          pout() << " bogus number of cells input = " << n_cell[ivec];
          return(-1);
        }
      hi[ivec] = n_cell[ivec] - 1;
    }

  a_domain.setSmall(lo);
  a_domain.setBig(hi);

  Vector<Real> prob_lo(SpaceDim, 1.0);
  Real prob_hi;

  pp.getarr("prob_lo",prob_lo,0,SpaceDim);
  pp.get("prob_hi",prob_hi);

  a_dx = (prob_hi-prob_lo[0])/n_cell[0];
  for (int idir = 0; idir < SpaceDim; idir++)
    {
      a_origin[idir] = prob_lo[idir];
    }

  pp.get("radius",a_radius);

  // ParmParse doesn't get RealVects, so work-around with Vector<Real>
  Vector<Real> vectorCenter;
  Vector<Real> vectorDirect;
  pp.getarr("center",vectorCenter,0,SpaceDim);
  pp.getarr("direct",vectorDirect,0,SpaceDim);
  for (int idir = 0; idir < SpaceDim; idir++)
    {
      a_center[idir] = vectorCenter[idir];
      a_direct[idir] = vectorDirect[idir];
    }

  return 0;
} // end read from file

int makeGeometry(MFIndexSpace& mfIndexSpace,
                 const Box& a_domain,
                 const Real& a_dx,
                 const RealVect& a_origin,
                 const RealVect& a_direct,
                 const RealVect& a_center,
                 const Real& a_radius)
{

  int eekflag = 0;
  Vector<GeometryService*> geometry(2, NULL);

  RealVect vectDx = RealVect::Unit;
  vectDx *= a_dx;

  TiltedCylinderIF  insideIF(a_radius, a_direct, a_center, true );
  GeometryShop workshop0(insideIF,0,vectDx);
  workshop0.m_phase = 0;
  geometry[0] = &workshop0;

  TiltedCylinderIF outsideIF(a_radius, a_direct, a_center, false);
  GeometryShop workshop1(outsideIF,0,vectDx);
  workshop1.m_phase=1;
  geometry[1] = &workshop1;

  mfIndexSpace.define(a_domain,a_origin,a_dx,geometry);

  return eekflag;
}

char iter_str[80];

//check stuff at irregular boundary.  vof0 comes from ebisBx0 fluid
//ebisBx1 is from the other fluid
int boundaryCheck(const VolIndex& a_vof0,
                  const EBISBox&  a_ebisBx0,
                  const EBISBox&  a_ebisBx1,
                 const Real&      a_tol)
{
  //  const IntVect& iv = a_vof0.gridIndex();
  int numBndry0 = a_ebisBx0.numFacePhase(a_vof0);
  for (int iirr0=0; iirr0 < numBndry0; iirr0++)
    {
      Real     bnArea0  = a_ebisBx0.bndryArea(    a_vof0, iirr0);
      RealVect centro0  = a_ebisBx0.bndryCentroid(a_vof0, iirr0);
      RealVect normal0  = a_ebisBx0.normal(       a_vof0, iirr0);
      VolIndex otherIndex0   = a_ebisBx0.faceIndex(    a_vof0, iirr0);
      //need to search through other faces's vofs to figure out
      //the correct one on the other side of this irregular face
      RealVect  centro1, normal1;
      Real      bnArea1;

      VolIndex vof1(otherIndex0);
      int numBndry1 = a_ebisBx1.numFacePhase(vof1);
      bool found = false;
      Vector<VolIndex> otherIndicies1(numBndry1);
      for (int iirr1=0; iirr1 < numBndry1; iirr1++)
        {
          VolIndex otherIndex1 = a_ebisBx1.faceIndex(vof1, iirr1);
          if (otherIndex1 == a_vof0)
            {
              found = true;
              bnArea1  = a_ebisBx1.bndryArea(    vof1, iirr1);
              centro1  = a_ebisBx1.bndryCentroid(vof1, iirr1);
              normal1  = a_ebisBx1.normal(       vof1, iirr1);
            }
          otherIndicies1[iirr1] = otherIndex1;
        }
      if (!found)
        {
          pout() << "mismatch of irregular faces" << endl;
          return -3;
        }
      if (Abs(bnArea1-bnArea0) > a_tol)
        {
          pout() << "mismatch of bndry areas" << endl;
          return -4;
        }
      for (int idir = 0; idir < SpaceDim; idir++)
        {
          if (Abs(Abs(normal1[idir]) - Abs(normal0[idir])) > a_tol)
            {
              pout() << "mismatch of normals" << endl;
              return -5;
            }
          if (Abs(Abs(centro1[idir])-Abs(centro0[idir])) > a_tol)
            {
              pout() << "mismatch of bndry centroids" << endl;
              return -6;
            }
        }
    }
  return 0;
}
int
checkConsistency(const MFIndexSpace&      a_mfIndexSpace,
                 const DisjointBoxLayout& a_grids,
                 const Box&               a_domain)
{
  int numPhases= a_mfIndexSpace.numPhases();
  CH_assert(numPhases==2);
  Vector<EBISLayout> layouts(2);
  a_mfIndexSpace.fillEBISLayout(layouts, a_grids, a_domain, 0);
#ifdef CH_USE_FLOAT
  Real tol = 1.0e-1;
#else
  Real tol = 1.0e-8;
#endif
  for (DataIterator dit = a_grids.dataIterator(); dit.ok(); ++dit)
    {
      const EBISBox& ebisBx0 = layouts[0][dit()];
      const EBISBox& ebisBx1 = layouts[1][dit()];
      IntVectSet commonIrreg = ebisBx0.getIrregIVS(a_grids[dit()]);
      commonIrreg           &= ebisBx1.getIrregIVS(a_grids[dit()]);

      for (IVSIterator ivsit(commonIrreg); ivsit.ok(); ++ivsit)
        {
          Vector<VolIndex> vofs0 = ebisBx0.getVoFs(ivsit());
          Vector<VolIndex> vofs1 = ebisBx1.getVoFs(ivsit());
          //see if total volume fraction is less than one
          Real totalVolFrac = 0;
          for (int ivof0 =0; ivof0 < vofs0.size(); ivof0++)
            {
              totalVolFrac += ebisBx0.volFrac(vofs0[ivof0]);
            }
          for (int ivof1 =1; ivof1 < vofs1.size(); ivof1++)
            {
              totalVolFrac += ebisBx1.volFrac(vofs1[ivof1]);
            }
          if (totalVolFrac > (1 + tol))
            {
              pout() << "volume fractions add up too high" << endl;
              return -1;
            }
          //see if area fractions add to less than one on each face
          for (int idir = 0; idir < SpaceDim; idir++)
            {
              for (SideIterator sit; sit.ok(); ++sit)
                {
                  Vector<FaceIndex> faces0 = ebisBx0.getAllFaces(ivsit(), idir, sit());
                  Vector<FaceIndex> faces1 = ebisBx1.getAllFaces(ivsit(), idir, sit());
                  Real totalAreaFrac = 0;
                  for (int iface0 =0; iface0 < faces0.size(); iface0++)
                    {
                      totalAreaFrac += ebisBx0.areaFrac(faces0[iface0]);
                    }
                  for (int iface1 =1; iface1 < faces1.size(); iface1++)
                    {
                      totalAreaFrac += ebisBx1.areaFrac(faces1[iface1]);
                    }
                  if (totalAreaFrac > (1 + tol))
                    {
                      pout() << "area fractions add up too high" << endl;
                      return -2;
                    }

                }
            }

          //see if the boundary areas and normals, centroids
          // on both sides of an irregular boundary match
          for (int ivof0 =0; ivof0 < vofs0.size(); ivof0++)
            {
              int bcheck = boundaryCheck(vofs0[ivof0], ebisBx0, ebisBx1, tol);
              if (bcheck != 0)
                {
                  pout() << "problem in checkBoundaryStuff 1" << endl;
                  return bcheck;
                }
            }
          //for symmetry as much as anything else, check same stuff
          //starting from the other side
          for (int ivof1 =0; ivof1 < vofs1.size(); ivof1++)
            {
              int bcheck = boundaryCheck(vofs1[ivof1], ebisBx1, ebisBx0, tol);
              if (bcheck != 0)
                {
                  pout() << "problem in checkBoundaryStuff 2" << endl;
                  return bcheck;
                }
            }
        }
    }
  return 0;
}

int
main(int argc,char **argv)
{
#ifdef CH_MPI
  MPI_Init(&argc, &argv);
#endif

  // begin forever present scoping trick
  {

    const char* in_file = "tilt.inputs";
    //parse input file
    ParmParse pp(0,NULL,NULL,in_file);
    Real radius;
    RealVect center;
    RealVect direct;
    RealVect origin;
    Real dx;
    Box domain;
    int eekflag = 0;
    MFIndexSpace mfIndexSpace;

    readGeometryInfo(domain,
                     dx,
                     origin,
                     direct,
                     center,
                     radius);

    Box domainFine(domain);
    ProblemDomain pFine(domain);

    CH_assert(eekflag == 0);

    Vector<Box> boxes;
    int maxsize;
    pp.get("maxboxsize", maxsize);
    domainSplit(domain, boxes, maxsize);

    Vector<int> procs(boxes.size(), 0);
    LoadBalance(procs, boxes);
    DisjointBoxLayout grids(boxes, procs);


    eekflag = makeGeometry(mfIndexSpace,
                           domain,
                           dx,
                           origin,
                           direct,
                           center,
                           radius);
    if (eekflag != 0)
      {
        pout() << "non zero eek detected = " << eekflag << endl;
        MayDay::Error("problem in makeGeometry");
      }
    eekflag = checkConsistency(mfIndexSpace, grids, domain);
#ifndef CH_USE_FLOAT
    if (eekflag != 0)
      {
        pout() << "non zero eek detected = " << eekflag << endl;
        MayDay::Error("problem in consistencyCheck");
      }
#endif

    pout() << "\n tilt test passed" << endl;
  } // end scoping trick

#ifdef CH_MPI
    MPI_Finalize();
#endif

    return 0;
}


