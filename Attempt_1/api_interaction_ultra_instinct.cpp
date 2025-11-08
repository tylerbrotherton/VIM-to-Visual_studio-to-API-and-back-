#include <iostream> // I/O for peasants and champions alike.
#include <fstream> // File reading/writing because APIs rarely read minds.
#include <sstream> // For turning files into single glorious strings.
#include <string> // Strings: the duct tape of modern programs.
#include <vector> // Vectors because arrays are so 1998.
#include <map> // Maps: key-value romance since forever.
#include <memory> // Smart pointers: do not fear the destructor.
#include <chrono> // Time utilities for backoff and existential dread.
#include <thread> // Sleep, nap, recover, try again.
#include <regex> // For the regex sorcery you'd rather avoid.
#include <filesystem> // Filesystem: the place where bugs hide.

#include <cpr/cpr.h> // cpr: the HTTP library that actually helps you ship.
#include <openssl/ssl.h> // For when things need to be encrypted and mysterious.
#include <boost/algorithm/string.hpp> // Boost string tools because we like extra power.

// ---------------------------------------------------------------------------
// THE LEGENDARY API HANDLER 9001
// If this file could run on its own it'd probably refactor you back to life.
// ---------------------------------------------------------------------------

class APIInteractionLogger { // Logging class, tiny and powerful.
public:
    enum class LogLevel { DEBUG, INFO, WARNING, ERROR }; // Severity levels, choose wisely.

    static void log(LogLevel level, const std::string& message) { // Single-point logging convenience.
        std::string levelStr; // Will become the human-readable prefix.
        switch (level) { // Tiny switch to convert enum to string.
            case LogLevel::DEBUG: levelStr = "DEBUG"; break; // Dev mode: spam it.
            case LogLevel::INFO: levelStr = "INFO"; break; // Normal mode: useful info.
            case LogLevel::WARNING: levelStr = "WARNING"; break; // Warning: things smell funny.
            case LogLevel::ERROR: levelStr = "ERROR"; break; // Error: run for debug logs.
        }
        std::cout << "[" << levelStr << "] " << message << std::endl; // Behold the single-line log.
    }
}; // APIInteractionLogger: concise, savage, informative.

// ---------------------------------------------------------------------------
// APICredentialManager
// Keeps secrets (in a file) and pretends it's secure. Don't commit creds.
// ---------------------------------------------------------------------------

class APICredentialManager { // Store and retrieve API tokens like a pro.
private:
    std::map<std::string, std::string> credentials; // Map of key -> token.
    std::string credentialFile; // Path to the credential file.

    void loadCredentials() { // Load credentials from file, tolerantly.
        std::ifstream file(credentialFile); // Open the file like it's about to tell you a secret.
        std::string line; // Buffer for each line.

        while (std::getline(file, line)) { // Read each line, ignoring the drama.
            std::vector<std::string> parts; // Will receive split parts.
            boost::split(parts, line, boost::is_any_of(":")); // Split by colon like civilized humans.
            if (parts.size() == 2) { // Expect exactly key:value format.
                credentials[parts[0]] = parts[1]; // Store the credential as if it's a fine wine.
            }
        }
    }

public:
    APICredentialManager(const std::string& file = "~/.api_credentials") // Default path for the bold.
        : credentialFile(file) { // Initialize the member with the given path.
        loadCredentials(); // Load immediately because preparedness is sexy.
    }

    std::string getCredential(const std::string& key) { // Fetch a credential; may be empty.
        return credentials[key]; // Return empty string if not present — the silent killer.
    }
}; // APICredentialManager: does what it says, with mild attitude.

// ---------------------------------------------------------------------------
// APIRequestBuilder
// A fluent builder so requests can be composed like sonnets.
// ---------------------------------------------------------------------------

class APIRequestBuilder { // Build requests like you're stacking LEGO bricks.
private:
    std::string endpoint; // The URL we will politely pester.
    std::string method; // HTTP method: GET, POST, PUT, DELETE, etc.
    std::map<std::string, std::string> headers; // Headers to send with dignity.
    std::map<std::string, std::string> queryParams; // Query params in the URL.
    std::string body; // Body payload, the main message in a bottle.
    std::string authType; // "bearer" or "apikey" or "sorcery".
    std::string authToken; // Token string, hopefully not "changeme".

public:
    APIRequestBuilder(const std::string& url) // Construct with URL, defaults to POST.
        : endpoint(url), method("POST") {} // Default method is POST because many APIs demand it.

