/*                                                                           
Developed by Sandeep Sharma and Garnet K.-L. Chan, 2012                      
Copyright (c) 2012, Garnet K.-L. Chan                                        
                                                                             
This program is integrated in Molpro with the permission of 
Sandeep Sharma and Garnet K.-L. Chan
*/


#ifndef SPIN_OPERATORFUNCTIONS_HEADER_H
#define SPIN_OPERATORFUNCTIONS_HEADER_H
#include "StateInfo.h"
#include "StackBaseOperator.h"
#include "timer.h"
#include "Stackspinblock.h"
#include "MatrixBLAS.h"
#include <math.h>
#include "global.h"
#include "StackMatrix.h"
#include <omp.h>
#include <iostream>
#include <map>
#include <vector>


#define TINY 1.e-20

namespace SpinAdapted{
namespace operatorfunctions
{

  //TENSOR TRACE A x I  ->  C
template<class B, class T1, class T2, class T3> void TensorTraceElement(const B *ablock, const T1& a, const B *cblock, const StateInfo *cstateinfo, T2& c, T3& cel, int cq, int cqprime, double scale)
{
  if (fabs(scale) < TINY) return;
  assert(c.allowed(cq, cqprime));
    
  int aq, aqprime, bq, bqprime, bstates;
  const char conjC = (ablock == cblock->get_leftBlock()) ? 'n' : 't';

  const std::vector<int> oldToNewI = cstateinfo->oldToNewState.at(cq);
  const std::vector<int> oldToNewJ = cstateinfo->oldToNewState.at(cqprime);

  const StateInfo* rS = cstateinfo->rightStateInfo, *lS = cstateinfo->leftStateInfo;
  int rowstride =0, colstride = 0;

  for (int oldi =0; oldi < oldToNewI.size(); oldi++) {
    colstride = 0;
    for (int oldj = 0; oldj < oldToNewJ.size(); oldj++)
    {
      if (conjC == 'n')
      {
	aq = cstateinfo->leftUnMapQuanta[ oldToNewI[oldi] ];
	aqprime = cstateinfo->leftUnMapQuanta[ oldToNewJ[oldj] ];
	bq = cstateinfo->rightUnMapQuanta[ oldToNewI[oldi] ];
	bqprime = cstateinfo->rightUnMapQuanta[ oldToNewJ[oldj] ];
	bstates = cstateinfo->rightStateInfo->getquantastates(bq);
      }
      else 
      {
	aq = cstateinfo->rightUnMapQuanta[ oldToNewI[oldi] ];
	aqprime = cstateinfo->rightUnMapQuanta[ oldToNewJ[oldj] ];
	bq = cstateinfo->leftUnMapQuanta[ oldToNewI[oldi] ];
	bqprime = cstateinfo->leftUnMapQuanta[ oldToNewJ[oldj] ];
	bstates = cstateinfo->leftStateInfo->getquantastates(bq);
      }
      
      if (a.allowed(aq, aqprime) && (bq == bqprime))
      {
	DiagonalMatrix unitMatrix(bstates);
	unitMatrix = 1.;

	Matrix unity(bstates, bstates);
	unity = unitMatrix;

	if (conjC == 'n')
	{
	  double scaleb = dmrginp.get_ninej()(lS->quanta[aqprime].get_s().getirrep() , rS->quanta[bqprime].get_s().getirrep(), cstateinfo->quanta[cqprime].get_s().getirrep(), 
				a.get_spin().getirrep(), 0, c.get_spin().getirrep(),
				lS->quanta[aq].get_s().getirrep() , rS->quanta[bq].get_s().getirrep(), cstateinfo->quanta[cq].get_s().getirrep());

	  scaleb *= Symmetry::spatial_ninej(lS->quanta[aqprime].get_symm().getirrep() , rS->quanta[bqprime].get_symm().getirrep(), cstateinfo->quanta[cqprime].get_symm().getirrep(), 
			       a.get_symm().getirrep(), 0, c.get_symm().getirrep(),
			       lS->quanta[aq].get_symm().getirrep() , rS->quanta[bq].get_symm().getirrep(), cstateinfo->quanta[cq].get_symm().getirrep());

	  MatrixTensorProduct (a.operator_element(aq, aqprime), a.conjugacy(), scale, unity, 'n', scaleb, 
	  	       cel, rowstride, colstride);
	}
	else {
	  double scaleb = dmrginp.get_ninej()(lS->quanta[bqprime].get_s().getirrep(), rS->quanta[aqprime].get_s().getirrep() , cstateinfo->quanta[cqprime].get_s().getirrep(), 
				0, a.get_spin().getirrep(), c.get_spin().getirrep(),
				lS->quanta[bq].get_s().getirrep(), rS->quanta[aq].get_s().getirrep() , cstateinfo->quanta[cq].get_s().getirrep());
	  scaleb *= Symmetry::spatial_ninej(lS->quanta[bqprime].get_symm().getirrep() , rS->quanta[aqprime].get_symm().getirrep(), cstateinfo->quanta[cqprime].get_symm().getirrep(), 
			       0, a.get_symm().getirrep(), c.get_symm().getirrep(),
			       lS->quanta[bq].get_symm().getirrep() , rS->quanta[aq].get_symm().getirrep(), cstateinfo->quanta[cq].get_symm().getirrep());
	  if (a.get_fermion() && IsFermion (cstateinfo->leftStateInfo->quanta[bqprime]) ) scaleb *= -1.;
	  MatrixTensorProduct (unity, 'n', scaleb, a.operator_element(aq, aqprime), a.conjugacy(), scale, 
	  		       cel, rowstride, colstride);
	}
      }
      colstride += cstateinfo->unCollectedStateInfo->quantaStates[ oldToNewJ[oldj] ];

    }
    rowstride += cstateinfo->unCollectedStateInfo->quantaStates[ oldToNewI[oldi] ];
    
  }
}

