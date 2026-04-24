#include "WebServerRunner.h"
#include "logging/Logger.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>

#ifdef _WIN32

WebServerRunner::WebServerRunner() = default;

WebServerRunner::~WebServerRunner() {
    stop();
}

void WebServerRunner::start() {
    if (mRunning.exchange(true)) {
        return;
    }

    mWorker = std::thread(&WebServerRunner::run, this);
}

void WebServerRunner::stop() {
    mRunning = false;
    if (mWorker.joinable()) {
        mWorker.join();
    }
}

bool WebServerRunner::hasCommand() const {
    std::lock_guard<std::mutex> lock(mCommandMutex);
    return mPendingCommand.has_value();
}

WebCommand WebServerRunner::getCommand() {
    std::lock_guard<std::mutex> lock(mCommandMutex);
    if (!mPendingCommand.has_value()) {
        throw std::runtime_error("No pending web command");
    }

    WebCommand command = *mPendingCommand;
    mPendingCommand.reset();
    return command;
}

void WebServerRunner::setState(OrchestratorState state) {
    std::lock_guard<std::mutex> lock(mStateMutex);
    mState = state;
}

OrchestratorState WebServerRunner::getState() const {
    std::lock_guard<std::mutex> lock(mStateMutex);
    return mState;
}

void WebServerRunner::run() {
    LOG_WARN("WebServerRunner is not implemented for Windows");
    while (mRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

bool WebServerRunner::tryStoreCommand(const WebCommand& command) {
    std::lock_guard<std::mutex> lock(mCommandMutex);
    if (mPendingCommand.has_value()) {
        return false;
    }

    mPendingCommand = command;
    return true;
}

#else

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

namespace {
constexpr int kServerPort = 8081;
constexpr int kListenBacklog = 8;
constexpr int kSocketPollTimeoutMs = 200;
constexpr int kSocketReadTimeoutMs = 1000;

struct HttpRequest {
    std::string method;
    std::string path;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

struct HttpResponse {
    int statusCode;
    std::string statusText;
    std::string body;
};

std::string toLowerCopy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

std::string trim(std::string value) {
    const auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char c) {
        return std::isspace(c) != 0;
    });
    const auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char c) {
        return std::isspace(c) != 0;
    }).base();

    if (first >= last) {
        return "";
    }

    return std::string(first, last);
}

std::string jsonEscape(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size());

    for (const char c : value) {
        switch (c) {
            case '\\':
                escaped += "\\\\";
                break;
            case '"':
                escaped += "\\\"";
                break;
            case '\n':
                escaped += "\\n";
                break;
            case '\r':
                escaped += "\\r";
                break;
            case '\t':
                escaped += "\\t";
                break;
            default:
                escaped += c;
                break;
        }
    }

    return escaped;
}

std::string formatLogTimestampForApi(const std::chrono::system_clock::time_point& timestamp) {
    const std::time_t rawTime = std::chrono::system_clock::to_time_t(timestamp);
    std::tm timeInfo{};

#ifdef _WIN32
    localtime_s(&timeInfo, &rawTime);
#else
    localtime_r(&rawTime, &timeInfo);
#endif

    std::ostringstream stream;
    stream << std::put_time(&timeInfo, "%H:%M:%S");
    return stream.str();
}

std::string buildLogsJsonBody() {
    const auto entries = Logger::instance().entries();

    std::ostringstream stream;
    stream << "{\"entries\":[";

    for (std::size_t index = 0; index < entries.size(); ++index) {
        const auto& entry = entries[index];
        if (index > 0) {
            stream << ',';
        }

        stream << "{\"timestamp\":\"" << jsonEscape(formatLogTimestampForApi(entry.timestamp))
               << "\",\"type\":\"" << jsonEscape(toString(entry.type))
               << "\",\"message\":\"" << jsonEscape(entry.message) << "\"}";
    }

    stream << "]}";
    return stream.str();
}