    APIRequestBuilder& setMethod(const std::string& httpMethod) { // Change method gracefully.
        method = httpMethod; // Replace the method.
        return *this; // Return *this to chain like a pro.
    }

    APIRequestBuilder& addHeader(const std::string& key, const std::string& value) { // Add an HTTP header.
        headers[key] = value; // Insert or overwrite the header.
        return *this; // Chainable goodness.
    }

    APIRequestBuilder& addQueryParam(const std::string& key, const std::string& value) { // Add query param.
        queryParams[key] = value; // Put it in the map.
        return *this; // Return *this for fluent calls.
    }

    APIRequestBuilder& setBody(const std::string& requestBody) { // Set the request body.
        body = requestBody; // Copy in the body (maybe big).
        return *this; // For chaining until infinity.
    }

    APIRequestBuilder& setAuthentication(const std::string& type, const std::string& token) { // Add auth header.
        authType = type; // e.g., "bearer"
        authToken = token; // the secret stuff

        if (authType == "bearer") { // Bearer token style auth.
            headers["Authorization"] = "Bearer " + authToken; // Standard bearer header.
        } else if (authType == "apikey") { // API key style auth.
            headers["X-API-Key"] = authToken; // Custom header; many APIs accept this.
        }
        return *this; // Chain like the wind.
    }

    cpr::Response execute() { // Execute the constructed request; pray if necessary.
        cpr::Header cprHeaders; // CPR's header container.
        for (const auto& kv : headers) { // Convert our map to CPR headers.
            cprHeaders[kv.first] = kv.second; // Assign each header entry.
        }

        cpr::Parameters cprParams; // CPR's parameters container for query params.
        for (const auto& kv : queryParams) { // Add each query param to the CPR structure.
            cprParams.Add({kv.first, kv.second}); // Magical Add call — make sure your CPR version supports it.
        }

        if (method == "POST") { // Handle POST requests.
            return cpr::Post(cpr::Url{endpoint}, cpr::Body{body}, cprHeaders, cprParams); // Send POST.
        } else if (method == "GET") { // Handle GET requests.
            return cpr::Get(cpr::Url{endpoint}, cprHeaders, cprParams); // Send GET.
        } else if (method == "PUT") { // Handle PUT requests.
            return cpr::Put(cpr::Url{endpoint}, cpr::Body{body}, cprHeaders, cprParams); // Send PUT.
        } else if (method == "DELETE") { // Handle DELETE requests.
            return cpr::Delete(cpr::Url{endpoint}, cprHeaders, cprParams); // Send DELETE.
        }

        throw std::runtime_error("Unsupported HTTP method — the universe hates this request."); // Fatal if unsupported.
    }
}; // APIRequestBuilder: small, mighty, opinionated.

// ---------------------------------------------------------------------------
// APIResponseHandler
// Saves the response to disk and logs the deed like a hero.
// ---------------------------------------------------------------------------

class APIResponseHandler { // Convert responses into files and status messages.
public:
    enum class ResponseFormat { JSON, XML, PLAIN, BINARY }; // Output formats we care about.

    static void processResponse(const cpr::Response& response,
                                ResponseFormat format = ResponseFormat::JSON,
                                const std::string& outputPath = "/tmp/api_response") { // Save response appropriately.
        if (response.status_code != 200) { // If it's not 200, something is wrong.
            APIInteractionLogger::log(APIInteractionLogger::LogLevel::ERROR, // Log the error.
                                      "API Request Failed: " + std::to_string(response.status_code)); // Make noise.
            throw std::runtime_error("API request failed — inspect logs, consult ritual."); // Throw to caller.
        }

        std::filesystem::path outputFile; // The file path we'll write to.
        switch (format) { // Determine extension by format.
            case ResponseFormat::JSON: outputFile = outputPath + ".json"; break; // JSON extension.
            case ResponseFormat::XML: outputFile = outputPath + ".xml"; break; // XML extension.
            case ResponseFormat::PLAIN: outputFile = outputPath + ".txt"; break; // Plain text.
            case ResponseFormat::BINARY: outputFile = outputPath + ".bin"; break; // Binary payload.
        }

        std::ofstream outFile(outputFile, std::ios::binary); // Open output file stream.
        outFile << response.text; // Dump response text into it like confetti.
        outFile.close(); // Close the file and celebrate.

        APIInteractionLogger::log(APIInteractionLogger::LogLevel::INFO, // Inform the world.
                                  "Response saved to: " + outputFile.string()); // Where to find the treasure.
    }
}; // APIResponseHandler: part archivist, part hype-man.