  //TENSOR TRACE A x I  ->  C
template<class B, class T1, class T2, class T3> void TensorTraceElement(const B *ablock, const T1& a, const B *cblock, const StateInfo *cstateinfo, T2& c, T3& cel, int cq, int cqprime)
{
  TensorTraceElement(ablock, a, cblock, cstateinfo, c, cel, cq, cqprime, 1.);
}

//TENSOR TRACE A x I  ->  C  
template<class B, class T1, class T2> void TensorTrace(const B *ablock, const T1& a, const B *cblock, const StateInfo *cstateinfo, T2& c, double scale= 1.0, int num_thrds = 1) {
  
  if (fabs(scale) < TINY) return;
  assert (a.get_initialised() && c.get_initialised());
  
  std::vector< std::pair<std::pair<int, int>, StackMatrix> >& nonZeroBlocks = c.get_nonZeroBlocks();

  //#pragma omp parallel for schedule(dynamic)
  for (int index = 0; index<nonZeroBlocks.size(); index++) {
    int cq = nonZeroBlocks[index].first.first, cqprime = nonZeroBlocks[index].first.second;
    TensorTraceElement(ablock, a, cblock, cstateinfo, c, nonZeroBlocks[index].second, cq, cqprime, scale);
  }

}

//***********************************************************


//TENSOR TRACE A x I -> cD  
template<class B, class T1> void TensorTrace (const B *ablock, const T1& a, const B* cblock, const StateInfo* cstateinfo, DiagonalMatrix& cDiagonal, Real scale)
{
  if (fabs(scale) < TINY) return;
  try
    {
      assert (a.get_initialised());
      const char conjC = (ablock == cblock->get_leftBlock()) ? 'n' : 't';

      const int aSz = ablock->get_stateInfo().quanta.size ();
      const int bSz = (conjC == 'n') ? cblock->get_stateInfo().rightStateInfo->quanta.size () : cblock->get_stateInfo().leftStateInfo->quanta.size ();
      const StateInfo& s = cblock->get_stateInfo();

      const StateInfo* lS = s.leftStateInfo, *rS = s.rightStateInfo;

      for (int aQ = 0; aQ < aSz; ++aQ)
	if (a.allowed(aQ, aQ))
	  for (int bQ = 0; bQ < bSz; ++bQ)
	    if (s.allowedQuanta (aQ, bQ, conjC))
	      {
		int cQ = s.quantaMap (aQ, bQ, conjC)[0];
		for (int cQState = 0; cQState < s.quantaStates [cQ]; ++cQState)
		  {
		    Real scaleB = 1.0;
		    int aQState;
		    int bQState;
		    
		    if (conjC == 'n')
		      {
			s.UnMapQuantumState (cQState, s.rightStateInfo->quantaStates [bQ], aQState, bQState);
			scaleB *= dmrginp.get_ninej()(lS->quanta[aQ].get_s().getirrep() , rS->quanta[bQ].get_s().getirrep(), cstateinfo->quanta[cQ].get_s().getirrep(), 
				a.get_spin().getirrep(), 0, 0,
				lS->quanta[aQ].get_s().getirrep() , rS->quanta[bQ].get_s().getirrep(), cstateinfo->quanta[cQ].get_s().getirrep());
			scaleB *= Symmetry::spatial_ninej(lS->quanta[aQ].get_symm().getirrep() , rS->quanta[bQ].get_symm().getirrep(), cstateinfo->quanta[cQ].get_symm().getirrep(), 
					   a.get_symm().getirrep(), 0, 0,
					   lS->quanta[aQ].get_symm().getirrep() , rS->quanta[bQ].get_symm().getirrep(), cstateinfo->quanta[cQ].get_symm().getirrep());
		      }
		    else
		      {
			scaleB *= dmrginp.get_ninej()(lS->quanta[bQ].get_s().getirrep() , rS->quanta[aQ].get_s().getirrep(), cstateinfo->quanta[cQ].get_s().getirrep(), 
					0, a.get_spin().getirrep(), 0,
				lS->quanta[bQ].get_s().getirrep() , rS->quanta[aQ].get_s().getirrep(), cstateinfo->quanta[cQ].get_s().getirrep());
			scaleB *= Symmetry::spatial_ninej(lS->quanta[bQ].get_symm().getirrep() , rS->quanta[aQ].get_symm().getirrep(), cstateinfo->quanta[cQ].get_symm().getirrep(), 
					     0, a.get_symm().getirrep(), 0,
					     lS->quanta[bQ].get_symm().getirrep() , rS->quanta[aQ].get_symm().getirrep(), cstateinfo->quanta[cQ].get_symm().getirrep());
			if (a.get_fermion()&& IsFermion(lS->quanta[bQ])) scaleB *= -1.0;
			s.UnMapQuantumState (cQState, s.rightStateInfo->quantaStates [aQ], bQState, aQState);
		      }
		    long dindex = s.unBlockedIndex [cQ] + cQState + 1;
		    cDiagonal(dindex) += scale * scaleB * a.operator_element(aQ, aQ)(aQState + 1, aQState + 1); 

		  }
    }
    }
  catch (Exception)
    {
      pout << Exception::what () << endl;
      abort ();
    }
}

//*****************************************************


//TENSOR PRODUCT A x B -> C
 template<class T1, class T2, class T3, class T4> void TensorProductElement(const T1& a, const T2& b, const StateInfo *brastateinfo, const StateInfo *ketstateinfo, T3& c, T4& cel, int cq, int cqprime, bool aIsLeftOp, double scale)
{
  if (fabs(scale) < TINY) return;
  assert (a.get_initialised());
  assert (b.get_initialised());
  assert (c.get_initialised());
 
  const std::vector<int>& oldToNewI = brastateinfo->oldToNewState.at(cq);
  const std::vector<int>& oldToNewJ = ketstateinfo->oldToNewState.at(cqprime);
 
  const char conjC = aIsLeftOp ? 'n' : 't';
  
  const StateInfo* lbraS = brastateinfo->leftStateInfo, *rbraS = brastateinfo->rightStateInfo;
  const StateInfo* lketS = ketstateinfo->leftStateInfo, *rketS = ketstateinfo->rightStateInfo;
  int rowstride = 0, colstride = 0;

  int aq, aqprime, bq, bqprime;

  //pout << "old to new size "<<oldToNewI.size()<<" "<<oldToNewJ.size()<<endl;
  for (int oldi =0; oldi < oldToNewI.size(); oldi++) {
    colstride = 0;
    for (int oldj = 0; oldj < oldToNewJ.size(); oldj++)
    {
      if (conjC == 'n')
      {
	aq = brastateinfo->leftUnMapQuanta[ oldToNewI[oldi] ];
	aqprime = ketstateinfo->leftUnMapQuanta[ oldToNewJ[oldj] ];
	bq = brastateinfo->rightUnMapQuanta[ oldToNewI[oldi] ];
	bqprime = ketstateinfo->rightUnMapQuanta[ oldToNewJ[oldj] ];
      }
      else 
      {
	aq = brastateinfo->rightUnMapQuanta[ oldToNewI[oldi] ];
	aqprime = ketstateinfo->rightUnMapQuanta[ oldToNewJ[oldj] ];
	bq = brastateinfo->leftUnMapQuanta[ oldToNewI[oldi] ];
	bqprime = ketstateinfo->leftUnMapQuanta[ oldToNewJ[oldj] ];
      }
  
      Real scaleA = scale;
      Real scaleB = 1.0;
      if (a.allowed(aq, aqprime) && b.allowed(bq, bqprime))
      {
	if (conjC == 'n')
	{
	  scaleB = dmrginp.get_ninej()(lketS->quanta[aqprime].get_s().getirrep() , rketS->quanta[bqprime].get_s().getirrep(), ketstateinfo->quanta[cqprime].get_s().getirrep(), 
			 a.get_spin().getirrep(), b.get_spin().getirrep(), c.get_spin().getirrep(),
			 lbraS->quanta[aq].get_s().getirrep() , rbraS->quanta[bq].get_s().getirrep(), brastateinfo->quanta[cq].get_s().getirrep());
	  scaleB *= Symmetry::spatial_ninej(lketS->quanta[aqprime].get_symm().getirrep() , rketS->quanta[bqprime].get_symm().getirrep(), ketstateinfo->quanta[cqprime].get_symm().getirrep(), 
			       a.get_symm().getirrep(), b.get_symm().getirrep(), c.get_symm().getirrep(),
			       lbraS->quanta[aq].get_symm().getirrep() , rbraS->quanta[bq].get_symm().getirrep(), brastateinfo->quanta[cq].get_symm().getirrep());
	  scaleB *= b.get_scaling(rbraS->quanta[bq], rketS->quanta[bqprime]);
	  scaleA *= a.get_scaling(lbraS->quanta[aq], lketS->quanta[aqprime]);
	  if (b.get_fermion() && IsFermion (ketstateinfo->leftStateInfo->quanta [aqprime])) scaleB *= -1;
	  MatrixTensorProduct (a.operator_element(aq, aqprime), a.conjugacy(), scaleA, 
			       b.operator_element(bq, bqprime), b.conjugacy(), scaleB, cel,rowstride, colstride);
	}
	else
	{
	  scaleB = dmrginp.get_ninej()(lketS->quanta[bqprime].get_s().getirrep(), rketS->quanta[aqprime].get_s().getirrep() , ketstateinfo->quanta[cqprime].get_s().getirrep(), 
			 b.get_spin().getirrep(), a.get_spin().getirrep(), c.get_spin().getirrep(),
			 lbraS->quanta[bq].get_s().getirrep(), rbraS->quanta[aq].get_s().getirrep() , brastateinfo->quanta[cq].get_s().getirrep());
	  scaleB *= Symmetry::spatial_ninej(lketS->quanta[bqprime].get_symm().getirrep() , rketS->quanta[aqprime].get_symm().getirrep(), ketstateinfo->quanta[cqprime].get_symm().getirrep(), 
			       b.get_symm().getirrep(), a.get_symm().getirrep(), c.get_symm().getirrep(),
			       lbraS->quanta[bq].get_symm().getirrep() , rbraS->quanta[aq].get_symm().getirrep(), brastateinfo->quanta[cq].get_symm().getirrep());
	  scaleB *= b.get_scaling(lbraS->quanta[bq], lketS->quanta[bqprime]);
	  scaleA *= a.get_scaling(rbraS->quanta[aq], rketS->quanta[aqprime]);
	  if (a.get_fermion() && IsFermion (ketstateinfo->leftStateInfo->quanta[bqprime]) ) scaleB *= -1.;
	  
	  MatrixTensorProduct (b.operator_element(bq, bqprime), b.conjugacy(), scaleB, 
			       a.operator_element(aq, aqprime), a.conjugacy(), scaleA, cel, rowstride, colstride);
	}
      }
      colstride += ketstateinfo->unCollectedStateInfo->quantaStates[ oldToNewJ[oldj] ];

    }
    rowstride += brastateinfo->unCollectedStateInfo->quantaStates[ oldToNewI[oldi] ];
    
  }

  
}

//TENSOR PRODUCT A x B -> C
 template<class B, class T1, class T2, class T3, class T4> void TensorProductElement(const B *ablock, const T1& a, const T2& b, const B *cblock, const StateInfo *cstateinfo, T3& c, T4& cel, int cq, int cqprime, double scale)
{
  //cstateinfo is not used
  //This function can be used for different bra and ket stateinfo.
  if (fabs(scale) < TINY) return;
  assert (a.get_initialised());
  assert (b.get_initialised());
  assert (c.get_initialised());
 
  const StateInfo *ketstateinfo = &cblock->get_ketStateInfo(), 
    *brastateinfo = &cblock->get_braStateInfo();

  const B* bblock = (cblock->get_leftBlock() == ablock) ? cblock->get_rightBlock() : cblock->get_leftBlock();
 
  const std::vector<int>& oldToNewI = brastateinfo->oldToNewState.at(cq);
  const std::vector<int>& oldToNewJ = ketstateinfo->oldToNewState.at(cqprime);
 
  const char conjC = (cblock->get_leftBlock() == ablock) ? 'n' : 't';
  
  const StateInfo* lbraS = brastateinfo->leftStateInfo, *rbraS = brastateinfo->rightStateInfo;
  const StateInfo* lketS = ketstateinfo->leftStateInfo, *rketS = ketstateinfo->rightStateInfo;
  int rowstride = 0, colstride = 0;

  int aq, aqprime, bq, bqprime;

  //pout << "old to new size "<<oldToNewI.size()<<" "<<oldToNewJ.size()<<endl;
  for (int oldi =0; oldi < oldToNewI.size(); oldi++) {
    colstride = 0;
    for (int oldj = 0; oldj < oldToNewJ.size(); oldj++)
    {
      if (conjC == 'n')
      {
	aq = brastateinfo->leftUnMapQuanta[ oldToNewI[oldi] ];
	aqprime = ketstateinfo->leftUnMapQuanta[ oldToNewJ[oldj] ];
	bq = brastateinfo->rightUnMapQuanta[ oldToNewI[oldi] ];
	bqprime = ketstateinfo->rightUnMapQuanta[ oldToNewJ[oldj] ];
      }
      else 
      {
	aq = brastateinfo->rightUnMapQuanta[ oldToNewI[oldi] ];
	aqprime = ketstateinfo->rightUnMapQuanta[ oldToNewJ[oldj] ];
	bq = brastateinfo->leftUnMapQuanta[ oldToNewI[oldi] ];
	bqprime = ketstateinfo->leftUnMapQuanta[ oldToNewJ[oldj] ];
      }
  
      Real scaleA = scale;
      Real scaleB = 1.0;
      if (a.allowed(aq, aqprime) && b.allowed(bq, bqprime))
      {
	if (conjC == 'n')
	{
	  scaleB = dmrginp.get_ninej()(lketS->quanta[aqprime].get_s().getirrep() , rketS->quanta[bqprime].get_s().getirrep(), ketstateinfo->quanta[cqprime].get_s().getirrep(), 
			 a.get_spin().getirrep(), b.get_spin().getirrep(), c.get_spin().getirrep(),
			 lbraS->quanta[aq].get_s().getirrep() , rbraS->quanta[bq].get_s().getirrep(), brastateinfo->quanta[cq].get_s().getirrep());
	  scaleB *= Symmetry::spatial_ninej(lketS->quanta[aqprime].get_symm().getirrep() , rketS->quanta[bqprime].get_symm().getirrep(), ketstateinfo->quanta[cqprime].get_symm().getirrep(), 
			       a.get_symm().getirrep(), b.get_symm().getirrep(), c.get_symm().getirrep(),
			       lbraS->quanta[aq].get_symm().getirrep() , rbraS->quanta[bq].get_symm().getirrep(), brastateinfo->quanta[cq].get_symm().getirrep());
	  scaleB *= b.get_scaling(rbraS->quanta[bq], rketS->quanta[bqprime]);
	  scaleA *= a.get_scaling(lbraS->quanta[aq], lketS->quanta[aqprime]);
	  if (b.get_fermion() && IsFermion (ketstateinfo->leftStateInfo->quanta [aqprime])) scaleB *= -1;
	  MatrixTensorProduct (a.operator_element(aq, aqprime), a.conjugacy(), scaleA, 
			       b.operator_element(bq, bqprime), b.conjugacy(), scaleB, cel,rowstride, colstride);
	}
	else
	{
	  scaleB = dmrginp.get_ninej()(lketS->quanta[bqprime].get_s().getirrep(), rketS->quanta[aqprime].get_s().getirrep() , ketstateinfo->quanta[cqprime].get_s().getirrep(), 
			 b.get_spin().getirrep(), a.get_spin().getirrep(), c.get_spin().getirrep(),
			 lbraS->quanta[bq].get_s().getirrep(), rbraS->quanta[aq].get_s().getirrep() , brastateinfo->quanta[cq].get_s().getirrep());
	  scaleB *= Symmetry::spatial_ninej(lketS->quanta[bqprime].get_symm().getirrep() , rketS->quanta[aqprime].get_symm().getirrep(), ketstateinfo->quanta[cqprime].get_symm().getirrep(), 
			       b.get_symm().getirrep(), a.get_symm().getirrep(), c.get_symm().getirrep(),
			       lbraS->quanta[bq].get_symm().getirrep() , rbraS->quanta[aq].get_symm().getirrep(), brastateinfo->quanta[cq].get_symm().getirrep());
	  scaleB *= b.get_scaling(lbraS->quanta[bq], lketS->quanta[bqprime]);
	  scaleA *= a.get_scaling(rbraS->quanta[aq], rketS->quanta[aqprime]);
	  if (a.get_fermion() && IsFermion (ketstateinfo->leftStateInfo->quanta[bqprime]) ) scaleB *= -1.;
	  
	  MatrixTensorProduct (b.operator_element(bq, bqprime), b.conjugacy(), scaleB, 
			       a.operator_element(aq, aqprime), a.conjugacy(), scaleA, cel, rowstride, colstride);
	}
      }
      colstride += ketstateinfo->unCollectedStateInfo->quantaStates[ oldToNewJ[oldj] ];

    }
    rowstride += brastateinfo->unCollectedStateInfo->quantaStates[ oldToNewI[oldi] ];
    
  }

  
}



//TENSOR PRODUCT A x B -> C
template<class T1, class T2, class T3> void TensorProduct (const T1& a, const T2& b, const StateInfo *brastateinfo, const StateInfo *ketstateinfo, T3& c, double scale, bool aIsLeftOp, int num_thrds=1)
{
  if (fabs(scale) < TINY) return;

  std::vector< std::pair<std::pair<int, int>, StackMatrix> >& nonZeroBlocks = c.get_nonZeroBlocks();

  //#pragma omp parallel for schedule(dynamic)
  for (int index = 0; index<nonZeroBlocks.size(); index++) {
    int cq = nonZeroBlocks[index].first.first, cqprime = nonZeroBlocks[index].first.second;
    TensorTraceElement(a, b, brastateinfo, ketstateinfo, c, c.operator_element(cq, cqprime), cq, cqprime, aIsLeftOp, scale);
  }

}

//TENSOR PRODUCT A x B -> C
 template<class B, class T1, class T2, class T3> void TensorProduct (const B *ablock, const T1& a, const T2& b, const B *cblock, const StateInfo *cstateinfo, T3& c, double scale, int num_thrds=1)
{
  if (fabs(scale) < TINY) return;
  int rows = c.nrows();
  int cols = c.ncols();

  //FIX THIS
  //#pragma omp parallel for schedule(dynamic)
  for (int cq = 0; cq < rows; ++cq)
  for (int cqprime = 0; cqprime < cols; ++cqprime)
  if (c.allowed(cq, cqprime)) {
      TensorProductElement(ablock, a, b, cblock, cstateinfo, c, c.operator_element(cq, cqprime), cq, cqprime, scale);
  }


  /*
  std::vector< std::pair<std::pair<int, int>, StackMatrix> >& nonZeroBlocks = c.get_nonZeroBlocks();
#pragma omp parallel for schedule(dynamic)
  for (int index = 0; index<nonZeroBlocks.size(); index++) {
    int cq = nonZeroBlocks[index].first.first, cqprime = nonZeroBlocks[index].first.second;
    if (c.conjugacy() == 'n')
      TensorProductElement(ablock, a, b, cblock, cstateinfo, c, c.operator_element(cq, cqprime), cq, cqprime, scale);
    else
      TensorProductElement(ablock, a, b, cblock, cstateinfo, c, c.operator_element(cq, cqprime), cqprime, cq, scale);
  }
  */
}

//************************************************


