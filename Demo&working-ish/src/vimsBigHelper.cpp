#include <iostream>
#include <string>
#include <stdexcept> // For runtime_error
#include <cstdlib>   // For getenv, exit
#include <curl/curl.h> // For libcurl functions
#include "json.hpp"   // For nlohmann/json

// Use nlohmann/json namespace
using json = nlohmann::json;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    // Append the new data to the string
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    // Return the number of bytes handled
    return size * nmemb;
}


std::string fetchAPIData(const std::string& endpoint_url, const std::string& prompt) {
    // 1. Get API Key from environment variable
    const char* api_key_cstr = std::getenv("API_KEY");
    if (api_key_cstr == nullptr) {
        throw std::runtime_error("Error: API_KEY environment variable not set.");
    }
    std::string api_key = api_key_cstr;

    // 2. Construct the full URL with the API key
    std::string full_url = endpoint_url + "?key=" + api_key;

    // 3. Construct the Gemini-specific JSON payload
    json payload;
    payload["contents"] = json::array({
        {
            {"parts", json::array({
                {{"text", prompt}}
            })}
        }
    });
    std::string payload_str = payload.dump(); // Serialize JSON to string

    CURL* curl = nullptr;
    CURLcode res;
    std::string readBuffer; // String to store the response
    long http_code = 0;
    struct curl_slist* headers = nullptr;

    try {
        curl = curl_easy_init(); // Initialize libcurl
        if (!curl) {
            throw std::runtime_error("Failed to initialize libcurl.");
        }

        // 4. Prepare the POST request headers
        headers = curl_slist_append(headers, "Content-Type: application/json");
        if (!headers) {
            throw std::runtime_error("Failed to create curl headers.");
        }

        // Set libcurl options
        curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload_str.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // 5. Make the request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            throw std::runtime_error("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
        }

        // Get the HTTP response code
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        // Cleanup
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        // Check for HTTP errors
        if (http_code >= 400) {
            throw std::runtime_error("HTTP Error: " + std::to_string(http_code) + "\n" + readBuffer);
        }

        // 6. Parse the Gemini-specific response
        try {
            json response_json = json::parse(readBuffer);
            std::string text_output = response_json["candidates"][0]["content"]["parts"][0]["text"];
            return text_output;
        } catch (const json::exception& e) {
            // Handle cases where the JSON response is not what we expect
            throw std::runtime_error("Error: Could not parse API response.\nJSON Error: " + std::string(e.what()) + "\nResponse: " + readBuffer);
        }

    } catch (...) {
        // Ensure cleanup happens even if an exception is thrown
        if (headers) curl_slist_free_all(headers);
        if (curl) curl_easy_cleanup(curl);
        throw; // Re-throw the caught exception
    }
}

int main(int argc, char* argv[]) {
    // Check for the correct number of arguments
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <endpoint_url> <prompt>" << std::endl;
        return 1; // Use 1 for error exit status
    }

    std::string endpoint = argv[1];
    std::string prompt_text = argv[2];

    try {
        std::string output = fetchAPIData(endpoint, prompt_text);
        std::cout << output << std::endl;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1; // Use 1 for error exit status
    }

    return 0; // Success
}
