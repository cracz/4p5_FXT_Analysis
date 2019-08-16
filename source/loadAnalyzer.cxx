/**
 * \brief Helper script for runing picoDst analysis calling picoAnalyzer.cxx
 *
 * \brief Simplified this macro. Basically two tasks needed to be completed before
 * running picoAnalyzer.cxx alone. (1) Load libStPicoDst.so into ROOT environment;
 * (2) pre-compile picoAnalyzer.cxx with ROOT CINT in C++ mode.
 *
 **/

void loadAnalyzer()
{
  gROOT->ProcessLine(".L StRoot/StPicoEvent/libStPicoDst.so");
  gROOT->ProcessLine(".L picoAnalyzer.cxx++");
}