std::string buildHttpResponse(const HttpResponse& response) {
    std::ostringstream stream;
    stream << "HTTP/1.1 " << response.statusCode << ' ' << response.statusText << "\r\n";
    stream << "Content-Type: application/json\r\n";
    stream << "Access-Control-Allow-Origin: *\r\n";
    stream << "Connection: close\r\n";
    stream << "Content-Length: " << response.body.size() << "\r\n\r\n";
    stream << response.body;
    return stream.str();
}

HttpResponse makeJsonResponse(int statusCode, const std::string& statusText, const std::string& body) {
    return HttpResponse{statusCode, statusText, body};
}

HttpResponse makeEmptyResponse(int statusCode, const std::string& statusText) {
    return HttpResponse{statusCode, statusText, ""};
}

bool recvAll(int socketFd, std::string& rawRequest) {
    char buffer[4096];
    std::size_t headersEnd = std::string::npos;
    std::size_t contentLength = 0;

    while (true) {
        const ssize_t bytesRead = recv(socketFd, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            return false;
        }

        rawRequest.append(buffer, static_cast<std::size_t>(bytesRead));

        if (headersEnd == std::string::npos) {
            headersEnd = rawRequest.find("\r\n\r\n");
            if (headersEnd != std::string::npos) {
                const std::string headerBlock = rawRequest.substr(0, headersEnd);
                const std::string contentLengthHeader = "content-length:";
                std::istringstream headerStream(headerBlock);
                std::string line;
                std::getline(headerStream, line);
                while (std::getline(headerStream, line)) {
                    if (!line.empty() && line.back() == '\r') {
                        line.pop_back();
                    }
                    const std::string lowerLine = toLowerCopy(line);
                    if (lowerLine.rfind(contentLengthHeader, 0) == 0) {
                        const std::string value = trim(line.substr(contentLengthHeader.size()));
                        contentLength = static_cast<std::size_t>(std::stoul(value));
                    }
                }
            }
        }

        if (headersEnd != std::string::npos) {
            const std::size_t expectedSize = headersEnd + 4 + contentLength;
            if (rawRequest.size() >= expectedSize) {
                return true;
            }
        }
    }
}

bool parseHttpRequest(const std::string& rawRequest, HttpRequest& request) {
    const std::size_t headersEnd = rawRequest.find("\r\n\r\n");
    if (headersEnd == std::string::npos) {
        return false;
    }

    std::istringstream requestStream(rawRequest.substr(0, headersEnd));
    std::string requestLine;
    if (!std::getline(requestStream, requestLine)) {
        return false;
    }

    if (!requestLine.empty() && requestLine.back() == '\r') {
        requestLine.pop_back();
    }

    std::istringstream requestLineStream(requestLine);
    if (!(requestLineStream >> request.method >> request.path)) {
        return false;
    }

    const std::size_t queryPos = request.path.find('?');
    if (queryPos != std::string::npos) {
        request.path = request.path.substr(0, queryPos);
    }

    std::string headerLine;
    while (std::getline(requestStream, headerLine)) {
        if (!headerLine.empty() && headerLine.back() == '\r') {
            headerLine.pop_back();
        }
        if (headerLine.empty()) {
            continue;
        }

        const std::size_t colonPos = headerLine.find(':');
        if (colonPos == std::string::npos) {
            continue;
        }

        std::string name = toLowerCopy(trim(headerLine.substr(0, colonPos)));
        std::string value = trim(headerLine.substr(colonPos + 1));
        request.headers[std::move(name)] = std::move(value);
    }

    request.body = rawRequest.substr(headersEnd + 4);
    return true;
}

bool extractJsonString(const std::string& body, const std::string& key, std::string& value) {
    const std::string needle = "\"" + key + "\"";
    const std::size_t keyPos = body.find(needle);
    if (keyPos == std::string::npos) {
        return false;
    }

    std::size_t colonPos = body.find(':', keyPos + needle.size());
    if (colonPos == std::string::npos) {
        return false;
    }

    std::size_t quotePos = colonPos + 1;
    while (quotePos < body.size() && std::isspace(static_cast<unsigned char>(body[quotePos])) != 0) {
        ++quotePos;
    }

    if (quotePos >= body.size() || body[quotePos] != '"') {
        return false;
    }

    ++quotePos;
    std::string parsed;
    bool escaped = false;
    while (quotePos < body.size()) {
        const char c = body[quotePos++];
        if (escaped) {
            switch (c) {
                case '"':
                case '\\':
                case '/':
                    parsed += c;
                    break;
                case 'n':
                    parsed += '\n';
                    break;
                case 'r':
                    parsed += '\r';
                    break;
                case 't':
                    parsed += '\t';
                    break;
                default:
                    return false;
            }
            escaped = false;
            continue;
        }

        if (c == '\\') {
            escaped = true;
            continue;
        }

        if (c == '"') {
            value = std::move(parsed);
            return true;
        }

        parsed += c;
    }

    return false;
}

