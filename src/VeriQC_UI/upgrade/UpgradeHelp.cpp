#include"UpgradeHelp.h"
#include "nlohmann/json.hpp" 
#include "HttpClient.h"
#include <QtCore> 
#include "CNewVersionDescDialog.h"
#include "../Components/MyMessageBox.h"
#include "AppSettingsManagement.h"


QString host = "https://developer.huace.cn/cloud/iot/firmware/open-api/v1/check-firmwares";
#define UPGRADE_APP_KEY			"tthj7bdyx6kl"
#define UPGRADE_APP_SECRET		"8196c98185d54312b4d5068c30c5e422"

static QString calculateSHA1(const QString& data) {
	QCryptographicHash sha1(QCryptographicHash::Sha1);
	sha1.addData(data.toLocal8Bit());
	return sha1.result().toHex();
}

UpgradeHelp::UpgradeHelp(QWidget *parentWidget, bool isNeedUpdate)
    :m_parentWidget(parentWidget),m_isNeedUpdate(isNeedUpdate)
{
	QString downLoadPath = QCoreApplication::applicationDirPath() +
		QDir::separator() + "tmp.zip";

	if (QFile::exists(downLoadPath))
	{
		QFile file(downLoadPath);
		file.remove();
	}
}
UpgradeHelp::~UpgradeHelp()
{

}
void UpgradeHelp::upgradeSoftWare()
{
    queryNewVersion(m_isNeedUpdate,m_parentWidget);
}

bool UpgradeHelp::isNeedUpdate() const
{
    return m_isNeedUpdate;
}
void UpgradeHelp::queryNewVersion(bool isNeedUpdate, QWidget *parentWidget)
{
#ifdef _WIN32
    auto j2 = R"({"pn": "VeriQC_UI_WIN64","version": "0.0.1"})"_json;
#elif defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    auto j2 = R"({"pn": "VeriQC_Linux","version": "0.0.1"})"_json;
#endif

	QString paramter = QString::fromStdString(j2.dump());
	QString curTime = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000);
	QString nonce = QString::number(QRandomGenerator::global()->bounded(1000));
	QString checksum = calculateSHA1(UPGRADE_APP_SECRET + nonce + curTime);
	QString sysName = "POSVANT";
	QString password = "posvantqaz";
	HttpClient(host).
        success([isNeedUpdate,parentWidget](const QString& response)
			{
				qInfo("Response: %s", response.toLocal8Bit().data());
                UpgradeHelp::checkSuccessAction(response, isNeedUpdate,parentWidget);
            }).fail([isNeedUpdate,parentWidget](const QString& response, int code)
				{
                    Q_UNUSED(response)
					if (isNeedUpdate == true)
					{
						QString content = tr("Query upgrade service failed,response code:%1").arg(code);
                        MyMessageBox::critical(parentWidget, tr("Upgrade"), content);
					}
				}).header("CurTime", curTime)
					.header("Nonce", nonce)
					.header("AppKey", UPGRADE_APP_KEY)
					.header("CheckSum", checksum)
					.header("sysName", sysName)
					.header("password", password)
					.json(paramter).post();
}

