project(frontier)

#commented out files in SRC_FILES below are internal test files, not needed for library
set(SRC_FILES 
	#front/dtestfront.c 
	front/fadv.c
	front/fbdry1.c
	front/fbdry2.c
	front/fbdry3.c
	front/fbdry4.c
	front/fcheck3d.c
	front/fcorrspnd.c
	front/fcrosscur.c
	front/fcrossext.c
	front/fcrstatus.c
	front/fdiagnostic.c
	front/fgb2d.c
	front/fgb3comp.c
	front/fgb3d.c
	front/fgb3dutil.c
	front/fgrid.c
	front/finit.c
	front/fint.c
	front/fmap.c
	front/fnode.c
	front/fnodesub.c
	front/foverture_adv.c
	front/foverture_patch.c
	front/fprint.c
	front/fprop2d.c
	front/fprop3d.c
	front/fredist.c
	front/fredist1d.c
	front/fredist2d.c
	front/fredist3d.c
	front/frp1.c
	front/frp2.c
	front/fscat1d.c
	front/fscat2d.c
	front/fscat3d1.c
	front/fscat3d2.c
	front/fscatter.c
	front/fstate.c
	front/fstate2d.c
	front/fsub.c
	front/ftop.c
	front/funtan2d.c
	front/funtan3d.c
	front/fuserhooks.c
	front/fuserintfc.c
	front/fvelo.c
	#front/testfront.c
	#front/tt.c
	intfc/arrayutils.c
	intfc/comp.c
	intfc/comp1d.c
	intfc/comp2d.c
	intfc/comp3d.c
	intfc/cross2d.c
	intfc/geomutils.c
	intfc/iblkb.c
	intfc/iblkc2.c
	intfc/iblkc3.c
	intfc/icheck3d.c
	intfc/idiagnostic.c
	intfc/iecomps.c
	intfc/ifourier.c
	intfc/igrid.c
	intfc/igview.c
	intfc/iMesh.c
	intfc/iMeshP.c
	intfc/imkcurve.c
	intfc/imksurf.c
	intfc/int3d.c
	intfc/intfc.c
	intfc/intfc_amr.c
	intfc/iprt3d.c
	intfc/iredist.c
	intfc/iredist_o2.c
	intfc/irefl.c
	intfc/iscatter.c
	intfc/isect2d.c
	intfc/isect3d.c
	intfc/isub.c
	intfc/isurgery.c
	intfc/itrisset.c
	intfc/iuserintfc.c
	intfc/iwallsurf.c
	intfc/ixgraph.c
	intfc/map.c
	intfc/ppcopy.c
	intfc/setb1d.c
	intfc/setb2d.c
	intfc/setb3d.c
	intfc/shift.c
	#intfc/testintfc.c
	intfc/top.c
	intfc/triangle.c
	intfc/trisurf.c
	intfc/userhooks.c
	intfc/zoom.c
	tri/overture_trigrid1.c
	tri/tri1d.c
	tri/tri2d.c
	tri/tri3d.c
	tri/tri3dutils.c
	tri/tricpy.c
	tri/triel1.c
	tri/triel2.c
	tri/triel3.c
	tri/trigrid1.c
	tri/trigrid2.c
	tri/triloc.c
	tri/tripcs.c
	tri/triprint.c
	tri/trisurgery.c
	tri/triuserintfc.c
	util/cleanup.c
	util/debug.c
	util/error.c
	util/fft.c
	util/fgetstrin.c
	util/fsort.c
	util/machine.c
	util/matrix.c
	util/other.c
	util/output.c
	util/ppsub.c
	util/quad.c
	util/roots.c
	util/runga.c
	util/screen.c
	util/sigplot.c
	util/simpleio.c
	util/sphhar.c
	util/times.c
	util/uinit.c
	util/vectormalloc.c
	util/vtk.c
	windows_math_func.c)
if (APPLE)
   include_directories(/usr/include/malloc) 
endif (APPLE)

set_source_files_properties( ${SRC_FILES} PROPERTIES 
	LANGUAGE C
	COMPILE_FLAGS "-DONED -DTWOD -DTHREED")
include_directories(. front intfc tri util)
add_library(frontier STATIC ${SRC_FILES})
target_include_directories(frontier PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/util>
  $<INSTALL_INTERFACE:b>  # <prefix>
  $<INSTALL_INTERFACE:util>  # <prefix>/util
)
install(TARGETS frontier 
	ARCHIVE DESTINATION bin)
