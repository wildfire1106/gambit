//
// FILE: dlenumpure.h -- EnumPureSolve interface
//
// $Id$
//

#ifndef DLENUMPURE_H
#define DLENUMPURE_H

#include "algdlgs.h"

class dialogEnumPure : public dialogAlgorithm {
private:
  void AlgorithmFields(void);

  const char *HelpString(void) const { return "EnumPure"; }

public:
  dialogEnumPure(wxWindow *p_parent = 0, bool p_subgames = false,
		 bool p_vianfg = false);
  virtual ~dialogEnumPure();

  int StopAfter(void) const;
};

#endif  // DLENUMPURE_H

