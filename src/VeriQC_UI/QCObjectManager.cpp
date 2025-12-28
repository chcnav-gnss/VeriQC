#include "QCObjectManager.h"

QCObjectManager::QCObjectManager()
{

}

QCObjectManager *QCObjectManager::getInstance()
{
    static QCObjectManager s_manager;
    return &s_manager;
}

QSharedPointer<QCObject> QCObjectManager::addQCObject()
{
    QSharedPointer<QCObject> newObject = QSharedPointer<QCObject>(new QCObject(this->generateQCObjectID()));
    m_objects.insert(newObject->getID(),newObject);
    return newObject;
}

QSharedPointer<QCObject> QCObjectManager::getQCObjectByID(int id) const
{
    return m_objects.value(id,nullptr);
}

QVector<QSharedPointer<QCObject> > QCObjectManager::getAllQCObjects() const
{
    return m_objects.values().toVector();
}

QVector<int> QCObjectManager::getAllQCObjectIds() const
{
    return m_objects.keys().toVector();
}

void QCObjectManager::deleteQCObject(int id)
{
    m_objects.remove(id);
}

int QCObjectManager::generateQCObjectID()
{
    static int s_idCount = 0;
    return ++s_idCount;
}