// ---------------------------------------------------------------------------
// RetryStrategy
// Tries again, because perseverance is a programmer's middle name.
// ---------------------------------------------------------------------------

class RetryStrategy { // Retry wrapper to tame flaky endpoints.
private:
    int maxRetries; // How many times to try before rage-quitting.
    std::chrono::milliseconds delay; // Initial delay between tries.

public:
    RetryStrategy(int retries = 3, int delayMs = 1000) // Default: 3 retries, 1 second initial delay.
        : maxRetries(retries), delay(delayMs) {} // Initialize members.

    template<typename Func>
    auto executeWithRetry(Func&& apiCall) { // Execute a callable with retry semantics.
        for (int attempt = 0; attempt < maxRetries; ++attempt) { // Loop up to maxRetries.
            try {
                return apiCall(); // If it works, return immediately.
            } catch (const std::exception& e) { // If it throws, handle.
                if (attempt == maxRetries - 1) throw; // Last attempt: rethrow for caller to handle.

                APIInteractionLogger::log(APIInteractionLogger::LogLevel::WARNING, // Log the retry.
                                          "Attempt " + std::to_string(attempt + 1) +
                                          " failed. Retrying in " + std::to_string(delay.count()) + "ms");
                std::this_thread::sleep_for(delay); // Sleep for the delay.
                delay *= 2; // Exponential backoff because servers like drama.
            }
        }
        throw std::runtime_error("Max retries exceeded — the API wins this round."); // If loop finishes oddly.
    }
}; // RetryStrategy: patient, persistent, slightly smug.

// ---------------------------------------------------------------------------
// FileContentReader
// Little utilities to read/write files like a civilized hacker.
// ---------------------------------------------------------------------------

class FileContentReader { // Read files into strings and write strings to files.
public:
    static std::string readFile(const std::string& filepath) { // Read entire file as string.
        std::ifstream file(filepath, std::ios::binary); // Binary mode to preserve bytes.
        if (!file) throw std::runtime_error("Cannot open file: " + filepath); // Error if missing.

        std::stringstream buffer; // Buffer to accumulate file contents.
        buffer << file.rdbuf(); // Stream file into buffer.
        return buffer.str(); // Return the glorious content.
    }

    static void writeFile(const std::string& filepath, const std::string& content) { // Write string to file.
        std::ofstream file(filepath, std::ios::binary); // Open for writing binary-safe.
        if (!file) throw std::runtime_error("Cannot write to file: " + filepath); // Blow up on failure.
        file << content; // Push content into file.
    }
}; // FileContentReader: simple, dependable, slightly smug.

// ---------------------------------------------------------------------------
// APIInteractionManager
// The conductor orchestrating credential loading, building, retrying, and saving.
// ---------------------------------------------------------------------------

class APIInteractionManager { // Higher-level API orchestration.
private:
    APICredentialManager credentialManager; // Manages tokens.
    RetryStrategy retryStrategy; // Handles retries.

public:
    std::string performAPIInteraction(const std::string& endpoint,
                                      const std::string& inputFilePath,
                                      const std::string& apiName = "default",
                                      const std::string& method = "POST") { // Simple interaction wrapper.
        try {
            std::string payload = FileContentReader::readFile(inputFilePath); // Read payload from file.
            std::string authType = "bearer"; // Default auth type for modern times.
            std::string authToken = credentialManager.getCredential(apiName + "_token"); // Retrieve token.

            auto apiResponse = retryStrategy.executeWithRetry([&]() { // Execute with retry semantics.
                return APIRequestBuilder(endpoint) // Build the request in one gorgeous chain.
                    .setMethod(method) // Set HTTP method.
                    .addHeader("Content-Type", "application/json") // Set content type, because JSON is life.
                    .setAuthentication(authType, authToken) // Set authentication headers.
                    .setBody(payload) // Attach body payload.
                    .execute(); // Fire it off like confetti.
            });

            APIResponseHandler::processResponse(apiResponse, APIResponseHandler::ResponseFormat::JSON); // Save result.
            return apiResponse.text; // Return the body to caller.
        } catch (const std::exception& e) { // Catch and log anything that went sideways.
            APIInteractionLogger::log(APIInteractionLogger::LogLevel::ERROR,
                                      std::string("API Interaction Failed: ") + e.what()); // Scream into the logs.
            throw; // Re-throw for caller to decide penalty.
        }
    }

