// SPDX-FileCopyrightText: 2026 Maxim Dokukin (maxdokukin.com)
// SPDX-License-Identifier: GPL-3.0-only
// xewe-os-module-web-interface/src/WebInterface/WebInterface.h
#pragma once

#include <WebServer.h>
#include <string>
#include <sstream>
#include <iomanip>

#include <XeWeOS.h>
#include "../Wifi/Wifi.h"


class WebInterface : public xewe::os::Module {
public:
                                WebInterface                (xewe::os::ModuleController& controller,
                                                             Wifi&                       wifi);

    void                        begin_routines_regular      ()       override;

    void                        loop                        ()                              override;
    std::string                 status                      (const bool verbose=false)      const override;

    WebServer&                  get_server                  ()                              { return http_server; }
private:
    Wifi&                       wifi;
    WebServer                   http_server                  {80};

    void                        serve_main_page               ();
    void                        handle_command_request        ();

    static const char           INDEX_HTML                  [] PROGMEM;
};