 template<class B, class T1, class T2> void TensorProduct (const B *ablock, const T1& a, const T2& b, const B* cblock, const StateInfo* cstateinfo, DiagonalMatrix& cDiagonal, double scale)
{
  if (fabs(scale) < TINY) return;
  const int aSz = a.nrows();
  const int bSz = b.nrows();
  const char conjC = (cblock->get_leftBlock() == ablock) ? 'n' : 't';
  const B* bblock = (cblock->get_leftBlock() == ablock) ? cblock->get_rightBlock() : cblock->get_leftBlock();
  const StateInfo& s = cblock->get_stateInfo();
  const StateInfo* lS = s.leftStateInfo, *rS = s.rightStateInfo;

  for (int aQ = 0; aQ < aSz; ++aQ)
    if (a.allowed(aQ, aQ))
      for (int bQ = 0; bQ < bSz; ++bQ)
	if (b.allowed(bQ, bQ))
	  if (s.allowedQuanta (aQ, bQ, conjC))
	  {
	    int cQ = s.quantaMap (aQ, bQ, conjC)[0];
	    Real scaleA = scale;
	    Real scaleB = 1;
	    if (conjC == 'n')
	      {
		scaleB *= dmrginp.get_ninej()(lS->quanta[aQ].get_s().getirrep() , rS->quanta[bQ].get_s().getirrep(), cstateinfo->quanta[cQ].get_s().getirrep(), 
					      a.get_spin().getirrep(), b.get_spin().getirrep(), 0,
					      lS->quanta[aQ].get_s().getirrep() , rS->quanta[bQ].get_s().getirrep(), cstateinfo->quanta[cQ].get_s().getirrep());
		scaleB *= Symmetry::spatial_ninej(lS->quanta[aQ].get_symm().getirrep() , rS->quanta[bQ].get_symm().getirrep(), cstateinfo->quanta[cQ].get_symm().getirrep(), 
				     a.get_symm().getirrep(), b.get_symm().getirrep(), 0,
				     lS->quanta[aQ].get_symm().getirrep() , rS->quanta[bQ].get_symm().getirrep(), cstateinfo->quanta[cQ].get_symm().getirrep());
		
		if (b.get_fermion() && IsFermion (lS->quanta [aQ])) scaleB *= -1.0;
		for (int aQState = 0; aQState < lS->quantaStates[aQ] ; aQState++)
		  MatrixDiagonalScale(a.operator_element(aQ, aQ)(aQState+1, aQState+1)*scaleA*scaleB, b.operator_element(bQ, bQ), 
				      cDiagonal.Store()+s.unBlockedIndex[cQ]+aQState*rS->quantaStates[bQ]);

	      }
	    else
	      {
		scaleB *= dmrginp.get_ninej()(lS->quanta[bQ].get_s().getirrep() , rS->quanta[aQ].get_s().getirrep(), cstateinfo->quanta[cQ].get_s().getirrep(), 
					      b.get_spin().getirrep(), a.get_spin().getirrep(), 0,
					      lS->quanta[bQ].get_s().getirrep() , rS->quanta[aQ].get_s().getirrep(), cstateinfo->quanta[cQ].get_s().getirrep());
		scaleB *= Symmetry::spatial_ninej(lS->quanta[bQ].get_symm().getirrep() , rS->quanta[aQ].get_symm().getirrep(), cstateinfo->quanta[cQ].get_symm().getirrep(), 
				     b.get_symm().getirrep(), a.get_symm().getirrep(), 0,
				     lS->quanta[bQ].get_symm().getirrep() , rS->quanta[aQ].get_symm().getirrep(), cstateinfo->quanta[cQ].get_symm().getirrep());
		
		if (a.get_fermion()&& IsFermion(lS->quanta[bQ])) scaleB *= -1.0;
		for (int bQState = 0; bQState < lS->quantaStates[bQ] ; bQState++)
		  MatrixDiagonalScale(b.operator_element(bQ, bQ)(bQState+1, bQState+1)*scaleA*scaleB, a.operator_element(aQ, aQ), 
				      cDiagonal.Store()+s.unBlockedIndex[cQ]+bQState*rS->quantaStates[aQ]);
	      }
	  }
}


template<class B, class T1, class T2> void TensorMultiply(const B *ablock, const T1& a, const B *cblock, T2& c, T2& v, const SpinQuantum dQ, double scale, int num_thrds=1)
{
  // cannot be used for situation with different bra and ket
  const int leftBraOpSz = cblock->get_leftBlock()->get_braStateInfo().quanta.size ();
  const int leftKetOpSz = cblock->get_leftBlock()->get_ketStateInfo().quanta.size ();
  const int rightBraOpSz = cblock->get_rightBlock()->get_braStateInfo().quanta.size ();
  const int rightKetOpSz = cblock->get_rightBlock()->get_ketStateInfo().quanta.size ();

  const StateInfo* lbraS = cblock->get_braStateInfo().leftStateInfo, *lketS = cblock->get_ketStateInfo().leftStateInfo;
  const StateInfo* rbraS = cblock->get_braStateInfo().rightStateInfo, *rketS = cblock->get_ketStateInfo().rightStateInfo;


  assert (cblock->get_leftBlock() == ablock || cblock->get_rightBlock() == ablock);
  if (cblock->get_leftBlock() == ablock)
  {
    const std::vector< std::pair<std::pair<int, int>, StackMatrix> >& nonZeroBlocks = a.get_nonZeroBlocks();
    //#pragma omp parallel for schedule(dynamic)
    for (int index = 0; index<nonZeroBlocks.size(); index++) {
      int lQ = nonZeroBlocks[index].first.first, lQPrime = nonZeroBlocks[index].first.second;
      for (int rQ = 0; rQ < rightKetOpSz; ++rQ) 
	if (c.allowed(lQPrime, rQ) && v.allowed(lQ, rQ))
	{
	  double fac=scale;
	  fac *= dmrginp.get_ninej()(lketS->quanta[lQPrime].get_s().getirrep(), rketS->quanta[rQ].get_s().getirrep() , c.get_deltaQuantum(0).get_s().getirrep(), 
				     a.get_spin().getirrep(), 0, a.get_spin().getirrep(),
				     lbraS->quanta[lQ].get_s().getirrep(), rketS->quanta[rQ].get_s().getirrep() , v.get_deltaQuantum(0).get_s().getirrep());
	  fac *= Symmetry::spatial_ninej(lketS->quanta[lQPrime].get_symm().getirrep() , rketS->quanta[rQ].get_symm().getirrep(), c.get_symm().getirrep(), 
					 a.get_symm().getirrep(), 0, a.get_symm().getirrep(),
					 lbraS->quanta[lQ].get_symm().getirrep() , rketS->quanta[rQ].get_symm().getirrep(), v.get_symm().getirrep());
	  fac *= a.get_scaling(lbraS->quanta[lQ], lketS->quanta[lQPrime]);
	  MatrixMultiply (a.operator_element(lQ, lQPrime), a.conjugacy(), c.operator_element(lQPrime, rQ), c.conjugacy(),
			  v.operator_element(lQ, rQ), fac);
	}
      
    }
  }
  else
  {
    const std::vector< std::pair<std::pair<int, int>, StackMatrix> >& nonZeroBlocks = a.get_nonZeroBlocks();
    //#pragma omp parallel for schedule(dynamic)
    for (int index = 0; index<nonZeroBlocks.size(); index++) {
      int rQ = nonZeroBlocks[index].first.first, rQPrime = nonZeroBlocks[index].first.second;
      for (int lQPrime = 0; lQPrime < leftKetOpSz; ++lQPrime) 
	if (v.allowed(lQPrime, rQ) && c.allowed(lQPrime, rQPrime)) {
	  double fac = scale;
	  fac *= dmrginp.get_ninej()(lketS->quanta[lQPrime].get_s().getirrep(), rketS->quanta[rQPrime].get_s().getirrep() , c.get_deltaQuantum(0).get_s().getirrep(), 
				     0, a.get_spin().getirrep(), a.get_spin().getirrep(),
				     lketS->quanta[lQPrime].get_s().getirrep(), rbraS->quanta[rQ].get_s().getirrep() , v.get_deltaQuantum(0).get_s().getirrep());
	  fac *= Symmetry::spatial_ninej(lketS->quanta[lQPrime].get_symm().getirrep() , rketS->quanta[rQPrime].get_symm().getirrep(), c.get_symm().getirrep(), 
					 0, a.get_symm().getirrep(), a.get_symm().getirrep(),
					 lketS->quanta[lQPrime].get_symm().getirrep() , rbraS->quanta[rQ].get_symm().getirrep(), v.get_symm().getirrep());
	  fac *= a.get_scaling(rbraS->quanta[rQ], rketS->quanta[rQPrime]);
	  double parity = a.get_fermion() && IsFermion(lketS->quanta[lQPrime]) ? -1 : 1;
	  
	  MatrixMultiply (c.operator_element(lQPrime, rQPrime), c.conjugacy(),
			  a.operator_element(rQ, rQPrime), TransposeOf(a.conjugacy()), v.operator_element(lQPrime, rQ), fac*parity);
	}
      
    }
  }
}

void TensorMultiply(const StackSpinBlock *ablock, const StackSparseMatrix& a, const StackSparseMatrix& b, const StackSpinBlock *cblock, StackWavefunction& c, StackWavefunction* v, const SpinQuantum opQ, double scale);
    
    

