/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-07-03 13:54:47
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-05 20:25:27
 * @FilePath: /Raspi2USBL/config/yamlconfig.h
 * @Description: load yaml config file
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */

#ifndef _YAMLCONFIG_H_
#define _YAMLCONFIG_H_

#include "../config/defineconfig.h"
#include "../core/systeminfo.h"
#include <iostream>
#include <sstream>
#include <string>
#include <yaml-cpp/yaml.h>

class YamlConfig {
public:
    YamlConfig() = default;
    YamlConfig(std::string loadConfigPath);
    ~YamlConfig() = default;

    void        init();
    bool        open(std::string loadConfigPath);
    bool        loadConfig(SystemInfo &systemInfo);
    std::string replaceKeyStr(const std::string str);

private:
    bool        isOpenYaml_   = false;
    bool        isLoadConfig_ = false;
    std::string yamlConfigPath_;
    YAML::Node  yamlConfigNode_;
    std::string timeStr;
};

#endif // _YAMLCONFIG_H_