bool canAcceptStart(OrchestratorState state) {
    return state == OrchestratorState::Stopped;
}

bool canAcceptStop(OrchestratorState state) {
    return state != OrchestratorState::Stopped &&
           state != OrchestratorState::Stopping &&
           state != OrchestratorState::Idle &&
           state != OrchestratorState::Faulted;
}

bool canAcceptGetObject(OrchestratorState state) {
    return state == OrchestratorState::Idle;
}

bool canAcceptSkipReq(OrchestratorState state) {
    return state != OrchestratorState::Stopped &&
           state != OrchestratorState::Stopping &&
           state != OrchestratorState::Starting &&
           state != OrchestratorState::Faulted;
}
}

WebServerRunner::WebServerRunner() = default;

WebServerRunner::~WebServerRunner() {
    stop();
}

void WebServerRunner::start() {
    if (mRunning.exchange(true)) {
        return;
    }

    mWorker = std::thread(&WebServerRunner::run, this);
}

void WebServerRunner::stop() {
    mRunning = false;
    if (mWorker.joinable()) {
        mWorker.join();
    }
}

bool WebServerRunner::hasCommand() const {
    std::lock_guard<std::mutex> lock(mCommandMutex);
    return mPendingCommand.has_value();
}

WebCommand WebServerRunner::getCommand() {
    std::lock_guard<std::mutex> lock(mCommandMutex);
    if (!mPendingCommand.has_value()) {
        throw std::runtime_error("No pending web command");
    }

    WebCommand command = *mPendingCommand;
    mPendingCommand.reset();
    return command;
}

void WebServerRunner::setState(OrchestratorState state) {
    std::lock_guard<std::mutex> lock(mStateMutex);
    mState = state;
}

OrchestratorState WebServerRunner::getState() const {
    std::lock_guard<std::mutex> lock(mStateMutex);
    return mState;
}

bool WebServerRunner::tryStoreCommand(const WebCommand& command) {
    std::lock_guard<std::mutex> lock(mCommandMutex);
    if (mPendingCommand.has_value()) {
        return false;
    }

    mPendingCommand = command;
    return true;
}

