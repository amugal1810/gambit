//#
//# FILE: player.h -- Declaration of Player data type
//#
//# $Id$
//#

#ifndef PLAYER_H
#define PLAYER_H

class gRational;

class Player   {
  friend class BaseExtForm;
  friend class ExtForm<double>;
  friend class ExtForm<gRational>;
  private:
    int number;
    gString name;
    
    gBlock<Infoset *> infosets;

    Player(int n) : number(n)  { }
    ~Player();

  public:
    const gString &GetName(void) const   { return name; }
    void SetName(const gString &s)       { name = s; }

    bool IsChance(void) const      { return (number == 0); }

    int NumInfosets(void) const    { return infosets.Length(); }

    Infoset *GetInfoset(const gString &name) const;
    const gArray<Infoset *> &InfosetList(void) const  { return infosets; }
};


#endif    //# PLAYER_H


