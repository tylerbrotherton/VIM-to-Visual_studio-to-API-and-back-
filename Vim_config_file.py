#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <curl/curl.h>
#include <vector>

class APIInteraction {
private:
    CURL* curl;
    std::string apiEndpoint;
    std::string method;
    std::string authType;
    std::string authValue;
    std::string contentType;

    // Callback for CURL response
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
        size_t total_size = size * nmemb;
        output->append((char*)contents, total_size);
        return total_size;
    }

public:
    APIInteraction(const std::string& endpoint, 
                   const std::string& httpMethod = "POST",
                   const std::string& authenticationMethod = "none",
                   const std::string& authenticationValue = "") 
        : apiEndpoint(endpoint), 
          method(httpMethod), 
          authType(authenticationMethod), 
          authValue(authenticationValue),
          contentType("application/json") {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
    }

    std::string sendAPIRequest(const std::string& payload) {
        if(!curl) {
            std::cerr << "CURL initialization failed" << std::endl;
            return "";
        }

        std::string responseString;
        struct curl_slist* headers = nullptr;

        // Set headers
        headers = curl_slist_append(headers, ("Content-Type: " + contentType).c_str());

        // Authentication handling
        if (authType == "bearer") {
            headers = curl_slist_append(headers, 
                ("Authorization: Bearer " + authValue).c_str());
        } else if (authType == "apikey") {
            headers = curl_slist_append(headers, 
                ("X-API-Key: " + authValue).c_str());
        }

        curl_easy_setopt(curl, CURLOPT_URL, apiEndpoint.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

        // Set HTTP Method
        if (method == "POST") {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        } else if (method == "GET") {
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        }

        // Perform the request
        CURLcode res = curl_easy_perform(curl);

        if(res != CURLE_OK) {
            std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
            return "";
        }

        // Clean up
        curl_slist_free_all(headers);

        return responseString;
    }

    // Read Vim buffer from file
    static std::string readVimBuffer(const std::string& filename) {
        std::ifstream file(filename);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    // Write response to tmux window
    void displayResponseInTmux(const std::string& response) {
        std::ofstream responseFile("/tmp/api_response.txt");
        responseFile << response;
        responseFile.close();

        // Open in tmux
        system("tmux new-window -n 'API Response' 'less /tmp/api_response.txt'");
    }

    ~APIInteraction() {
        if(curl) curl_easy_cleanup(curl);
        curl_global_cleanup();
    }
};

// Main function for CLI interaction
int
