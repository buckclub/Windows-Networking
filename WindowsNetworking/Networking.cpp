#include "Networking.h"

std::wstring ConvertToWString(const std::string& input) {
    int outputSize = MultiByteToWideChar(CP_UTF8, 0, input.c_str(), -1, nullptr, 0);
    std::wstring output(outputSize, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, input.c_str(), -1, &output[0], outputSize);
    return output;
}

bool Networking::SendDiscordWebhookMessage(const char* webhookUrl, const char* message) {
    nlohmann::json webhookData;
    webhookData["content"] = message;
    std::string postData = webhookData.dump();
    std::string contentType = "application/json";
    bool result = HttpPostRequest(webhookUrl, contentType, postData);
    return result;
}

bool Networking::SendDiscordWebhookMessage(const char* webhookUrl, nlohmann::json webhookData) {
    std::string postData = webhookData.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
    std::string contentType = "application/json";
    bool result = HttpPostRequest(webhookUrl, contentType, postData);
    return result;
}


std::string Networking::HttpGetRequest(const std::string& url) {
    std::wstring wUrl = ConvertToWString(url);
    WinHttpHandle hSession = WinHttpOpen(L"Chrome", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        return "";
    }

    URL_COMPONENTS urlComponents = { sizeof(URL_COMPONENTS) };
    std::wstring hostName(256, 0);
    std::wstring urlPath(256, 0);

    urlComponents.lpszHostName = &hostName[0];
    urlComponents.dwHostNameLength = hostName.size();
    urlComponents.lpszUrlPath = &urlPath[0];
    urlComponents.dwUrlPathLength = urlPath.size();

    if (!WinHttpCrackUrl(wUrl.c_str(), wUrl.length(), 0, &urlComponents)) {
        WinHttpCloseHandle(hSession);
        return "";
    }

    WinHttpHandle hConnect = WinHttpConnect(hSession, urlComponents.lpszHostName, urlComponents.nPort, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return "";
    }

    DWORD requestFlags = WINHTTP_FLAG_SECURE;
    WinHttpHandle hRequest = WinHttpOpenRequest(hConnect, L"GET", urlComponents.lpszUrlPath, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, requestFlags);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    if (!WinHttpReceiveResponse(hRequest, nullptr)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    std::string buffer;
    DWORD bytesRead = 0;
    do {
        char readBuffer[1024] = {};
        if (!WinHttpReadData(hRequest, readBuffer, sizeof(readBuffer), &bytesRead)) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return "";
        }
        buffer.append(readBuffer, bytesRead);
    } while (bytesRead > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return buffer;
}

bool Networking::HttpPostRequest(const std::string& url, const std::string& contentType, const std::string& postData) {
    std::wstring wUrl = ConvertToWString(url);
    std::wstring wContentType = ConvertToWString(contentType);

    WinHttpHandle hSession = WinHttpOpen(L"Chrome", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        printf_s("Failed to open session\n");
        return false;
    }

    URL_COMPONENTS urlComponents = { sizeof(URL_COMPONENTS) };
    std::wstring hostName(256, 0);
    std::wstring urlPath(256, 0);

    urlComponents.lpszHostName = &hostName[0];
    urlComponents.dwHostNameLength = hostName.size();
    urlComponents.lpszUrlPath = &urlPath[0];
    urlComponents.dwUrlPathLength = urlPath.size();

    if (!WinHttpCrackUrl(wUrl.c_str(), wUrl.length(), 0, &urlComponents)) {
        WinHttpCloseHandle(hSession);
        return false;
    }

    WinHttpHandle hConnect = WinHttpConnect(hSession, urlComponents.lpszHostName, urlComponents.nPort, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return false;
    }

    DWORD requestFlags = WINHTTP_FLAG_SECURE;
    WinHttpHandle hRequest = WinHttpOpenRequest(hConnect, L"POST", urlComponents.lpszUrlPath, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, requestFlags);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    std::wstring wFullHeader = L"Content-Type: " + wContentType;

    if (!WinHttpSendRequest(hRequest, wFullHeader.c_str(), -1L, (LPVOID)postData.c_str(), postData.size(), postData.size(), 0)) {
        DWORD errorCode = GetLastError();

        LPVOID lpMsgBuf;
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&lpMsgBuf,
            0,
            NULL);

        printf_s("Error code: %lu\nMessage: %ls\n", errorCode, (LPWSTR)lpMsgBuf);
        LocalFree(lpMsgBuf);

        printf_s("Failed to send request\n");
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    if (!WinHttpReceiveResponse(hRequest, nullptr)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Clean up
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return true;
}
