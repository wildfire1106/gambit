//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute Nash equilibria via heuristic search on game supports
// (Porter, Nudelman & Shoham, 2004)
// Implemented by Litao Wei
//
// This file is part of Gambit
// Copyright (c) 2006, Litao Wei and The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include "nfgcpoly.h"

//-------------------------------------------------------------------------
//              HeuristicPolEnumModule: Member functions
//-------------------------------------------------------------------------

HeuristicPolEnumModule::HeuristicPolEnumModule(const StrategySupport &S, int p_stopAfter)
  : m_stopAfter(p_stopAfter), NF(S.GetGame()), support(S),
    Space(support.ProfileLength()-NF->NumPlayers()), 
    Lex(&Space, lex), num_vars(support.ProfileLength()-NF->NumPlayers()), 
    count(0), nevals(0), is_singular(false)
{ 
//  gEpsilon(eps,12);
}


int HeuristicPolEnumModule::PolEnum(void)
{
  gPolyList<gDouble> equations = NashOnSupportEquationsAndInequalities();

  /*
  // equations for equality of strat j to strat j+1
  for( i=1;i<=NF.NumPlayers();i++) 
    for(j=1;j<support.NumStrats(i);j++) 
      equations+=IndifferenceEquation(i,j,j+1);

  for( i=1;i<=NF.NumPlayers();i++)
    if(support.NumStrats(i)>2) 
      equations+=Prob(i,support.NumStrats(i));
  */

  // set up the rectangle of search
  Vector<gDouble> bottoms(num_vars), tops(num_vars);
  bottoms = (gDouble)0;
  tops = (gDouble)1;
 
  gRectangle<gDouble> Cube(bottoms, tops); 

  // start QuikSolv
  Gambit::List<Vector<gDouble> > solutionlist = NashOnSupportSolnVectors(equations,
								   Cube);

  int index = SaveSolutions(solutionlist);
  return index;	 
}

int HeuristicPolEnumModule::SaveSolutions(const Gambit::List<Vector<gDouble> > &list)
{
  MixedStrategyProfile<double> profile(support);
  int i,j,k,kk,index=0;
  double sum;

  for(k=1;k<=list.Length();k++) {
    kk=0;
    for(i=1;i<=NF->NumPlayers();i++) {
      sum=0;
      for(j=1;j<support.NumStrats(i);j++) {
	profile(i,j) = (list[k][j+kk]).ToDouble();
	sum+=profile(i,j);
      }
      profile(i,j) = (double)1.0 - sum;
      kk+=(support.NumStrats(i)-1);
    }
    index = solutions.Append(profile);
  }
  return index;
}

bool HeuristicPolEnumModule::EqZero(gDouble x) const
{
  if(x <= eps && x >= -eps) return 1;
  return 0;
}     

long HeuristicPolEnumModule::NumEvals(void) const
{
  return nevals;
}

double HeuristicPolEnumModule::Time(void) const
{
  return time;
}

const Gambit::List<MixedStrategyProfile<double> > &HeuristicPolEnumModule::GetSolutions(void) const
{
  return solutions;
}

gPoly<gDouble> HeuristicPolEnumModule::Prob(int p, int strat) const
{
  gPoly<gDouble> equation(&Space,&Lex);
  Vector<int> exps(num_vars);
  int i,j,kk = 0;
  
  for(i=1;i<p;i++) 
    kk+=(support.NumStrats(i)-1);

  if(strat<support.NumStrats(p)) {
    exps=0;
    exps[strat+kk]=1;
    exp_vect const_exp(&Space,exps);
    gMono<gDouble> const_term((gDouble)1,const_exp);
    gPoly<gDouble> new_term(&Space,const_term,&Lex);
    equation+=new_term;
  }
  else {
    for(j=1;j<support.NumStrats(p);j++) {
      exps=0;
      exps[j+kk]=1;
      exp_vect exponent(&Space,exps);
      gMono<gDouble> term((gDouble)(-1),exponent);
      gPoly<gDouble> new_term(&Space,term,&Lex);
      equation+=new_term;
    }
    exps=0;
    exp_vect const_exp(&Space,exps);
    gMono<gDouble> const_term((gDouble)1,const_exp);
    gPoly<gDouble> new_term(&Space,const_term,&Lex);
    equation+=new_term;
  }
  return equation;
}

gPoly<gDouble> 
HeuristicPolEnumModule::IndifferenceEquation(int i, int strat1, int strat2) const
{
  PureStrategyProfile profile(NF);

  StrategyIterator A(support, i, strat1);
  StrategyIterator B(support, i, strat2);

  gPoly<gDouble> equation(&Space,&Lex);
  do {
    gPoly<gDouble> term(&Space,(gDouble)1,&Lex);
    profile = A.GetProfile();
    int k;
    for(k=1;k<=NF->NumPlayers();k++) 
      if(i!=k) 
	term*=Prob(k,support.GetIndex(profile.GetStrategy(k)));
    gDouble coeff,ap,bp;
    ap = (double) A.GetPayoff(i);
    bp = (double) B.GetPayoff(i);
    coeff = ap - bp;
    term*=coeff;
    equation+=term;
    A.NextContingency();
  } while (B.NextContingency());
  return equation;
}


gPolyList<gDouble>   HeuristicPolEnumModule::IndifferenceEquations()  const
{
  gPolyList<gDouble> equations(&Space,&Lex);

  for(int pl=1;pl<=NF->NumPlayers();pl++) 
    for(int j=1;j<support.NumStrats(pl);j++) 
      equations+=IndifferenceEquation(pl,j,j+1);

  return equations;
}
 
