/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI QC Object Module
 * *-
 * @file QCObjectManager.h
 * @author CHC
 * @date 2025-09-28
 * @brief management all QC Objects
 * 
**************************************************************************/
#ifndef QCOBJECTMANAGER_H
#define QCOBJECTMANAGER_H

#include <QMap>
#include <QSharedPointer>
#include "QCObject.h"

class QCObjectManager
{
public:
    static QCObjectManager *getInstance();
    QSharedPointer<QCObject> addQCObject();
    QSharedPointer<QCObject> getQCObjectByID(int id)const;
    QVector<QSharedPointer<QCObject> > getAllQCObjects()const;
    QVector<int> getAllQCObjectIds()const;
    void deleteQCObject(int id);
private:
    int generateQCObjectID();
private:
    QCObjectManager();
    QMap<int, QSharedPointer<QCObject>> m_objects;
};

#endif // QCOBJECTMANAGER_H