void WebServerRunner::run() {
    const int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        LOG_ERROR("WebServerRunner failed to create socket");
        mRunning = false;
        return;
    }

    const int enable = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(kServerPort);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(serverFd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        const std::string bindError =
            "WebServerRunner failed to bind to 127.0.0.1:" + std::to_string(kServerPort) + ": " + std::strerror(errno);
        LOG_ERROR(bindError.c_str());
        close(serverFd);
        mRunning = false;
        return;
    }

    if (listen(serverFd, kListenBacklog) < 0) {
        const std::string listenError =
            "WebServerRunner failed to listen on 127.0.0.1:" + std::to_string(kServerPort) + ": " + std::strerror(errno);
        LOG_ERROR(listenError.c_str());
        close(serverFd);
        mRunning = false;
        return;
    }

    LOG_INFO("Web server is started");

    while (mRunning) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(serverFd, &readSet);

        timeval timeout{};
        timeout.tv_sec = 0;
        timeout.tv_usec = kSocketPollTimeoutMs * 1000;

        const int ready = select(serverFd + 1, &readSet, nullptr, nullptr, &timeout);
        if (ready <= 0) {
            continue;
        }

        sockaddr_in clientAddress{};
        socklen_t clientLength = sizeof(clientAddress);
        const int clientFd = accept(serverFd, reinterpret_cast<sockaddr*>(&clientAddress), &clientLength);
        if (clientFd < 0) {
            continue;
        }

        timeval socketTimeout{};
        socketTimeout.tv_sec = kSocketReadTimeoutMs / 1000;
        socketTimeout.tv_usec = (kSocketReadTimeoutMs % 1000) * 1000;
        setsockopt(clientFd, SOL_SOCKET, SO_RCVTIMEO, &socketTimeout, sizeof(socketTimeout));
        setsockopt(clientFd, SOL_SOCKET, SO_SNDTIMEO, &socketTimeout, sizeof(socketTimeout));

        HttpResponse response{500, "Internal Server Error", "{\"error\":\"Unhandled request\"}"};
        std::string rawRequest;
        if (recvAll(clientFd, rawRequest)) {
            HttpRequest request;
            if (!parseHttpRequest(rawRequest, request)) {
                response = makeJsonResponse(400, "Bad Request", "{\"error\":\"Invalid HTTP request\"}");
            } else if (request.method == "GET" && request.path == "/getstate") {
                response = makeJsonResponse(
                    200,
                    "OK",
                    std::string("{\"state\":\"") + jsonEscape(toString(getState())) +
                        "\",\"stateLabel\":\"" + jsonEscape(toUiString(getState())) + "\"}"
                );
            } else if (request.method == "GET" && request.path == "/logs") {
                response = makeJsonResponse(200, "OK", buildLogsJsonBody());
            } else if (request.method == "POST" && request.path == "/cmdStart") {
                if (!canAcceptStart(getState())) {
                    response = makeJsonResponse(409, "Conflict", "{\"error\":\"cmdStart is not valid in the current state\"}");
                } else if (!tryStoreCommand(WebCommand{WebCommandType::Start, ""})) {
                    response = makeJsonResponse(409, "Conflict", "{\"error\":\"Another command is already pending\"}");
                } else {
                    response = makeJsonResponse(202, "Accepted", "{\"status\":\"accepted\"}");
                }
            } else if (request.method == "POST" && request.path == "/cmdStop") {
                if (!canAcceptStop(getState())) {
                    response = makeJsonResponse(409, "Conflict", "{\"error\":\"cmdStop is not valid in the current state\"}");
                } else if (!tryStoreCommand(WebCommand{WebCommandType::Stop, ""})) {
                    response = makeJsonResponse(409, "Conflict", "{\"error\":\"Another command is already pending\"}");
                } else {
                    response = makeJsonResponse(202, "Accepted", "{\"status\":\"accepted\"}");
                }
            } else if (request.method == "POST" && request.path == "/cmdGetObj") {
                std::string objectId;
                if (!extractJsonString(request.body, "objectId", objectId) || objectId.empty()) {
                    response = makeJsonResponse(400, "Bad Request", "{\"error\":\"cmdGetObj requires a non-empty objectId\"}");
                } else if (!canAcceptGetObject(getState())) {
                    response = makeJsonResponse(409, "Conflict", "{\"error\":\"cmdGetObj is not valid in the current state\"}");
                } else if (!tryStoreCommand(WebCommand{WebCommandType::GetObject, objectId})) {
                    response = makeJsonResponse(409, "Conflict", "{\"error\":\"Another command is already pending\"}");
                } else {
                    response = makeJsonResponse(202, "Accepted", "{\"status\":\"accepted\"}");
                }
            } else if (request.method == "POST" && request.path == "/cmdSkipReq") {
                if (!canAcceptSkipReq(getState())) {
                    response = makeEmptyResponse(409, "Conflict");
                } else if (!tryStoreCommand(WebCommand{WebCommandType::SkipReq, ""})) {
                    response = makeEmptyResponse(409, "Conflict");
                } else {
                    response = makeEmptyResponse(204, "No Content");
                }
            } else {
                response = makeJsonResponse(404, "Not Found", "{\"error\":\"Unknown endpoint\"}");
            }
        } else {
            response = makeJsonResponse(400, "Bad Request", "{\"error\":\"Failed to read request\"}");
        }

        const std::string payload = buildHttpResponse(response);
        send(clientFd, payload.c_str(), payload.size(), 0);
        close(clientFd);
    }

    close(serverFd);
}

#endif