    std::string advancedAPIInteraction(const std::string& endpoint,
                                       const std::string& inputFilePath,
                                       const std::map<std::string, std::string>& customHeaders = {},
                                       const std::map<std::string, std::string>& queryParams = {},
                                       const std::string& method = "POST",
                                       APIResponseHandler::ResponseFormat responseFormat = APIResponseHandler::ResponseFormat::JSON) { // More options.
        try {
            std::string payload = FileContentReader::readFile(inputFilePath); // Read input file payload.
            auto requestBuilder = APIRequestBuilder(endpoint) // Start building the request.
                .setMethod(method) // Set HTTP method upfront.
                .addHeader("Content-Type", "application/json") // Default header, humble but effective.
                .setBody(payload); // Place the payload where it needs to be.

            for (const auto& kv : customHeaders) requestBuilder.addHeader(kv.first, kv.second); // Add extra headers like it's 2005.
            for (const auto& kv : queryParams) requestBuilder.addQueryParam(kv.first, kv.second); // Add query params for the pedants.

            auto apiResponse = retryStrategy.executeWithRetry([&]() { return requestBuilder.execute(); }); // Execute with retries.

            APIResponseHandler::processResponse(apiResponse, responseFormat); // Save and log the response.
            return apiResponse.text; // Return response body to caller.
        } catch (const std::exception& e) { // If something breaks, log and rethrow.
            APIInteractionLogger::log(APIInteractionLogger::LogLevel::ERROR,
                                      std::string("Advanced API Interaction Failed: ") + e.what()); // Log the defeat.
            throw; // Let caller handle the sorrow.
        }
    }
}; // APIInteractionManager: conductor, ringmaster, friend.

// ---------------------------------------------------------------------------
// main()
// The final frontier — where parameters meet destiny.
// Usage example:
//    ./api_overlord <endpoint> <input_file> <method> [api_name] [output_path]
// Example:
//    ./api_overlord "https://postman-echo.com/post" "payload.json" "POST" "default" "/tmp/output"
// ---------------------------------------------------------------------------

int main(int argc, char* argv[]) { // The chosen one: main.
    try {
        // Check for required parameters like a bouncer at a code club.
        if (argc < 4) {
            std::cerr << "Usage: " << argv[0]
                      << " <endpoint> <input_file> <method> [api_name] [output_path]\n";
            std::cerr << "Example: " << argv[0]
                      << " \"https://postman-echo.com/post\" payload.json POST\n";
            return 1; // Early exit before the chaos begins.
        }

        // Grab CLI arguments because hardcoding is for mortals.
        std::string endpoint = argv[1];       // The API endpoint.
        std::string inputFile = argv[2];      // JSON or whatever you’re sending.
        std::string method = argv[3];         // GET, POST, PUT, DELETE, etc.
        std::string apiName = (argc > 4) ? argv[4] : "default"; // Optional API name for credentials.
        std::string outputPath = (argc > 5) ? argv[5] : "/tmp/api_response"; // Optional output path.

        // Log that we’re about to do something heroic.
        APIInteractionLogger::log(APIInteractionLogger::LogLevel::INFO,
            "Starting API call to: " + endpoint + " using " + method);

        // Construct the all-powerful manager.
        APIInteractionManager apiManager;

        // Do the deed — one request to rule them all.
        std::string responseText = apiManager.performAPIInteraction(
            endpoint, inputFile, apiName, method
        );

        // Process and save the response like the data hoarder you are.
        APIResponseHandler::processResponse(
            cpr::Response{200, "", responseText, {}, cpr::Url{endpoint}},
            APIResponseHandler::ResponseFormat::JSON,
            outputPath
        );

        // Log success because we deserve it.
        APIInteractionLogger::log(APIInteractionLogger::LogLevel::INFO,
            "Request completed successfully. Output saved to: " + outputPath + ".json");

        return 0; // Mission accomplished, no survivors (except us).
    } catch (const std::exception& e) {
        // Catastrophic meltdown caught here.
        APIInteractionLogger::log(APIInteractionLogger::LogLevel::ERROR,
            std::string("Fatal Error: ") + e.what());
        return 1; // Exit in shame.
    }
}
