// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest
#include "OCPP201.hpp"

#include <fmt/core.h>
#include <fstream>

namespace module {

const std::string EVEREST_OCPP_SHARE_PATH = "share/everest/ocpp201";
const std::string INIT_SQL = "init.sql";

namespace fs = std::filesystem;

void OCPP201::init() {
    invoke_init(*p_main);

    this->ocpp_share_path = fs::path(this->info.everest_prefix) / EVEREST_OCPP_SHARE_PATH;

    auto configured_config_path = fs::path(this->config.ChargePointConfigPath);

    // try to find the config file if it has been provided as a relative path
    if (!fs::exists(configured_config_path) && configured_config_path.is_relative()) {
        configured_config_path = this->ocpp_share_path / configured_config_path;
    }
    if (!fs::exists(configured_config_path)) {
        EVLOG_AND_THROW(Everest::EverestConfigError(
            fmt::format("OCPP config file is not available at given path: {} which was resolved to: {}",
                        this->config.ChargePointConfigPath, configured_config_path.string())));
    }
    EVLOG_info << "OCPP config: " << configured_config_path.string();

    std::ifstream ifs(configured_config_path.c_str());
    std::string config_file((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    auto json_config = json::parse(config_file);

    if (!fs::exists(this->config.MessageLogPath)) {
        try {
            fs::create_directory(this->config.MessageLogPath);
        } catch (const fs::filesystem_error& e) {
            EVLOG_AND_THROW(e);
        }
    }

    this->charge_point = std::make_unique<ocpp::v201::ChargePoint>(json_config, this->ocpp_share_path.string(),
                                                                   this->config.MessageLogPath);
    this->charge_point->start();
}

void OCPP201::ready() {
    invoke_ready(*p_main);
}

} // namespace module
