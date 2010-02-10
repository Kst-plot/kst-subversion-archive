SET(tutorialFolder share/apps/kst/tutorial)

ADD_CUSTOM_COMMAND(
  PRE_BUILD 
  OUTPUT gyrodata.dat 
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/gyrodata.dat.sh ${CMAKE_SOURCE_DIR}
)

ADD_CUSTOM_TARGET("gyrodata" ALL DEPENDS gyrodata.dat)

########### install files ###############

install(FILES  "all.kst" DESTINATION ${tutorialFolder})
install(FILES  "boomB.dat" DESTINATION ${tutorialFolder})
install(FILES  "cbi.dat" DESTINATION ${tutorialFolder})
install(FILES  "dasi.dat" DESTINATION ${tutorialFolder})
install(FILES  "boomAB.dat" DESTINATION ${tutorialFolder})
install(FILES  "boom.dat" DESTINATION ${tutorialFolder})
install(FILES  "combined.dat" DESTINATION ${tutorialFolder})
install(FILES  "demo.kst" DESTINATION ${tutorialFolder})
install(FILES  "maxima.dat" DESTINATION ${tutorialFolder})
install(FILES  "boomA.dat" DESTINATION ${tutorialFolder})
install(FILES  "boom_lce.dat" DESTINATION ${tutorialFolder})
install(FILES  "gyrodata.dat" DESTINATION ${tutorialFolder})
install(FILES  "vsa.dat" DESTINATION ${tutorialFolder})
