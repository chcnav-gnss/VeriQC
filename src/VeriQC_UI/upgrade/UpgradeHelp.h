/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI upgrade Module
 * *-
 * @file UpgradeHelp.h
 * @author CHC
 * @date 2025-09-28
 * @brief Helper for check new version and upgrade
 * 
**************************************************************************/
#ifndef UPGRADEHELP_H_
#define UPGRADEHELP_H_ 
#include <qstring.h>
#include<QObject>
#include "nlohmann/json.hpp" 
#include "UpgradeDTO.h"
class UpgradeHelp : public QObject
{
	Q_OBJECT
public:
    UpgradeHelp(QWidget *parentWidget,bool isNeedUpdate=false);
	~UpgradeHelp();
    void upgradeSoftWare();
    bool isNeedUpdate()const;
private:
    static void queryNewVersion(bool isNeedUpdate, QWidget *parentWidget);
    static void checkSuccessAction(const QString& response, bool isNeedUpdate, QWidget *parentWidget);
	static void killSoftWare(QStringList args);
    static void showIsNeweastVersion(QWidget *parentWidget);
    static CheckDTO getCheckRespDto(const nlohmann::json& jsonObj);
    static LoginRespDTO getLoginRespDto(const nlohmann::json& jsonObj);
private:
    QWidget *m_parentWidget;
    bool m_isNeedUpdate;
};
#endif //UPGRADEHELP_H_