CheckDTO UpgradeHelp::getCheckRespDto(const nlohmann::json& jsonObj)
{
	CheckDTO rtn;
	if (!jsonObj.contains("fileSize") || jsonObj["fileSize"].is_null())
	{
		rtn.fileSize = 0;
	}
	else
	{
		rtn.fileSize = jsonObj["fileSize"].get<int>();
	}
	if (!jsonObj.contains("firmwareName") ||
		jsonObj["firmwareName"].is_null())
	{
		rtn.firmwareName = "";
	}
	else
	{
		rtn.firmwareName = jsonObj["firmwareName"].get<std::string>();
	}
	if (!jsonObj.contains("md5") ||
		jsonObj["md5"].is_null())
	{
		rtn.md5 = "";
	}
	else
	{
		rtn.md5 = jsonObj["md5"].get<std::string>();
	}
	if (!jsonObj.contains("recommendType") ||
		jsonObj["recommendType"].is_null())
	{
		rtn.recommendType = 0;
	}
	else
	{
		rtn.recommendType = jsonObj["recommendType"].get<int>();
	}
	if (!jsonObj.contains("url") ||
		jsonObj["url"].is_null())
	{
		rtn.url = "";
	}
	else
	{
		rtn.url = jsonObj["url"].get<std::string>();
	}
	if (!jsonObj.contains("version") ||
		jsonObj["version"].is_null())
	{
		rtn.version = "";
	}
	else
	{
		rtn.version = jsonObj["version"].get<std::string>();
	}
	if (jsonObj.contains("descList") &&
		!jsonObj["descList"].is_null())
	{
		for (auto& descJson : jsonObj["descList"])
		{
			DescDTO descDto;
			if (!descJson.contains("langCode") ||
				descJson["langCode"].is_null())
			{
				descDto.langCode = "";
			}
			else
			{
				descDto.langCode = descJson["langCode"].get<std::string>();
			}
			if (!descJson.contains("desc") ||
				descJson["desc"].is_null())
			{
				descDto.desc = "";
			}
			else
			{
				descDto.desc = descJson["desc"].get<std::string>();
			}
			rtn.descList.push_back(descDto);
		}
	}
	return rtn;
}
LoginRespDTO UpgradeHelp::getLoginRespDto(const nlohmann::json& jsonObj)
{
	LoginRespDTO rtn;
	if (!jsonObj.contains("code") || jsonObj["code"].is_null())
	{
		rtn.code = 0;
	}
	else
	{
		rtn.code = jsonObj["code"].get<int>();
	}
	if (jsonObj.contains("data") &&
		!jsonObj["data"].is_null())
	{
		nlohmann::json descJson = jsonObj["data"];
		UserDTO descDto;
		if (!descJson.contains("userId") ||
			descJson["userId"].is_null())
		{
			descDto.userId = "";
		}
		else
		{
			descDto.userId = descJson["userId"].get<std::string>();
		}
		if (!descJson.contains("token") ||
			descJson["token"].is_null())
		{
			descDto.token = "";
		}
		else
		{
			descDto.token = descJson["token"].get<std::string>();
		}
		rtn.data = descDto;
	}
	return rtn;
}
void UpgradeHelp::checkSuccessAction(const QString& response, bool isNeedUpdate, QWidget *parentWidget)
{
	nlohmann::json j = nlohmann::json::parse(response.toStdString());
	CheckRespDTO versionDto;
	nlohmann::json code = j.at("code");
	if (code.is_string() && code != "SUCCESS")
	{
		if (isNeedUpdate)
		{
            showIsNeweastVersion(parentWidget);
		}
		return;
	}
	nlohmann::json jData = j.at("data");
    versionDto.data = getCheckRespDto(jData);
	QString currentVersion = qApp->applicationVersion();
	QVersionNumber currentVersionNumber = QVersionNumber::fromString(currentVersion);
	QString serverVersion = QString::fromStdString(versionDto.data.version);
	//QString ignoreVersion = SettingManager::GetInstance()->ReadValue
	//("VersionUpdateIgnore", "Version");
	//if (!isNeedUpdate && ignoreVersion == serverVersion)
	//{
	//	startUserBehaviorUploadPro();
	//	return;
	//}
	QVersionNumber serverVersionNumber = QVersionNumber::fromString(serverVersion);
	int compareVersion = QVersionNumber::compare(currentVersionNumber, serverVersionNumber);
	if (compareVersion < 0)
	{
		QString url = "http://" + QString::fromStdString(versionDto.data.url);
		QStringList args;
		args << currentVersion;
		args << serverVersion;
		args << url;
		args << QString::number(versionDto.data.fileSize);
		if (versionDto.data.recommendType == 1)
		{
			killSoftWare(args);
		}
		else if (versionDto.data.recommendType == 2)
		{
			QString desc;
            QLocale::Language eLangType = AppSettingsManagement::getInstance()->getLanguage();
			for (int i = 0; i < versionDto.data.descList.size(); i++)
			{
				DescDTO dto = versionDto.data.descList.at(i);
				QString language = QString::fromStdString(dto.langCode);
				if (language == "zh_CN" && eLangType == QLocale::Chinese)
				{
					desc = QString::fromStdString(dto.desc);
					break;
				}
				if (language == "en")
				{
					desc = QString::fromStdString(dto.desc);
				}
			}
			desc = serverVersion + "\n\n" + desc;
            CNewVersionDescDialog upgradeView(serverVersion, desc, parentWidget);
			int rtn = upgradeView.exec();
			if (rtn == QDialog::Accepted)
			{
				killSoftWare(args);
			}
		}
	}
	else 
	{
		if (isNeedUpdate)
		{
            showIsNeweastVersion(parentWidget);
		}
	}
}
void UpgradeHelp::killSoftWare(QStringList args)
{
    AppSettingsManagement::getInstance()->setUpgradePlugArgs(args);
	qApp->exit(0);
}

void UpgradeHelp::showIsNeweastVersion(QWidget *parentWidget)
{
	QString content = tr("neweastVersion");
    MyMessageBox::information(parentWidget, tr("Upgrade"), content);
}

