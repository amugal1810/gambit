//
// FILE: eliap.cc -- Extensive Form Liapunov module
//
// $Id$
//

#include "eliap.h"

#include "gfunc.h"
#include "math/gmatrix.h"

EFLiapParams::EFLiapParams(void)
  : nTries(10)
{ }

class EFLiapFunc : public gFunction<double>  {
  private:
    long _nevals;
    const Efg::Game &_efg;
    BehavProfile<double> _p;

    double Value(const gVector<double> &x);

  public:
    EFLiapFunc(const Efg::Game &, const BehavProfile<double> &);
    virtual ~EFLiapFunc();
    
    long NumEvals(void) const  { return _nevals; }
};


EFLiapFunc::EFLiapFunc(const Efg::Game &E,
		       const BehavProfile<double> &start)
  : _nevals(0L), _efg(E), _p(start)
{ }

EFLiapFunc::~EFLiapFunc()
{ }


double EFLiapFunc::Value(const gVector<double> &v)
{
  _nevals++;
  ((gVector<double> &) _p).operator=(v);
    //_p = v;
  return _p.LiapValue();
}

static void PickRandomProfile(BehavProfile<double> &p)
{
  double sum, tmp;

  for (int pl = 1; pl <= p.GetGame().NumPlayers(); pl++)  {
    for (int iset = 1; iset <= p.GetGame().Players()[pl]->NumInfosets();
	 iset++)  {
      sum = 0.0;
      int act;
    
      for (act = 1; act < p.Support().NumActions(pl, iset); act++)  {
	do
	  tmp = Uniform();
	while (tmp + sum > 1.0);
	p(pl, iset, act) = tmp;
	sum += tmp;
      }
  
// with truncation, this is unnecessary
      p(pl, iset, act) = 1.0 - sum;
    }
  }
}


static void AddSolution(gList<BehavSolution> &solutions,
			const BehavProfile<double> &profile,
		        double /*value*/, double epsilon)
{
  int index = solutions.Append(BehavSolution(profile, algorithmEfg_LIAP_EFG));
  solutions[index].SetEpsilon(epsilon);
}

extern void Project(gVector<double> &, const gArray<int> &);

static void InitMatrix(gMatrix<double> &xi, const gArray<int> &dim)
{
  xi.MakeIdent();

  gVector<double> foo(xi.NumColumns());
  for (int i = 1; i <= xi.NumRows(); i++)   {
    xi.GetRow(i, foo);
    Project(foo, dim);
    xi.SetRow(i, foo);
  }
}

extern bool Powell(gPVector<double> &p,
		   gMatrix<double> &xi,
		   gFunction<double> &func,
		   double &fret, int &iter,
		   int maxits1, double tol1, int maxitsN, double tolN,
		   gOutput &tracefile, int tracelevel, bool interior,
		   gStatus &status);


bool Liap(const Efg::Game &E, EFLiapParams &params,
	  const BehavProfile<double> &start,
	  gList<BehavSolution> &solutions, gStatus &p_status,
	  long &nevals, long &niters)
{
  static const double ALPHA = .00000001;

  EFLiapFunc F(E, start);

  BehavProfile<double> p(start);

  // if starting vector not interior, perturb it towards centroid
  int kk;
  for(kk=1;kk <= p.Length() && p[kk]>ALPHA;kk++);
  if(kk<=p.Length()) {
    BehavProfile<double> c(start.Support());
    for(int k=1;k<=p.Length();k++)
      p[k] = c[k]*ALPHA + p[k]*(1.0-ALPHA);
  }

  gMatrix<double> xi(p.Length(), p.Length());

  double value;
  int iter;

  for (int i = 1; (params.nTries == 0 || i <= params.nTries) &&
       (params.stopAfter==0 || solutions.Length() < params.stopAfter); 
       i++)   {
    p_status.Get();
    if (i > 1)  PickRandomProfile(p);

    InitMatrix(xi, p.Lengths());
    
    if(params.trace>0)
      *params.tracefile << "\nTry #: " << i << " p: ";
    
    if (Powell(p, xi, F, value, iter,
		       params.maxits1, params.tol1, params.maxitsN, 
		       params.tolN,*params.tracefile, params.trace-1, true, 
		       p_status)) {
      
      bool add = true;
      int ii=1;
      while(ii<=solutions.Length() && add == true) {
	if(solutions[ii].Equals(p)) 
	  add = false;
	ii++;
      }

      if (add)  {
	if(params.trace>0)
	  *params.tracefile << p;
	
	AddSolution(solutions, p, value, pow(params.tolN,.5));
      }
    }
  }

  nevals = F.NumEvals();
  niters = 0L;

  return (solutions.Length() > 0);
}



//------------------------------------------
// Interfacing to solve-by-subgame code
//------------------------------------------

void efgLiapSolve::SolveSubgame(const FullEfg &E, const EFSupport &sup,
				gList<BehavSolution> &solns,
				gStatus &p_status)
{
  BehavProfile<double> bp(sup);
  
  subgame_number++;

  gArray<int> infosets(infoset_subgames.Lengths());

  for (int pl = 1; pl <= E.NumPlayers(); pl++)  {
    int niset = 1;
    for (int iset = 1; iset <= infosets[pl]; iset++)  {
      if (infoset_subgames(pl, iset) == subgame_number)  {
	for (int act = 1; act <= bp.Support().NumActions(pl, niset); act++)
	  bp(pl, niset, act) = start(pl, iset, act);
	niset++;
      }
    }
  }

  long this_nevals, this_niters;

  Liap(E, params, bp, solns, p_status, this_nevals, this_niters);

  nevals += this_nevals;
}

extern void MarkedSubgameRoots(const Efg::Game &, gList<Node *> &);

efgLiapSolve::efgLiapSolve(const Efg::Game &E, const EFLiapParams &p,
			   const BehavProfile<gNumber> &s, int max)
  : SubgameSolver(max), nevals(0), subgame_number(0),
    infoset_subgames(E.NumInfosets()), params(p), start(s)
{
  gList<Node *> subroots;
  MarkedSubgameRoots(E, subroots);

  for (int pl = 1; pl <= E.NumPlayers(); pl++)   {
    EFPlayer *player = E.Players()[pl];
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      int index;

      Infoset *infoset = player->Infosets()[iset];
      Node *member = infoset->Members()[1];

      for (index = 1; index <= subroots.Length() &&
	   member->GetSubgameRoot() != subroots[index]; index++);

      infoset_subgames(pl, iset) = index;
    }
  }   

}

efgLiapSolve::~efgLiapSolve()   { }