gPolyList<gDouble> HeuristicPolEnumModule::LastActionProbPositiveInequalities() const
{
  gPolyList<gDouble> equations(&Space,&Lex);

  for(int pl=1;pl<=NF->NumPlayers();pl++)
    if(support.NumStrats(pl)>2) 
      equations+=Prob(pl,support.NumStrats(pl));

  return equations;
}

gPolyList<gDouble> HeuristicPolEnumModule::NashOnSupportEquationsAndInequalities() const
{
  gPolyList<gDouble> equations(&Space,&Lex);
  
  equations += IndifferenceEquations();
  equations += LastActionProbPositiveInequalities();

  return equations;
}


Gambit::List<Vector<gDouble> > 
HeuristicPolEnumModule::NashOnSupportSolnVectors(const gPolyList<gDouble> &equations,
					const gRectangle<gDouble> &Cube)
{  
  QuikSolv<gDouble> quickie(equations);
  //  p_status.SetProgress(0);

  try {
    quickie.FindCertainNumberOfRoots(Cube,2147483647, m_stopAfter);
  }
  catch (SingularMatrixException) {
    is_singular = true;
  }

  return quickie.RootList();
}

bool HeuristicPolEnumModule::IsSingular() const
{
  return is_singular;
}


//---------------------------------------------------------------------------
//                        PolEnumParams: member functions
//---------------------------------------------------------------------------

int PolEnum(const StrategySupport &support, int p_stopAfter,
	    Gambit::List<MixedStrategyProfile<double> > &solutions, 
	    long &nevals, double &time, bool &is_singular)
{
  HeuristicPolEnumModule module(support, p_stopAfter);
  module.PolEnum();
  nevals = module.NumEvals();
  time = module.Time();
  solutions = module.GetSolutions();
  if (module.IsSingular()) 
    is_singular = true;
  else 
    is_singular = false;
  return 1;
}


//---------------------------------------------------------------------------
//                        Polish Equilibrum for Nfg
//---------------------------------------------------------------------------

static
MixedStrategyProfile<double> PolishEquilibrium(const StrategySupport &support, 
				       const MixedStrategyProfile<double> &sol, 
				       bool &is_singular)
{
  HeuristicPolEnumModule module(support, 0);
  Vector<gDouble> vec = module.SolVarsFromMixedStrategyProfile(sol);

  /* //DEBUG
  gbtPVector<double> xx = module.SeqFormProbsFromSolVars(vec);
  MixedStrategyProfile<gbtNumber> newsol = module.SequenceForm().ToMixed(xx);

  gout << "sol.Profile = " << *(sol.Profile()) << "\n";
  gout << "vec  = " << vec << "\n";
  gout << "xx   = " << xx << "\n";
  gout << "newsol   = " << newsol << "\n";

    exit(0);
  if ( newsol != *(sol.Profile()) ) {
    gout << "Failure of reversibility in PolishEquilibrium.\n";
    exit(0);
  }
  */

  //DEBUG
  //  gout << "Prior to Polishing vec is " << vec << ".\n";

  module.PolishKnownRoot(vec);

  //DEBUG
  //  gout << "After Polishing vec is " << vec << ".\n";

  return module.ReturnPolishedSolution(vec);
}


Vector<gDouble> 
HeuristicPolEnumModule::SolVarsFromMixedStrategyProfile(const MixedStrategyProfile<double> &sol) const
{
  int numvars(0);

  for (int pl = 1; pl <= NF->NumPlayers(); pl++) 
    numvars += support.NumStrats(pl) - 1;

  Vector<gDouble> answer(numvars);
  int count(0);

  for (int pl = 1; pl <= NF->NumPlayers(); pl++) 
    for (int j = 1; j < support.NumStrats(pl); j++) {
      count ++;
      answer[count] = (gDouble)sol(pl,j);
    }

  return answer;
}

const int HeuristicPolEnumModule::PolishKnownRoot(Vector<gDouble> &point) const
{
  //DEBUG
  //  gout << "Prior to Polishing point is " << point << ".\n";

  if (point.Length() > 0) {
    // equations for equality of strat j to strat j+1
    gPolyList<gDouble> equations(&Space,&Lex);
    equations += IndifferenceEquations();

    //DEBUG
    //    gout << "We are about to construct quickie with Dmnsn() = "
    //  << Space->Dmnsn() << " and equations = \n"
    //	 << equations << "\n";
    
    // start QuikSolv
    QuikSolv<gDouble> quickie(equations);
    
    //DEBUG
    //    gout << "We constructed quickie.\n";
    
    try { 
      point = quickie.NewtonPolishedRoot(point);
    }
    catch (SingularMatrixException &) {
      return 0;
    }

    //DEBUG
    //    gout << "After Polishing point = " << point << ".\n";

  }

  return 1;	 
}

MixedStrategyProfile<double>
HeuristicPolEnumModule::ReturnPolishedSolution(const Vector<gDouble> &root) const
{
  MixedStrategyProfile<double> profile(support);

  int j;
  int kk=0;
  for(int pl=1;pl<=NF->NumPlayers();pl++) {
    double sum=0;
    for(j=1;j<support.NumStrats(pl);j++) {
      profile(pl,j) = (root[j+kk]).ToDouble();
      sum+=profile(pl,j);
    }
    profile(pl,j) = (double)1.0 - sum;
    kk+=(support.NumStrats(pl)-1);
  }
       
  return profile;
}

