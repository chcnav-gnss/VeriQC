/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI upgrade Module
 * *-
 * @file UpgradeDTO.h
 * @author CHC
 * @date 2025-09-28
 * @brief http response DTO
 * 
**************************************************************************/
#ifndef UPGRADEDTO_H_
#define UPGRADEDTO_H_ 
#include <qstring.h> 
#include "nlohmann/json.hpp"  
struct UserDTO
{
	std::string userId;
	std::string  token;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserDTO, userId, token)
struct LoginRespDTO
{
	int code;
	UserDTO data;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LoginRespDTO, code, data)
struct DescDTO
{
	std::string langCode;
	std::string desc;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DescDTO, langCode, desc)
struct CheckDTO {

	int fileSize;

	std::string firmwareName;
	std::string md5;
	int recommendType;
	std::string url;
	std::string version;
	std::vector<DescDTO> descList; 
};
struct CheckRespDTO
{
	int code;
	CheckDTO data; 
};

#endif //UPGRADEDTO_H_
