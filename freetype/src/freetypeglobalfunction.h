#ifndef FREETYPEGLOBALFUNCTION_H
#define FREETYPEGLOBALFUNCTION_H

#include <QStringList>

QStringList familyNameFromPath(const QString &filePath);
int faceIndexFromName(const QString &filePath,const QString &name);

#endif // FREETYPEGLOBALFUNCTION_H