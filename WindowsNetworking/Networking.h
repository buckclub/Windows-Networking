#pragma once

#include <string>
#include <wchar.h>
#include <Windows.h>
#include <winhttp.h>
#include <codecvt>
#include <locale>
#include <memory>
#include "nlohmann.h"
#pragma comment(lib, "winhttp.lib")

class Networking {
public:
    static Networking& GetInstance() {
        static Networking instance;
        return instance;
    }
    Networking(Networking const&) = delete;
    void operator=(Networking const&) = delete;

    bool SendDiscordWebhookMessage(const char* webhookUrl, const char* message);
    bool SendDiscordWebhookMessage(const char* webhookUrl, nlohmann::json webhookData);
    std::string HttpGetRequest(const std::string& url);
    bool HttpPostRequest(const std::string& url, const std::string& contentType, const std::string& postData);
private:
    Networking() {}
};

class WinHttpHandle {
public:
    WinHttpHandle(HINTERNET handle) : handle_(handle) {}
    ~WinHttpHandle() {
        if (handle_) {
            WinHttpCloseHandle(handle_);
        }
    }
    operator HINTERNET() const { return handle_; }

private:
    HINTERNET handle_;
};
