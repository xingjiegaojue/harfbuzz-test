#ifndef CODECONVERT_H
#define CODECONVERT_H

// return 0 if success
int utf16ToBig5(unsigned short utf16InputCode,unsigned short *pBig5OutputCode);

int unicodeToGB2312(unsigned short unicodeInputCode,unsigned short *pGB2312OutputCode);

#endif // CODECONVERT_H