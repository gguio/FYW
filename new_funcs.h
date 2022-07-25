
#ifndef NEW_FUNCS_H
#define NEW_FUNCS_H

int hist3D(std::string argv[]);
int **centralities(std::string argv[]);
void Free( int ** M);
void Free_rms( double ** rms);
int proj(std::string argv[], int ** M);
int hist3D_pp (std::string argv[]);
int rfactors(std::string argv[], int ** MAB, int ** Mpp);
int hist3D_channels (std::string argv[]);
int hist3D_pp_channels (std::string argv[]);
void drawing (std::string argv[]);
double **RMS (std::string argv[]);
int minus_background (std::string settings[]);


#endif
