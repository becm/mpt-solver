# PDE solver module
Interfaces to the [BACOL](http://cs.smu.ca/~muir/BACOLI-3_Webpage.htm)
solver family.

|File|SHA256|
|-|-|
|bacol.f|290f72a8ed403b10963366b6895db5ec7d934dff147b1d08989d2177eea4ccc6|
|bacolrnoblas.f|dec78bafcbf9cf5646e3a1264478de0e06f17d8077b0cab1c0f2df1d1d5b146a|

## Local Changes
Modifications must be applied to BACOLR to allow linking with DASSL-based BACOL
implementation.

Steps used to generate a modified `bacolrr.f` based on the original `bacolrnoblas.f`:

- rename:
  - reinit -> reinitr
  - remesh -> remeshr (plus: fix nint > nintmx error, see updated BACOL-2 code)
  - caljac -> caljacr
  
- remove (shared with bacol.f):
  - bsplvd
  - bsplvn
    
  - colpnt
  - crdcmp
  - crslve
    
  - errest
  - errval
    
  - eval
  - gauleg
  - iniy
  - interv
  - meshsq
    
  - values
  - imtql1
  - imtql2
