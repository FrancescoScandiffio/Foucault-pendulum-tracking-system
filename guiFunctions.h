#ifndef FOUCAULT_GUIFUNCTIONS_H
#define FOUCAULT_GUIFUNCTIONS_H

#include <string>

void changePointFocus(int status, void *data);

void toggleDrag(int status, void *data);

void mouseCallBack(int event, int x, int y, int flags, void *userdata);

void savePoints(int status, void *data);

inline bool fileExist(const std::string &name);

int calibrateCamera();

void drawGraph();

void usage();



#endif //FOUCAULT_GUIFUNCTIONS_H
