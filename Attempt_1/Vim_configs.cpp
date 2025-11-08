#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

class VSCodeAPIInteraction {
private:
    std::string endpoint;
    std::string authType;
    std::string authToken;

public:
    VSCodeAPIInteraction(
        const std::string& apiEndpoint, 
        const std::string& authentication = "none", 
        const std::string& token = ""
    ) : 
        endpoint(apiEndpoint), 
        authType(authentication), 
        authToken(token) {}

    nlohmann::json sendAPIRequest(const std::string& payload) {
        cpr::Response response;
        cpr::Header headers = {
            {"Content-Type", "application/json"}
        };

        // Authentication handling
        if (authType == "bearer") {
            headers["Authorization"] = "Bearer " + authToken;
        } else if (authType == "apikey") {
            headers["X-API-Key"] = authToken;
        }

        try {
            response = cpr::Post(
                cpr::Url{endpoint},
                cpr::Body{payload},
                headers
            );

            if (response.status_code == 200) {
                return nlohmann::json::parse(response.text);
            } else {
                throw std::runtime_error("API call failed: " + 
                    std::to_string(response.status_code));
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return nlohmann::json{};
        }
    }

    // Read file content (simulating VS Code text editor)
    static std::string readFileContent(const std::string& filepath) {
        std::ifstream file(filepath);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    // Write response to file
    void writeResponseToFile(const nlohmann::json& response) {
        std::ofstream outputFile("/tmp/api_response.json");
        outputFile << response.dump(4);
        outputFile.close();
    }
};

// Command-line interface for testing
int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] 
                  << " <endpoint> <filepath> [auth_type] [auth_token]\n";
        return 1;
    }

    std::string endpoint = argv[1];
    std::string filepath = argv[2];
    std::string authType = argc > 3 ? argv[3] : "none";
    std::string authToken = argc > 4 ? argv[4] : "";

    try {
        VSCodeAPIInteraction apiInteraction(
            endpoint, 
            authType, 
            authToken
        );

        // Read file content
        std::string payload = VSCodeAPIInteraction::readFileContent(filepath);

        // Send API request
        nlohmann::json response = apiInteraction.sendAPIRequest(payload);

        // Write response to file
        apiInteraction.writeResponseToFile(response);

       // std::cout << "API Response written to /tmp/api_response.json\n";
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