  void Product (const StackSpinBlock *ablock, const Baseoperator<Matrix>& a, const Baseoperator<Matrix>& b, Baseoperator<Matrix>& c, double scale);



  void OperatorScaleAdd(double scaleV, const StackSpinBlock& b, const Baseoperator<Matrix>& op1, Baseoperator<Matrix>& op2);
  template<class T1, class T2, class T3>void MultiplyProduct(const T1& a, const T2& b, T3& c, Real scale)
  {
    if (fabs(scale) < TINY) return;
    const int aSz = a.nrows();
    const int aSzPrime = a.ncols();
    const int bSzPrime = b.ncols();
    
    assert (a.ncols() == b.nrows() && c.nrows() == a.nrows() &&
	    c.ncols() == b.ncols());
    
    for (int aQ = 0; aQ < aSz; ++aQ)
      for (int aQPrime = 0; aQPrime < aSzPrime; ++aQPrime)
	for (int bQPrime = 0; bQPrime < bSzPrime; ++bQPrime)
	  {
	    if (a.allowed(aQ, aQPrime) && b.allowed(aQPrime, bQPrime) && c.allowed(aQ, bQPrime) ) {
	      
	      MatrixMultiply (a.operator_element(aQ, aQPrime), a.conjugacy(), b.operator_element(aQPrime, bQPrime), b.conjugacy(),
			      c.operator_element(aQ, bQPrime), scale);
	    }
	  }
  }



void MultiplyWithOwnTranspose(const StackSparseMatrix& a, StackSparseMatrix& c, Real scale);

}
}
#endif
